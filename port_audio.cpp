#include "port_audio.h"

#include "port_audio_callback_data.h"

#include "core/os/memory.h"
#include "core/os/os.h"

#ifdef PA_USE_WASAPI
#include <pa_win_wasapi.h>
#endif

#include <portaudio.h>

#pragma region IMP_DETAILS

class CallbackUserDataGdBinding {
public:
    PortAudio *port_audio;
    Ref<PortAudioStream> stream;
    Callable audio_callback;
    Callable stream_finished_callback;
    Ref<PortAudioCallbackData> audio_callback_data;
    uint64_t last_call_duration;
    int output_sample_size;
    int input_sample_size;
    int output_channel_count;
    int input_channel_count;

    CallbackUserDataGdBinding() {
        port_audio = nullptr;
        last_call_duration = 0;
        stream = Ref<PortAudioStream>();
        audio_callback = Callable();
        stream_finished_callback = Callable();
        audio_callback_data = Ref<PortAudioCallbackData>();
        output_sample_size = 0;
        input_sample_size = 0;
        output_channel_count = 0;
        input_channel_count = 0;
    }
};

static int port_audio_callback_gd_binding_converter(const void *p_input_buffer, void *p_output_buffer,
                                                    unsigned long p_frames_per_buffer,
                                                    const PaStreamCallbackTimeInfo *p_time_info,
                                                    PaStreamCallbackFlags p_status_flags, void *p_user_data) {

    uint64_t micro_seconds_start = OS::get_singleton()->get_ticks_usec();

    CallbackUserDataGdBinding *user_data = (CallbackUserDataGdBinding *) p_user_data;
    if (!user_data) {
        print_line("PortAudio::port_audio_callback_converter: !user_data");
        return PortAudio::PortAudioCallbackResult::ABORT;
    }

    // retrieve callback data
    Ref<PortAudioCallbackData> audio_callback_data = user_data->audio_callback_data;
    Ref<StreamPeerBuffer> input_buffer;
    bool has_input = false;
    if (p_input_buffer) {
        input_buffer = user_data->audio_callback_data->get_input_buffer();
        has_input = input_buffer.is_valid();
    }
    Ref<StreamPeerBuffer> output_buffer;
    bool has_output = false;
    if (p_output_buffer) {
        output_buffer = user_data->audio_callback_data->get_output_buffer();
        has_output = output_buffer.is_valid();
    }

    // copy input buffer to godot type, if available
    if (has_input) {
        input_buffer->seek(0);
        uint8_t *input_buffer_ptr = (uint8_t *) p_input_buffer;
        input_buffer->put_data(input_buffer_ptr, p_frames_per_buffer);
    }

    // provide params
    audio_callback_data->set_input_buffer_adc_time(p_time_info->inputBufferAdcTime);
    audio_callback_data->set_current_time(p_time_info->currentTime);
    audio_callback_data->set_output_buffer_dac_time(p_time_info->outputBufferDacTime);
    audio_callback_data->set_frames_per_buffer(p_frames_per_buffer);
    audio_callback_data->set_status_flags(p_status_flags);
    audio_callback_data->set_last_call_duration(user_data->last_call_duration);

    // set buffer to start
    if (has_output) {
        output_buffer->seek(0);
    }

    // perform callback
    Variant variant = audio_callback_data;
    const Variant *variant_ptr = &variant;
    const Variant **p_args = &variant_ptr;
    Variant result;
    Callable::CallError error;
    user_data->audio_callback.call(p_args, 1, result, error);
    if (error.error != Callable::CallError::CALL_OK) {
        print_line("PortAudio::port_audio_callback_converter: != Variant::CallError::CALL_OK");
    }

    // write to output buffer
    if (has_output) {
        int buffer_size = p_frames_per_buffer * user_data->output_channel_count * user_data->output_sample_size;
        int bytes_written = output_buffer->get_position();
        if (bytes_written > buffer_size) {
            print_line(
                    vformat("PortAudio::port_audio_callback_converter: bytes_written (%d) > p_frames_per_buffer (%d) - data truncated",
                            bytes_written, buffer_size));
            return PortAudio::PortAudioCallbackResult::CONTINUE;
        }
        output_buffer->seek(0);
        int read;
        uint8_t *output_buffer_ptr = (uint8_t *) p_output_buffer;
        output_buffer->get_partial_data(output_buffer_ptr, bytes_written, read);
    }

    // evaluate callback result
    int callback_result = 0;
    if (result.get_type() != Variant::INT) {
        print_line(vformat("PortAudio::port_audio_callback_converter: invalid return type: %s - returning 0",
                           result.get_type()));
    } else {
        callback_result = result;
    }

    uint64_t micro_seconds_end = OS::get_singleton()->get_ticks_usec();
    user_data->last_call_duration = micro_seconds_end - micro_seconds_start;

    return callback_result;
}

