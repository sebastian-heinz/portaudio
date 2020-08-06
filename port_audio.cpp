#include "port_audio.h"

#include "port_audio_callback_data.h"

#include "portaudio/include/portaudio.h"

#pragma region IMP_DETAILS

class CallbackUserDataGdBinding {
public:
	PortAudio *port_audio;
	Ref<PortAudioStream> stream;
	Ref<FuncRef> audio_callback;
	Ref<FuncRef> stream_finished_callback;
	Ref<PortAudioCallbackData> audio_callback_data;
	CallbackUserDataGdBinding::CallbackUserDataGdBinding() {
		port_audio = NULL;
		stream = Ref<PortAudioStream>();
		audio_callback = Ref<FuncRef>();
		stream_finished_callback = Ref<FuncRef>();
		audio_callback_data = Ref<PortAudioCallbackData>();
	}
};

static int port_audio_callback_gd_binding_converter(const void *p_input_buffer, void *p_output_buffer,
		unsigned long p_frames_per_buffer, const PaStreamCallbackTimeInfo *p_time_info,
		PaStreamCallbackFlags p_status_flags, void *p_user_data) {

	CallbackUserDataGdBinding *user_data = (CallbackUserDataGdBinding *)p_user_data;
	if (!user_data) {
		print_line("PortAudio::port_audio_callback_converter: !user_data");
		return 0;
	}

	Ref<PortAudioCallbackData> audio_callback_data = user_data->audio_callback_data;

	// copy input buffer to godot type, if available
	if (p_input_buffer) {
		PoolVector<uint8_t> input_buffer;
		input_buffer.resize(p_frames_per_buffer); // TODO look for a solution withou allocating
		PoolVector<uint8_t>::Write write_input_buffer = input_buffer.write();
		uint8_t *write_input_buffer_ptr = write_input_buffer.ptr();
		copymem(write_input_buffer_ptr, p_input_buffer, p_frames_per_buffer);
		write_input_buffer.release();
	}

	// provide params
	audio_callback_data->set_input_buffer_adc_time(p_time_info->inputBufferAdcTime);
	audio_callback_data->set_current_time(p_time_info->currentTime);
	audio_callback_data->set_output_buffer_dac_time(p_time_info->outputBufferDacTime);
	audio_callback_data->set_frames_per_buffer(p_frames_per_buffer);
	audio_callback_data->set_status_flags(p_status_flags);

	// perform callback
	Variant::CallError call_error;
	Variant var_user_data = audio_callback_data;
	const Variant *var_user_data_ptr = &var_user_data;

	Array args;
	args.resize(1);
	args[0] = audio_callback_data;
	Variant result = user_data->audio_callback->call_funcv(args);

	//Variant result = user_data->audio_callback->call_func(&var_user_data_ptr, 1, call_error);
	//if (call_error.error != Variant::CallError::CALL_OK) {
	//	print_line(vformat("PortAudio::port_audio_callback_gd_binding_converter: call_error.error: %s", call_error.error));
	//	return 0;
	//}

	// copy result
	PoolVector<uint8_t> output_buffer = user_data->audio_callback_data->get_output_buffer();
	PoolVector<uint8_t>::Read read_output_buffer = output_buffer.read();
	const uint8_t *read_output_buffer_ptr = read_output_buffer.ptr();
	copymem(p_output_buffer, read_output_buffer_ptr, output_buffer.size());
	read_output_buffer.release();
	return result;
}

static void port_audio_stream_finished_callback_gd_binding_converter(void *p_user_data) {
	CallbackUserDataGdBinding *user_data = (CallbackUserDataGdBinding *)p_user_data;
	if (!user_data) {
		print_line("PortAudio::port_audio_stream_finished_callback_gd_binding_converter: !user_data");
		return;
	}
	Ref<FuncRef> stream_finished_callback = user_data->stream_finished_callback;
	if (stream_finished_callback.is_null()) {
		print_line("PortAudio::port_audio_stream_finished_callback_gd_binding_converter: stream_finished_callback.is_null())");
		return;
	}
	Variant::CallError call_error;
	Variant var_user_data = user_data->audio_callback_data;
	const Variant *var_user_data_ptr = &var_user_data;
	Variant result = stream_finished_callback->call_func(&var_user_data_ptr, 1, call_error);
	if (call_error.error != Variant::CallError::CALL_OK) {
		print_line(vformat("PortAudio::port_audio_stream_finished_callback_gd_binding_converter: call_error.error: %s", call_error.error));
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
		return (PortAudio::PortAudioError)p_error;
	}
	print_error(vformat("PortAudio::get_error: undefined error code: %d", p_error));
	return PortAudio::PortAudioError::UNDEFINED;
}