static void port_audio_stream_finished_callback_gd_binding_converter(void *p_user_data) {
    CallbackUserDataGdBinding *user_data = (CallbackUserDataGdBinding *) p_user_data;
    if (!user_data) {
        print_line("PortAudio::port_audio_stream_finished_callback_gd_binding_converter: !user_data");
        return;
    }
    Callable stream_finished_callback = user_data->stream_finished_callback;
    if (stream_finished_callback.is_null()) {
        print_line(
                "PortAudio::port_audio_stream_finished_callback_gd_binding_converter: stream_finished_callback.is_null())");
        return;
    }
    Variant var_user_data = user_data->audio_callback_data;
    const Variant *var_user_data_ptr = &var_user_data;
    Callable::CallError call_error;
    Variant result;
    stream_finished_callback.call(&var_user_data_ptr, 1, result, call_error);
    if (call_error.error != Callable::CallError::CALL_OK) {
        print_line(vformat("PortAudio::port_audio_stream_finished_callback_gd_binding_converter: call_error.error: %s",
                           call_error.error));
    }
}

static PortAudio::PortAudioError get_error(PaError p_error) {
    switch (p_error) {
        case paNoError:
            return PortAudio::PortAudioError::NO_ERROR;
        case paNotInitialized:
            return PortAudio::PortAudioError::NOT_INITIALIZED;
        case paUnanticipatedHostError:
            return PortAudio::PortAudioError::UNANTICIPATED_HOST_ERROR;
        case paInvalidChannelCount:
            return PortAudio::PortAudioError::INVALID_CHANNEL_COUNT;
        case paInvalidSampleRate:
            return PortAudio::PortAudioError::INVALID_SAMPLE_RATE;
        case paInvalidDevice:
            return PortAudio::PortAudioError::INVALID_DEVICE;
        case paInvalidFlag:
            return PortAudio::PortAudioError::INVALID_FLAG;
        case paSampleFormatNotSupported:
            return PortAudio::PortAudioError::SAMPLE_FORMAT_NOT_SUPPORTED;
        case paBadIODeviceCombination:
            return PortAudio::PortAudioError::BAD_IO_DEVICE_COMBINATION;
        case paInsufficientMemory:
            return PortAudio::PortAudioError::INSUFFICIENT_MEMORY;
        case paBufferTooBig:
            return PortAudio::PortAudioError::BUFFER_TO_BIG;
        case paBufferTooSmall:
            return PortAudio::PortAudioError::BUFFER_TOO_SMALL;
        case paNullCallback:
            return PortAudio::PortAudioError::NULL_CALLBACK;
        case paBadStreamPtr:
            return PortAudio::PortAudioError::BAD_STREAM_PTR;
        case paTimedOut:
            return PortAudio::PortAudioError::TIMED_OUT;
        case paInternalError:
            return PortAudio::PortAudioError::INTERNAL_ERROR;
        case paDeviceUnavailable:
            return PortAudio::PortAudioError::DEVICE_UNAVAILABLE;
        case paIncompatibleHostApiSpecificStreamInfo:
            return PortAudio::PortAudioError::INCOMPATIBLE_HOST_API_SPECIFIC_STREAM_INFO;
        case paStreamIsStopped:
            return PortAudio::PortAudioError::STREAM_IS_STOPPED;
        case paStreamIsNotStopped:
            return PortAudio::PortAudioError::STREAM_IS_NOT_STOPPED;
        case paInputOverflowed:
            return PortAudio::PortAudioError::INPUT_OVERFLOWED;
        case paOutputUnderflowed:
            return PortAudio::PortAudioError::OUTPUT_UNDERFLOWED;
        case paHostApiNotFound:
            return PortAudio::PortAudioError::HOST_API_NOT_FOUND;
        case paInvalidHostApi:
            return PortAudio::PortAudioError::INVALID_HOST_API;
        case paCanNotReadFromACallbackStream:
            return PortAudio::PortAudioError::CAN_NOT_READ_FROM_A_CALLBACK_STREAM;
        case paCanNotWriteToACallbackStream:
            return PortAudio::PortAudioError::CAN_NOT_WRITE_TO_A_CALLBACK_STREAM;
        case paCanNotReadFromAnOutputOnlyStream:
            return PortAudio::PortAudioError::CAN_NOT_READ_FROM_AN_OUT_PUTONLY_STREAM;
        case paCanNotWriteToAnInputOnlyStream:
            return PortAudio::PortAudioError::CAN_NOT_WRITE_TO_AN_INPUT_ONLY_STREAM;
        case paIncompatibleStreamHostApi:
            return PortAudio::PortAudioError::INCOMPATIBLE_STREAM_HOST_API;
        case paBadBufferPtr:
            return PortAudio::PortAudioError::BAD_BUFFER_PTR;
    }
    if (p_error > 0) {
        // not an error but a return value.
        return (PortAudio::PortAudioError) p_error;
    }
    print_error(vformat("PortAudio::get_error: undefined error code: %d", p_error));
    return PortAudio::PortAudioError::UNDEFINED;
}

static PaSampleFormat get_sample_format(PortAudioStreamParameter::PortAudioSampleFormat p_sample_format) {

    // todo check non interleaved flag

    //PaSampleFormat sample_format = 0;
    //if (p_sample_format & PortAudioStreamParameter::PortAudioSampleFormat::NON_INTERLEAVED) {
    //	non_interleaved = true;
    //}
    //
    //switch (p_sample_format) {
    //	case PortAudioStreamParameter::PortAudioSampleFormat::FLOAT_32:
    //		return paFloat32;
    //	case PortAudioStreamParameter::PortAudioSampleFormat::INT_32:
    //		return paInt32;
    //	case PortAudioStreamParameter::PortAudioSampleFormat::INT_24:
    //		return paInt24;
    //	case PortAudioStreamParameter::PortAudioSampleFormat::INT_16:
    //		return paInt16;
    //	case PortAudioStreamParameter::PortAudioSampleFormat::INT_8:
    //		return paInt8;
    //	case PortAudioStreamParameter::PortAudioSampleFormat::U_INT_8:
    //		return paUInt8;
    //	case PortAudioStreamParameter::PortAudioSampleFormat::CUSTOM_FORMAT:
    //		return paCustomFormat;
    //}

    //print_error(vformat("PortAudio::get_sample_format: undefined sample_format code: %d", p_sample_format));
    return (PaSampleFormat) p_sample_format;
}

#pragma endregion IMP_DETAILS

PortAudio *PortAudio::singleton = NULL;

PortAudio *PortAudio::get_singleton() {
    return singleton;
}

int PortAudio::get_version() {
    return Pa_GetVersion();
}

String PortAudio::get_version_text() {
    return String(Pa_GetVersionText());
}

Dictionary PortAudio::get_version_info() {
    const PaVersionInfo *pa_version_info = Pa_GetVersionInfo();
    Dictionary version_info;
    version_info["version_major"] = pa_version_info->versionMajor;
    version_info["version_minor"] = pa_version_info->versionMinor;
    version_info["version_sub_minor"] = pa_version_info->versionSubMinor;
    version_info["version_control_revision"] = pa_version_info->versionControlRevision;
    version_info["version_text"] = pa_version_info->versionText;
    return version_info;
}

String PortAudio::get_error_text(PortAudio::PortAudioError p_error) {
    switch (p_error) {
        case UNDEFINED:
            return "Undefined error code";
        case NOT_PORT_AUDIO_NODE:
            return "Provided node is not of type PortAudioNode";
        case INVALID_FUNC_REF:
            return "INVALID_FUNC_REF";
        case STREAM_NOT_FOUND:
            return "STREAM_NOT_FOUND";
        case STREAM_USER_DATA_NOT_FOUND:
            return "STREAM_USER_DATA_NOT_FOUND";
    }
    return String(Pa_GetErrorText(p_error));
}

PortAudio::PortAudioError PortAudio::initialize() {
    PaError err = Pa_Initialize();
    return get_error(err);
}

PortAudio::PortAudioError PortAudio::terminate() {
    PaError err = Pa_Terminate();
    return get_error(err);
}

int PortAudio::get_host_api_count() {
    return Pa_GetHostApiCount();
}

int PortAudio::get_default_host_api() {
    return Pa_GetDefaultHostApi();
}

Dictionary PortAudio::get_host_api_info(int p_host_api) {
    const PaHostApiInfo *pa_api_info = Pa_GetHostApiInfo(p_host_api);
    if (pa_api_info == NULL) {
        // TODO error
    }
    Dictionary api_info;
    api_info["struct_version"] = pa_api_info->structVersion;
    api_info["type"] = (int) pa_api_info->type; // TODO enum
    api_info["name"] = String(pa_api_info->name);
    api_info["device_count"] = pa_api_info->deviceCount;
    api_info["default_input_device"] = pa_api_info->defaultInputDevice;
    api_info["default_output_device"] = pa_api_info->defaultOutputDevice;
    return api_info;
}

int PortAudio::host_api_type_id_to_host_api_index(int p_host_api_type_id) {
    return Pa_HostApiTypeIdToHostApiIndex((PaHostApiTypeId) p_host_api_type_id);
}

int PortAudio::host_api_device_index_to_device_index(int p_host_api, int p_host_api_device_index) {
    return Pa_HostApiDeviceIndexToDeviceIndex(p_host_api, p_host_api_device_index);
}

Dictionary PortAudio::get_last_host_error_info() {
    const PaHostErrorInfo *pa_host_error_info = Pa_GetLastHostErrorInfo();
    if (pa_host_error_info == NULL) {
        // TODO error
        return Dictionary();
    }
    Dictionary host_error_info;
    host_error_info["host_api_type"] = (int) pa_host_error_info->hostApiType;
    host_error_info["error_code"] = (int64_t) pa_host_error_info->errorCode;
    host_error_info["error_text"] = String(pa_host_error_info->errorText);
    return host_error_info;
}

int PortAudio::get_device_count() {
    return Pa_GetDeviceCount();
}

int PortAudio::get_default_input_device() {
    return Pa_GetDefaultInputDevice();
}

int PortAudio::get_default_output_device() {
    return Pa_GetDefaultOutputDevice();
}