static PaSampleFormat get_sample_format(PortAudioStreamParameter::PortAudioSampleFormat p_sample_format) {
	switch (p_sample_format) {
		case PortAudioStreamParameter::PortAudioSampleFormat::FLOAT_32:
			return paFloat32;
		case PortAudioStreamParameter::PortAudioSampleFormat::INT_32:
			return paInt32;
		case PortAudioStreamParameter::PortAudioSampleFormat::INT_24:
			return paInt24;
		case PortAudioStreamParameter::PortAudioSampleFormat::INT_16:
			return paInt16;
		case PortAudioStreamParameter::PortAudioSampleFormat::INT_8:
			return paInt8;
		case PortAudioStreamParameter::PortAudioSampleFormat::U_INT_8:
			return paUInt8;
		case PortAudioStreamParameter::PortAudioSampleFormat::CUSTOM_FORMAT:
			return paCustomFormat;
		case PortAudioStreamParameter::PortAudioSampleFormat::NON_INTERLEAVED:
			return paNonInterleaved;
	}
	print_error(vformat("PortAudio::get_sample_format: undefined sample_format code: %d", p_sample_format));
	return paFloat32;
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
	Dictionary api_info;
	api_info["struct_version"] = pa_api_info->structVersion;
	api_info["type"] = (int)pa_api_info->type; // TODO enum
	api_info["name"] = String(pa_api_info->name);
	api_info["device_count"] = pa_api_info->deviceCount;
	api_info["default_input_device"] = pa_api_info->defaultInputDevice;
	api_info["default_output_device"] = pa_api_info->defaultOutputDevice;
	return api_info;
}

int PortAudio::host_api_type_id_to_host_api_index(int p_host_api_type_id) {
	return Pa_HostApiTypeIdToHostApiIndex((PaHostApiTypeId)p_host_api_type_id);
}

int PortAudio::host_api_device_index_to_device_index(int p_host_api, int p_host_api_device_index) {
	return Pa_HostApiDeviceIndexToDeviceIndex(p_host_api, p_host_api_device_index);
}