Dictionary PortAudio::get_device_info(int p_device_index) {
    const PaDeviceInfo *pa_device_info = Pa_GetDeviceInfo((PaDeviceIndex) p_device_index);
    if (pa_device_info == NULL) {
        // TODO error
        return Dictionary();
    }
    Dictionary device_info;
    device_info["struct_version"] = pa_device_info->structVersion;
    device_info["name"] = String(pa_device_info->name);
    device_info["host_api"] = pa_device_info->hostApi;
    device_info["max_input_channels"] = pa_device_info->maxInputChannels;
    device_info["max_output_channels"] = pa_device_info->maxOutputChannels;
    device_info["default_low_input_latency"] = pa_device_info->defaultLowInputLatency;
    device_info["default_low_output_latency"] = pa_device_info->defaultLowOutputLatency;
    device_info["default_high_input_latency"] = pa_device_info->defaultHighInputLatency;
    device_info["default_high_output_latency"] = pa_device_info->defaultHighOutputLatency;
    device_info["default_sample_rate"] = pa_device_info->defaultSampleRate;
    return device_info;
}

PortAudio::PortAudioError PortAudio::is_format_supported(Ref<PortAudioStreamParameter> p_input_stream_parameter,
                                                         Ref<PortAudioStreamParameter> p_output_stream_parameter,
                                                         double p_sample_rate) {
    const PaStreamParameters pa_input_parameter = {
            p_input_stream_parameter->get_device_index(),
            p_input_stream_parameter->get_channel_count(),
            get_sample_format(p_input_stream_parameter->get_sample_format()),
            p_input_stream_parameter->get_suggested_latency(),
            p_input_stream_parameter->get_host_api_specific_stream_info(),
    };
    const PaStreamParameters pa_output_parameter = {
            p_output_stream_parameter->get_device_index(),
            p_output_stream_parameter->get_channel_count(),
            get_sample_format(p_output_stream_parameter->get_sample_format()),
            p_output_stream_parameter->get_suggested_latency(),
            p_output_stream_parameter->get_host_api_specific_stream_info(),
    };
    PaError err = Pa_IsFormatSupported(&pa_input_parameter, &pa_output_parameter, p_sample_rate);
    return get_error(err);
}

PortAudio::PortAudioError
PortAudio::open_stream(Ref<PortAudioStream> p_stream, Callable p_audio_callback, Variant p_user_data) {
    if (p_audio_callback.is_null()) {
        return PortAudio::PortAudioError::INVALID_FUNC_REF;
    }

    CallbackUserDataGdBinding *user_data = new CallbackUserDataGdBinding();
    user_data->port_audio = this;
    user_data->audio_callback = p_audio_callback;
    user_data->audio_callback_data.instance();
    user_data->audio_callback_data->set_user_data(p_user_data);

    const PaStreamParameters *pa_input_parameter_ptr = nullptr;
    Ref<PortAudioStreamParameter> input_parameter = p_stream->get_input_stream_parameter();
    if (input_parameter.is_valid()) {
        PaSampleFormat pa_sample_format = get_sample_format(input_parameter->get_sample_format());
        PaError sample_size = Pa_GetSampleSize(pa_sample_format);
        if (sample_size <= 0) {
            return get_error(sample_size);
        }
        user_data->input_channel_count = input_parameter->get_channel_count();
        user_data->input_sample_size = (int) sample_size;
        const PaStreamParameters pa_input_parameter = {
                input_parameter->get_device_index(),
                input_parameter->get_channel_count(),
                get_sample_format(input_parameter->get_sample_format()),
                input_parameter->get_suggested_latency(),
                input_parameter->get_host_api_specific_stream_info(),
        };
        pa_input_parameter_ptr = &pa_input_parameter;
        Ref<StreamPeerBuffer> input_buffer;
        input_buffer.instance();
        input_buffer->resize(p_stream->get_frames_per_buffer());
        user_data->audio_callback_data->set_output_buffer(input_buffer);
    }

    const PaStreamParameters *pa_output_parameter_ptr = nullptr;
    Ref<PortAudioStreamParameter> output_parameter = p_stream->get_output_stream_parameter();
    if (output_parameter.is_valid()) {
        PaSampleFormat pa_sample_format = get_sample_format(output_parameter->get_sample_format());
        PaError sample_size = Pa_GetSampleSize(pa_sample_format);
        if (sample_size <= 0) {
            return get_error(sample_size);
        }
        user_data->output_channel_count = output_parameter->get_channel_count();
        user_data->output_sample_size = (int) sample_size;
        const PaStreamParameters pa_output_parameter = {
                output_parameter->get_device_index(),
                output_parameter->get_channel_count(),
                pa_sample_format,
                output_parameter->get_suggested_latency(),
                output_parameter->get_host_api_specific_stream_info(),
        };
        pa_output_parameter_ptr = &pa_output_parameter;
        Ref<StreamPeerBuffer> output_buffer;
        output_buffer.instance();
        output_buffer->resize(p_stream->get_frames_per_buffer());
        user_data->audio_callback_data->set_output_buffer(output_buffer);
    }

    PaStream *stream;
    PaError err = Pa_OpenStream(&stream,
                                pa_input_parameter_ptr,
                                pa_output_parameter_ptr,
                                p_stream->get_sample_rate(),
                                p_stream->get_frames_per_buffer(),
                                p_stream->get_stream_flags(),
                                &port_audio_callback_gd_binding_converter,
                                user_data);
    if (err == PaErrorCode::paNoError) {
        p_stream->set_stream(stream);
        data_map.insert(std::pair<Ref<PortAudioStream>, void *>(p_stream, user_data));
    } else {
        delete user_data;
    }
    return get_error(err);
}

PortAudio::PortAudioError PortAudio::open_default_stream(Ref<PortAudioStream> p_stream,
                                                         PortAudioStreamParameter::PortAudioSampleFormat p_sample_format,
                                                         Callable p_audio_callback, Variant p_user_data) {
    if (p_audio_callback.is_null()) {
        return PortAudio::PortAudioError::INVALID_FUNC_REF;
    }

    CallbackUserDataGdBinding *user_data = new CallbackUserDataGdBinding();
    user_data->port_audio = this;
    user_data->audio_callback = p_audio_callback;
    user_data->audio_callback_data.instance();
    user_data->audio_callback_data->set_user_data(p_user_data);

    PaSampleFormat pa_sample_format = get_sample_format(p_sample_format);
    PaError sample_size = Pa_GetSampleSize(pa_sample_format);
    if (sample_size <= 0) {
        return get_error(sample_size);
    }

    Ref<PortAudioStreamParameter> input_parameter = p_stream->get_input_stream_parameter();
    if (input_parameter.is_valid() && input_parameter->get_channel_count() > 0) {
        Ref<StreamPeerBuffer> input_buffer;
        input_buffer.instance();
        input_buffer->resize(p_stream->get_frames_per_buffer());
        user_data->audio_callback_data->set_input_buffer(input_buffer);
        user_data->input_sample_size = (int) sample_size;
        user_data->input_channel_count = p_stream->get_input_channel_count();
        input_parameter->set_sample_format(p_sample_format);
    }

    Ref<PortAudioStreamParameter> output_parameter = p_stream->get_output_stream_parameter();
    if (output_parameter.is_valid() && output_parameter->get_channel_count() > 0) {
        Ref<StreamPeerBuffer> output_buffer;
        output_buffer.instance();
        output_buffer->resize(p_stream->get_frames_per_buffer());
        user_data->audio_callback_data->set_output_buffer(output_buffer);
        user_data->output_sample_size = (int) sample_size;
        user_data->output_channel_count = p_stream->get_output_channel_count();
        output_parameter->set_sample_format(p_sample_format);
    }

    PaStream *stream;
    PaError err = Pa_OpenDefaultStream(&stream,
                                       p_stream->get_input_channel_count(),
                                       p_stream->get_output_channel_count(),
                                       pa_sample_format,
                                       p_stream->get_sample_rate(),
                                       p_stream->get_frames_per_buffer(),
                                       &port_audio_callback_gd_binding_converter,
                                       user_data);
    if (err == PaErrorCode::paNoError) {
        p_stream->set_stream(stream);
        data_map.insert(std::pair<Ref<PortAudioStream>, void *>(p_stream, user_data));
    }
    return get_error(err);
}

PortAudio::PortAudioError PortAudio::start_stream(Ref<PortAudioStream> p_stream) {
    PaStream *stream = (PaStream *) p_stream->get_stream();
    PaError err = Pa_StartStream(stream);
    return get_error(err);
}

PortAudio::PortAudioError PortAudio::stop_stream(Ref<PortAudioStream> p_stream) {
    PaStream *stream = (PaStream *) p_stream->get_stream();
    PaError err = Pa_StopStream(stream);
    return get_error(err);
}

PortAudio::PortAudioError PortAudio::abort_stream(Ref<PortAudioStream> p_stream) {
    PaStream *stream = (PaStream *) p_stream->get_stream();
    PaError err = Pa_AbortStream(stream);
    return get_error(err);
}

PortAudio::PortAudioError PortAudio::close_stream(Ref<PortAudioStream> p_stream) {
    PaStream *stream = (PaStream *) p_stream->get_stream();
    data_map.erase(p_stream);
    PaError err = Pa_CloseStream(stream);
    return get_error(err);
}

PortAudio::PortAudioError
PortAudio::set_stream_finished_callback(Ref<PortAudioStream> p_stream, Callable p_stream_finished_callback) {
    std::map<Ref<PortAudioStream>, void *>::iterator it = data_map.find(p_stream);
    if (it == data_map.end()) {
        print_line("PortAudio::set_stream_finished_callback: stream not found");
        return PortAudioError::STREAM_NOT_FOUND;
    }
    CallbackUserDataGdBinding *user_data = (CallbackUserDataGdBinding *) it->second;
    if (!user_data) {
        print_line("PortAudio::set_stream_finished_callback: !user_data");
        return PortAudioError::STREAM_USER_DATA_NOT_FOUND;
    }
    PaStream *stream = (PaStream *) p_stream->get_stream();
    PaStreamFinishedCallback *pa_stream_finished;
    if (p_stream_finished_callback.is_null()) {
        pa_stream_finished = nullptr;
        if (!user_data->stream_finished_callback.is_null()) {
            // unregister callback
            user_data->stream_finished_callback = Callable();
        }
    } else {
        pa_stream_finished = &port_audio_stream_finished_callback_gd_binding_converter;
        user_data->stream_finished_callback = p_stream_finished_callback;
    }
    PaError err = Pa_SetStreamFinishedCallback(stream, pa_stream_finished);
    return get_error(err);
}