Dictionary PortAudio::get_last_host_error_info() {
	const PaHostErrorInfo *pa_host_error_info = Pa_GetLastHostErrorInfo();
	Dictionary host_error_info;
	host_error_info["host_api_type"] = (int)pa_host_error_info->hostApiType;
	host_error_info["error_code"] = (int64_t)pa_host_error_info->errorCode;
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
	const PaDeviceInfo *pa_device_info = Pa_GetDeviceInfo((PaDeviceIndex)p_device_index);
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

PortAudio::PortAudioError PortAudio::is_format_supported(Ref<PortAudioStreamParameter> p_input_stream_parameter, Ref<PortAudioStreamParameter> p_output_stream_parameter, double p_sample_rate) {
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

PortAudio::PortAudioError PortAudio::open_stream(Ref<PortAudioStream> p_stream, Ref<FuncRef> p_audio_callback, Variant p_user_data) {
	if (!p_audio_callback->is_valid()) {
		PortAudio::PortAudioError::INVALID_FUNC_REF;
	}
	const PaStreamParameters *pa_input_parameter_ptr = NULL;
	Ref<PortAudioStreamParameter> input_parameter = p_stream->get_input_stream_parameter();
	if (input_parameter.is_valid()) {
		const PaStreamParameters pa_input_parameter = {
			input_parameter->get_device_index(),
			input_parameter->get_channel_count(),
			get_sample_format(input_parameter->get_sample_format()),
			input_parameter->get_suggested_latency(),
			input_parameter->get_host_api_specific_stream_info(),
		};
		pa_input_parameter_ptr = &pa_input_parameter;
	}
	const PaStreamParameters *pa_output_parameter_ptr = NULL;
	Ref<PortAudioStreamParameter> output_parameter = p_stream->get_output_stream_parameter();
	if (output_parameter.is_valid()) {
		const PaStreamParameters pa_output_parameter = {
			output_parameter->get_device_index(),
			output_parameter->get_channel_count(),
			get_sample_format(output_parameter->get_sample_format()),
			output_parameter->get_suggested_latency(),
			output_parameter->get_host_api_specific_stream_info(),
		};
		pa_output_parameter_ptr = &pa_output_parameter;
	}
	// TODO delete(p_audio_callback) - maybe add to list & add finished flag and clean up periodically
	CallbackUserDataGdBinding *user_data = new CallbackUserDataGdBinding();
	user_data->port_audio = this;
	user_data->audio_callback = p_audio_callback;
	user_data->audio_callback_data.instance();
	user_data->audio_callback_data->set_user_data(p_user_data);
	user_data->audio_callback_data->get_output_buffer_ptr()->resize(1024);
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
	}
	return get_error(err);
}

PortAudio::PortAudioError PortAudio::open_default_stream(Ref<PortAudioStream> p_stream, Ref<FuncRef> p_audio_callback, Variant p_user_data) {
	if (!p_audio_callback->is_valid()) {
		PortAudio::PortAudioError::INVALID_FUNC_REF;
	}
	const PaStreamParameters *pa_input_parameter_ptr = NULL;
	Ref<PortAudioStreamParameter> input_parameter = p_stream->get_input_stream_parameter();
	if (input_parameter.is_valid()) {
		const PaStreamParameters pa_input_parameter = {
			input_parameter->get_device_index(),
			input_parameter->get_channel_count(),
			get_sample_format(input_parameter->get_sample_format()),
			input_parameter->get_suggested_latency(),
			input_parameter->get_host_api_specific_stream_info(),
		};
		pa_input_parameter_ptr = &pa_input_parameter;
	}
	const PaStreamParameters *pa_output_parameter_ptr = NULL;
	Ref<PortAudioStreamParameter> output_parameter = p_stream->get_output_stream_parameter();
	if (output_parameter.is_valid()) {
		const PaStreamParameters pa_output_parameter = {
			output_parameter->get_device_index(),
			output_parameter->get_channel_count(),
			get_sample_format(output_parameter->get_sample_format()),
			output_parameter->get_suggested_latency(),
			output_parameter->get_host_api_specific_stream_info(),
		};
		pa_output_parameter_ptr = &pa_output_parameter;
	}
	// TODO delete(p_audio_callback) - maybe add to list & add finished flag and clean up periodically
	CallbackUserDataGdBinding *user_data = new CallbackUserDataGdBinding();
	user_data->port_audio = this;
	user_data->audio_callback = p_audio_callback;
	user_data->audio_callback_data.instance();
	user_data->audio_callback_data->set_user_data(p_user_data);
	user_data->audio_callback_data->get_output_buffer_ptr()->resize(1024);

	PaStream *stream;
	PaError err = Pa_OpenDefaultStream(&stream,
			p_stream->get_input_channel_count(),
			p_stream->get_output_channel_count(),
			get_sample_format(p_stream->get_sample_format()),
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
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaError err = Pa_StartStream(stream);
	return get_error(err);
}

PortAudio::PortAudioError PortAudio::stop_stream(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaError err = Pa_StopStream(stream);
	return get_error(err);
}

PortAudio::PortAudioError PortAudio::abort_stream(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaError err = Pa_AbortStream(stream);
	return get_error(err);
}

PortAudio::PortAudioError PortAudio::close_stream(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaError err = Pa_CloseStream(stream);
	return get_error(err);
}

PortAudio::PortAudioError PortAudio::set_stream_finished_callback(Ref<PortAudioStream> p_stream, Ref<FuncRef> p_stream_finished_callback) {
	std::map<Ref<PortAudioStream>, void *>::iterator it = data_map.find(p_stream);
	if (it == data_map.end()) {
		print_line("PortAudio::set_stream_finished_callback: stream not found");
		return PortAudioError::STREAM_NOT_FOUND;
	}
	CallbackUserDataGdBinding *user_data = (CallbackUserDataGdBinding *)it->second;
	if (!user_data) {
		print_line("PortAudio::set_stream_finished_callback: !user_data");
		return PortAudioError::STREAM_USER_DATA_NOT_FOUND;
	}
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaStreamFinishedCallback *pa_stream_finished;
	if (p_stream_finished_callback->is_valid()) {
		pa_stream_finished = &port_audio_stream_finished_callback_gd_binding_converter;
		user_data->stream_finished_callback = p_stream_finished_callback;
	} else {
		// unregister callback
		pa_stream_finished = NULL;
		if (user_data->stream_finished_callback.is_valid()) {
			user_data->stream_finished_callback.unref();
		}
	}
	PaError err = Pa_SetStreamFinishedCallback(stream, pa_stream_finished);
	return get_error(err);
}

PortAudio::PortAudioError PortAudio::is_stream_stopped(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaError err = Pa_IsStreamStopped(stream);
	return get_error(err);
}

PortAudio::PortAudioError PortAudio::is_stream_active(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaError err = Pa_IsStreamActive(stream);
	return get_error(err);
}

double PortAudio::get_stream_time(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaTime time = Pa_GetStreamTime(stream);
	return time;
}

Dictionary PortAudio::get_stream_info(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	const PaStreamInfo *pa_stream_info = Pa_GetStreamInfo(stream);
	Dictionary stream_info;
	stream_info["struct_version"] = pa_stream_info->structVersion;
	stream_info["input_latency"] = pa_stream_info->inputLatency;
	stream_info["output_latency"] = pa_stream_info->outputLatency;
	stream_info["sample_rate"] = pa_stream_info->sampleRate;
	return stream_info;
}

double PortAudio::get_stream_cpu_load(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	return Pa_GetStreamCpuLoad(stream);
}

PortAudio::PortAudioError PortAudio::read_stream(Ref<PortAudioStream> p_stream, PoolVector<uint8_t> p_buffer, uint64_t p_frames) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PoolVector<uint8_t>::Write buffer_write = p_buffer.write();
	void *buffer = buffer_write.ptr();
	PaError err = Pa_ReadStream(stream, buffer, p_frames);
	buffer_write.release();
	return get_error(err);
}

PortAudio::PortAudioError PortAudio::write_stream(Ref<PortAudioStream> p_stream, PoolVector<uint8_t> p_buffer, uint64_t p_frames) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PoolVector<uint8_t>::Read buffer_read = p_buffer.read();
	void *buffer = (void *)buffer_read.ptr();
	PaError err = Pa_WriteStream(stream, buffer, p_frames);
	buffer_read.release();
	return get_error(err);
}

int64_t PortAudio::get_stream_read_available(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	signed long available = Pa_GetStreamReadAvailable(stream);
	return available;
}

int64_t PortAudio::get_stream_write_available(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
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

void PortAudio::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_version"), &PortAudio::get_version);
	ClassDB::bind_method(D_METHOD("get_version_text"), &PortAudio::get_version_text);
	ClassDB::bind_method(D_METHOD("get_version_info"), &PortAudio::get_version_info);
	ClassDB::bind_method(D_METHOD("get_error_text", "error"), &PortAudio::get_error_text);
	ClassDB::bind_method(D_METHOD("initialize"), &PortAudio::initialize);
	ClassDB::bind_method(D_METHOD("terminate"), &PortAudio::terminate);
	ClassDB::bind_method(D_METHOD("get_host_api_count"), &PortAudio::get_host_api_count);
	ClassDB::bind_method(D_METHOD("get_default_host_api"), &PortAudio::get_default_host_api);
	ClassDB::bind_method(D_METHOD("get_host_api_info", "host_api"), &PortAudio::get_host_api_info);
	ClassDB::bind_method(D_METHOD("host_api_type_id_to_host_api_index", "host_api_type_id"), &PortAudio::host_api_type_id_to_host_api_index);
	ClassDB::bind_method(D_METHOD("host_api_device_index_to_device_index", "host_api", "host_api_device_index"), &PortAudio::host_api_device_index_to_device_index);
	ClassDB::bind_method(D_METHOD("get_last_host_error_info"), &PortAudio::get_last_host_error_info);
	ClassDB::bind_method(D_METHOD("get_device_count"), &PortAudio::get_device_count);
	ClassDB::bind_method(D_METHOD("get_default_input_device"), &PortAudio::get_default_input_device);
	ClassDB::bind_method(D_METHOD("get_default_output_device"), &PortAudio::get_default_output_device);
	ClassDB::bind_method(D_METHOD("get_device_info", "device_index"), &PortAudio::get_device_info);
	ClassDB::bind_method(D_METHOD("is_format_supported", "input_stream_parameter", "output_stream_parameter", "sample_rate"), &PortAudio::is_format_supported);
	ClassDB::bind_method(D_METHOD("open_stream", "stream", "audio_callback", "user_data"), &PortAudio::open_stream);
	ClassDB::bind_method(D_METHOD("open_default_stream", "stream", "audio_callback", "user_data"), &PortAudio::open_default_stream);
	ClassDB::bind_method(D_METHOD("close_stream", "stream"), &PortAudio::close_stream);
	ClassDB::bind_method(D_METHOD("set_stream_finished_callback", "stream", "stream_finished_callback"), &PortAudio::set_stream_finished_callback);
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

	// PortAudioError
	BIND_ENUM_CONSTANT(UNDEFINED);
	BIND_ENUM_CONSTANT(NOT_PORT_AUDIO_NODE);

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