PortAudio::PortAudioError PortAudio::is_stream_stopped(Ref<PortAudioStream> p_stream) {
    PaStream *stream = (PaStream *) p_stream->get_stream();
    PaError err = Pa_IsStreamStopped(stream);
    return get_error(err);
}

PortAudio::PortAudioError PortAudio::is_stream_active(Ref<PortAudioStream> p_stream) {
    PaStream *stream = (PaStream *) p_stream->get_stream();
    PaError err = Pa_IsStreamActive(stream);
    return get_error(err);
}

double PortAudio::get_stream_time(Ref<PortAudioStream> p_stream) {
    PaStream *stream = (PaStream *) p_stream->get_stream();
    PaTime time = Pa_GetStreamTime(stream);
    return time;
}

Dictionary PortAudio::get_stream_info(Ref<PortAudioStream> p_stream) {
    PaStream *stream = (PaStream *) p_stream->get_stream();
    const PaStreamInfo *pa_stream_info = Pa_GetStreamInfo(stream);
    Dictionary stream_info;
    stream_info["struct_version"] = pa_stream_info->structVersion;
    stream_info["input_latency"] = pa_stream_info->inputLatency;
    stream_info["output_latency"] = pa_stream_info->outputLatency;
    stream_info["sample_rate"] = pa_stream_info->sampleRate;
    return stream_info;
}

double PortAudio::get_stream_cpu_load(Ref<PortAudioStream> p_stream) {
    PaStream *stream = (PaStream *) p_stream->get_stream();
    return Pa_GetStreamCpuLoad(stream);
}

PortAudio::PortAudioError
PortAudio::read_stream(Ref<PortAudioStream> p_stream, PackedByteArray p_buffer, uint64_t p_frames) {
    PaStream *stream = (PaStream *) p_stream->get_stream();
    void *buffer = p_buffer.ptrw();
    PaError err = Pa_ReadStream(stream, buffer, p_frames);
    return get_error(err);
}

PortAudio::PortAudioError
PortAudio::write_stream(Ref<PortAudioStream> p_stream, PackedByteArray p_buffer, uint64_t p_frames) {
    PaStream *stream = (PaStream *) p_stream->get_stream();
    const void *buffer = p_buffer.ptr();
    PaError err = Pa_WriteStream(stream, buffer, p_frames);
    return get_error(err);
}

int64_t PortAudio::get_stream_read_available(Ref<PortAudioStream> p_stream) {
    PaStream *stream = (PaStream *) p_stream->get_stream();
    signed long available = Pa_GetStreamReadAvailable(stream);
    return available;
}

int64_t PortAudio::get_stream_write_available(Ref<PortAudioStream> p_stream) {
    PaStream *stream = (PaStream *) p_stream->get_stream();
    signed long available = Pa_GetStreamWriteAvailable(stream);
    return available;
}

PortAudio::PortAudioError PortAudio::get_sample_size(PortAudioStreamParameter::PortAudioSampleFormat p_sample_format) {
    PaSampleFormat pa_sample_format = get_sample_format(p_sample_format);
    PaError err = Pa_GetSampleSize(pa_sample_format);
    return get_error(err);
}

void PortAudio::sleep(unsigned int p_ms) {
    Pa_Sleep(p_ms);
}

PortAudio::PortAudioError PortAudio::util_device_index_to_host_api_index(int p_device_index) {
    const PaDeviceInfo *pa_device_info = Pa_GetDeviceInfo((PaDeviceIndex) p_device_index);
    if (pa_device_info == nullptr) {
        return PortAudioError::INVALID_DEVICE;
    }
    return (PortAudio::PortAudioError) pa_device_info->hostApi;
}

PortAudio::PortAudioError PortAudio::util_enable_exclusive_mode(Ref<PortAudioStreamParameter> p_stream_parameter) {
#ifdef PA_USE_WASAPI
    int device_index = p_stream_parameter->get_device_index();
    const PaDeviceInfo *pa_device_info = Pa_GetDeviceInfo((PaDeviceIndex) device_index);
    if (pa_device_info == NULL) {
        return PortAudioError::INVALID_DEVICE;
    }
    const PaHostApiInfo *pa_api_info = Pa_GetHostApiInfo(pa_device_info->hostApi);
    if (pa_api_info == NULL) {
        return PortAudioError::HOST_API_NOT_FOUND;
    }
    if (pa_api_info->type == paWASAPI) {
        PaWasapiStreamInfo *wasapiInfo = (PaWasapiStreamInfo *) memalloc(sizeof(PaWasapiStreamInfo));
        wasapiInfo->size = sizeof(PaWasapiStreamInfo);
        wasapiInfo->hostApiType = paWASAPI;
        wasapiInfo->version = 1;
        wasapiInfo->flags = (paWinWasapiExclusive | paWinWasapiThreadPriority);
        wasapiInfo->threadPriority = eThreadPriorityProAudio;
        p_stream_parameter->set_host_api_specific_stream_info(wasapiInfo);
        return PortAudioError::NO_ERROR;
    }
    return PortAudioError::INVALID_HOST_API;
#else
    // TODO wasapi not supported error
    return PortAudioError::INTERNAL_ERROR;
#endif
}

void
PortAudio::util_insert_buffer(Ref<StreamPeerBuffer> p_source, int p_source_offset, Ref<StreamPeerBuffer> p_destination,
                              int p_destination_offset, int p_length) {
    PackedByteArray src = p_source->get_data_array();
    p_destination->seek(p_destination_offset);
    p_destination->put_data(&src.ptr()[p_source_offset], p_length);
}

void PortAudio::util_write_buffer(Ref<StreamPeerBuffer> p_source, Ref<StreamPeerBuffer> p_destination, int p_length) {
    int src_position = p_source->get_position();
    int dst_position = p_destination->get_position();
    if (src_position + p_length > p_source->get_size()) {
        p_length = p_source->get_size() - src_position;
        if (p_length <= 0) {
            return;
        }
    }
    util_insert_buffer(p_source, src_position, p_destination, dst_position, p_length);
    p_source->seek(src_position + p_length);
    p_destination->seek(dst_position + p_length);
}

void PortAudio::_bind_methods() {
    // PA_* bindings
    ClassDB::bind_method(D_METHOD("get_version"), &PortAudio::get_version);
    ClassDB::bind_method(D_METHOD("get_version_text"), &PortAudio::get_version_text);
    ClassDB::bind_method(D_METHOD("get_version_info"), &PortAudio::get_version_info);
    ClassDB::bind_method(D_METHOD("get_error_text", "error"), &PortAudio::get_error_text);
    ClassDB::bind_method(D_METHOD("initialize"), &PortAudio::initialize);
    ClassDB::bind_method(D_METHOD("terminate"), &PortAudio::terminate);
    ClassDB::bind_method(D_METHOD("get_host_api_count"), &PortAudio::get_host_api_count);
    ClassDB::bind_method(D_METHOD("get_default_host_api"), &PortAudio::get_default_host_api);
    ClassDB::bind_method(D_METHOD("get_host_api_info", "host_api"), &PortAudio::get_host_api_info);
    ClassDB::bind_method(D_METHOD("host_api_type_id_to_host_api_index", "host_api_type_id"),
                         &PortAudio::host_api_type_id_to_host_api_index);
    ClassDB::bind_method(D_METHOD("host_api_device_index_to_device_index", "host_api", "host_api_device_index"),
                         &PortAudio::host_api_device_index_to_device_index);
    ClassDB::bind_method(D_METHOD("get_last_host_error_info"), &PortAudio::get_last_host_error_info);
    ClassDB::bind_method(D_METHOD("get_device_count"), &PortAudio::get_device_count);
    ClassDB::bind_method(D_METHOD("get_default_input_device"), &PortAudio::get_default_input_device);
    ClassDB::bind_method(D_METHOD("get_default_output_device"), &PortAudio::get_default_output_device);
    ClassDB::bind_method(D_METHOD("get_device_info", "device_index"), &PortAudio::get_device_info);
    ClassDB::bind_method(
            D_METHOD("is_format_supported", "input_stream_parameter", "output_stream_parameter", "sample_rate"),
            &PortAudio::is_format_supported);
    ClassDB::bind_method(D_METHOD("open_stream", "stream", "audio_callback", "user_data"), &PortAudio::open_stream);
    ClassDB::bind_method(D_METHOD("open_default_stream", "stream", "sample_format", "audio_callback", "user_data"),
                         &PortAudio::open_default_stream);

    //ClassDB::bind_method(D_METHOD("connect", "signal", "callable", "binds", "flags"), &Object::connect, DEFVAL(Array()), DEFVAL(0));

    ClassDB::bind_method(D_METHOD("close_stream", "stream"), &PortAudio::close_stream);
    ClassDB::bind_method(D_METHOD("set_stream_finished_callback", "stream", "stream_finished_callback"),
                         &PortAudio::set_stream_finished_callback);
    ClassDB::bind_method(D_METHOD("start_stream", "stream"), &PortAudio::start_stream);
    ClassDB::bind_method(D_METHOD("stop_stream", "stream"), &PortAudio::stop_stream);
    ClassDB::bind_method(D_METHOD("abort_stream", "stream"), &PortAudio::abort_stream);
    ClassDB::bind_method(D_METHOD("is_stream_stopped", "stream"), &PortAudio::is_stream_stopped);
    ClassDB::bind_method(D_METHOD("is_stream_active", "stream"), &PortAudio::is_stream_active);
    ClassDB::bind_method(D_METHOD("get_stream_info", "stream"), &PortAudio::get_stream_info);
    ClassDB::bind_method(D_METHOD("get_stream_time", "stream"), &PortAudio::get_stream_time);
    ClassDB::bind_method(D_METHOD("get_stream_cpu_load", "stream"), &PortAudio::get_stream_cpu_load);
    ClassDB::bind_method(D_METHOD("read_stream", "stream", "buffer", "frames"), &PortAudio::read_stream);
    ClassDB::bind_method(D_METHOD("write_stream", "stream", "buffer", "frames"), &PortAudio::write_stream);
    ClassDB::bind_method(D_METHOD("get_stream_read_available", "stream"), &PortAudio::get_stream_read_available);
    ClassDB::bind_method(D_METHOD("get_stream_write_available", "stream"), &PortAudio::get_stream_write_available);
    ClassDB::bind_method(D_METHOD("get_sample_size", "sample_format"), &PortAudio::get_sample_size);
    ClassDB::bind_method(D_METHOD("sleep", "ms"), &PortAudio::sleep);

    // Util
    ClassDB::bind_method(D_METHOD("util_device_index_to_host_api_index", "device_index"),
                         &PortAudio::util_device_index_to_host_api_index);
    ClassDB::bind_method(D_METHOD("util_enable_exclusive_mode", "stream_parameter"),
                         &PortAudio::util_enable_exclusive_mode);
    ClassDB::bind_method(
            D_METHOD("util_insert_buffer", "source", "source_offset", "destination", "destination_offset", "length"),
            &PortAudio::util_insert_buffer);
    ClassDB::bind_method(D_METHOD("util_write_buffer", "source", "destination", "length"),
                         &PortAudio::util_write_buffer);

    // PortAudioError - Custom
    BIND_ENUM_CONSTANT(UNDEFINED);
    BIND_ENUM_CONSTANT(NOT_PORT_AUDIO_NODE);
    BIND_ENUM_CONSTANT(INVALID_FUNC_REF);
    BIND_ENUM_CONSTANT(STREAM_NOT_FOUND);
    BIND_ENUM_CONSTANT(STREAM_USER_DATA_NOT_FOUND);
    // PortAudioError - Origninal
    BIND_ENUM_CONSTANT(NO_ERROR);
    BIND_ENUM_CONSTANT(NOT_INITIALIZED);
    BIND_ENUM_CONSTANT(UNANTICIPATED_HOST_ERROR);
    BIND_ENUM_CONSTANT(INVALID_CHANNEL_COUNT);
    BIND_ENUM_CONSTANT(INVALID_SAMPLE_RATE);
    BIND_ENUM_CONSTANT(INVALID_DEVICE);
    BIND_ENUM_CONSTANT(INVALID_FLAG);
    BIND_ENUM_CONSTANT(SAMPLE_FORMAT_NOT_SUPPORTED);
    BIND_ENUM_CONSTANT(BAD_IO_DEVICE_COMBINATION);
    BIND_ENUM_CONSTANT(INSUFFICIENT_MEMORY);
    BIND_ENUM_CONSTANT(BUFFER_TO_BIG);
    BIND_ENUM_CONSTANT(BUFFER_TOO_SMALL);
    BIND_ENUM_CONSTANT(NULL_CALLBACK);
    BIND_ENUM_CONSTANT(BAD_STREAM_PTR);
    BIND_ENUM_CONSTANT(TIMED_OUT);
    BIND_ENUM_CONSTANT(INTERNAL_ERROR);
    BIND_ENUM_CONSTANT(DEVICE_UNAVAILABLE);
    BIND_ENUM_CONSTANT(INCOMPATIBLE_HOST_API_SPECIFIC_STREAM_INFO);
    BIND_ENUM_CONSTANT(STREAM_IS_STOPPED);
    BIND_ENUM_CONSTANT(STREAM_IS_NOT_STOPPED);
    BIND_ENUM_CONSTANT(INPUT_OVERFLOWED);
    BIND_ENUM_CONSTANT(OUTPUT_UNDERFLOWED);
    BIND_ENUM_CONSTANT(HOST_API_NOT_FOUND);
    BIND_ENUM_CONSTANT(INVALID_HOST_API);
    BIND_ENUM_CONSTANT(CAN_NOT_READ_FROM_A_CALLBACK_STREAM);
    BIND_ENUM_CONSTANT(CAN_NOT_WRITE_TO_A_CALLBACK_STREAM);
    BIND_ENUM_CONSTANT(CAN_NOT_READ_FROM_AN_OUT_PUTONLY_STREAM);
    BIND_ENUM_CONSTANT(CAN_NOT_WRITE_TO_AN_INPUT_ONLY_STREAM);
    BIND_ENUM_CONSTANT(INCOMPATIBLE_STREAM_HOST_API);
    BIND_ENUM_CONSTANT(BAD_BUFFER_PTR);

    // PortAudioCallbackResult
    BIND_ENUM_CONSTANT(CONTINUE);
    BIND_ENUM_CONSTANT(COMPLETE);
    BIND_ENUM_CONSTANT(ABORT);
}

PortAudio::PortAudio() {
    singleton = this;
    PortAudio::PortAudioError err = initialize();
    if (err != PortAudio::PortAudioError::NO_ERROR) {
        print_error(vformat("PortAudio::PortAudio: failed to initialize (%d)", err));
    }
}

PortAudio::~PortAudio() {
    PortAudio::PortAudioError err = terminate();
    if (err != PortAudio::PortAudioError::NO_ERROR) {
        print_error(vformat("PortAudio::PortAudio: failed to terminate (%d)", err));
    }
    data_map.clear();
}
