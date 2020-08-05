#include "port_audio.h"

#include "port_audio_node.h"

#include "portaudio/include/portaudio.h"

#pragma region IMP_DETAILS
class PaCallbackUserData {
public:
	PortAudio *port_audio;
	PortAudio::AudioCallback *audio_callback;
	void *audio_callback_user_data;
	PoolVector<uint8_t> output_buffer;
	PaCallbackUserData::PaCallbackUserData() {
		port_audio = NULL;
		audio_callback = NULL;
		audio_callback_user_data = NULL;
		output_buffer = PoolVector<uint8_t>();
	}
};

static int port_audio_callback_converter(const void *p_input_buffer, void *p_output_buffer,
		unsigned long p_frames_per_buffer, const PaStreamCallbackTimeInfo *p_time_info,
		PaStreamCallbackFlags p_status_flags, void *p_user_data) {

	PaCallbackUserData *user_data = (PaCallbackUserData *)p_user_data;
	if (!user_data) {
		print_line("PortAudio::port_audio_callback_converter: !user_data");
		return 0;
	}

	// copy input buffer to godot type, if available
	PoolVector<uint8_t> input_buffer;
	if (p_input_buffer) {
		input_buffer.resize(p_frames_per_buffer); // TODO look for a solution withou allocating
		PoolVector<uint8_t>::Write write_input_buffer = input_buffer.write();
		uint8_t *write_input_buffer_ptr = write_input_buffer.ptr();
		copymem(write_input_buffer_ptr, p_input_buffer, p_frames_per_buffer);
		write_input_buffer.release();
	}

	// provide time info
	Dictionary time_info;
	time_info["input_buffer_adc_time"] = p_time_info->inputBufferAdcTime;
	time_info["current_time"] = p_time_info->currentTime;
	time_info["output_buffer_dac_time"] = p_time_info->outputBufferDacTime;

	PoolVector<uint8_t> output_buffer = user_data->output_buffer;

	// perform callback
	PortAudio::AudioCallback *audio_callback = (PortAudio::AudioCallback *)user_data->audio_callback;
	int result = audio_callback(input_buffer, output_buffer, p_frames_per_buffer, time_info, p_status_flags, user_data->audio_callback_user_data);

	// copy result
	PoolVector<uint8_t>::Read read_output_buffer = output_buffer.read();
	const uint8_t *read_output_buffer_ptr = read_output_buffer.ptr();
	copymem(p_output_buffer, read_output_buffer_ptr, output_buffer.size());
	read_output_buffer.release();

	return result;
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

static PaSampleFormat get_sample_format(PortAudio::PortAudioSampleFormat p_sample_format) {
	switch (p_sample_format) {
		case PortAudio::PortAudioSampleFormat::FLOAT_32:
			return paFloat32;
		case PortAudio::PortAudioSampleFormat::INT_32:
			return paInt32;
		case PortAudio::PortAudioSampleFormat::INT_24:
			return paInt24;
		case PortAudio::PortAudioSampleFormat::INT_16:
			return paInt16;
		case PortAudio::PortAudioSampleFormat::INT_8:
			return paInt8;
		case PortAudio::PortAudioSampleFormat::U_INT_8:
			return paUInt8;
		case PortAudio::PortAudioSampleFormat::CUSTOM_FORMAT:
			return paCustomFormat;
		case PortAudio::PortAudioSampleFormat::NON_INTERLEAVED:
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

/** Retrieve the release number of the currently running PortAudio build.
 For example, for version "19.5.1" this will return 0x00130501.

 @see paMakeVersionNumber
*/
int PortAudio::get_version() {
	return Pa_GetVersion();
}

/** Retrieve a textual description of the current PortAudio build,
 e.g. "PortAudio V19.5.0-devel, revision 1952M".
 The format of the text may change in the future. Do not try to parse the
 returned string.

 @deprecated As of 19.5.0, use Pa_GetVersionInfo()->versionText instead.
*/
String PortAudio::get_version_text() {
	return String(Pa_GetVersionText());
}

/** Retrieve version information for the currently running PortAudio build.
 @return A pointer to an immutable PaVersionInfo structure.

 @note This function can be called at any time. It does not require PortAudio
 to be initialized. The structure pointed to is statically allocated. Do not
 attempt to free it or modify it.

 @see PaVersionInfo, paMakeVersionNumber
 @version Available as of 19.5.0.
*/
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
/**
 * Translate the supplied PortAudio error code into a human readable message.
*/
String PortAudio::get_error_text(PortAudio::PortAudioError p_error) {
	switch (p_error) {
		case UNDEFINED:
			return "Undefined error code";
		case NOT_PORT_AUDIO_NODE:
			return "Provided node is not of type PortAudioNode";
	}
	return String(Pa_GetErrorText(p_error));
}

/** Library initialization function - call this before using PortAudio.
 This function initializes internal data structures and prepares underlying
 host APIs for use.  With the exception of Pa_GetVersion(), Pa_GetVersionText(),
 and Pa_GetErrorText(), this function MUST be called before using any other
 PortAudio API functions.

 If Pa_Initialize() is called multiple times, each successful 
 call must be matched with a corresponding call to Pa_Terminate(). 
 Pairs of calls to Pa_Initialize()/Pa_Terminate() may overlap, and are not 
 required to be fully nested.

 Note that if Pa_Initialize() returns an error code, Pa_Terminate() should
 NOT be called.

 @return paNoError if successful, otherwise an error code indicating the cause
 of failure.

 @see Pa_Terminate
*/
PortAudio::PortAudioError PortAudio::initialize() {
	PaError err = Pa_Initialize();
	return get_error(err);
}

/** Library termination function - call this when finished using PortAudio.
 This function deallocates all resources allocated by PortAudio since it was
 initialized by a call to Pa_Initialize(). In cases where Pa_Initialise() has
 been called multiple times, each call must be matched with a corresponding call
 to Pa_Terminate(). The final matching call to Pa_Terminate() will automatically
 close any PortAudio streams that are still open.

 Pa_Terminate() MUST be called before exiting a program which uses PortAudio.
 Failure to do so may result in serious resource leaks, such as audio devices
 not being available until the next reboot.

 @return paNoError if successful, otherwise an error code indicating the cause
 of failure.
 
 @see Pa_Initialize
*/
PortAudio::PortAudioError PortAudio::terminate() {
	PaError err = Pa_Terminate();
	return get_error(err);
}

/** Retrieve the number of available host APIs. Even if a host API is
 available it may have no devices available.

 @return A non-negative value indicating the number of available host APIs
 or, a PaErrorCode (which are always negative) if PortAudio is not initialized
 or an error is encountered.

 @see PaHostApiIndex
*/
int PortAudio::get_host_api_count() {
	return Pa_GetHostApiCount();
}

/** Retrieve the index of the default host API. The default host API will be
 the lowest common denominator host API on the current platform and is
 unlikely to provide the best performance.

 @return A non-negative value ranging from 0 to (Pa_GetHostApiCount()-1)
 indicating the default host API index or, a PaErrorCode (which are always
 negative) if PortAudio is not initialized or an error is encountered.
*/
int PortAudio::get_default_host_api() {
	return Pa_GetDefaultHostApi();
}

/** Retrieve a pointer to a structure containing information about a specific
 host Api.

 @param hostApi A valid host API index ranging from 0 to (Pa_GetHostApiCount()-1)

 @return A pointer to an immutable PaHostApiInfo structure describing
 a specific host API. If the hostApi parameter is out of range or an error
 is encountered, the function returns NULL.

 The returned structure is owned by the PortAudio implementation and must not
 be manipulated or freed. The pointer is only guaranteed to be valid between
 calls to Pa_Initialize() and Pa_Terminate().
*/
Dictionary PortAudio::get_host_api_info(int p_host_api) {
	const PaHostApiInfo *pa_api_info = Pa_GetHostApiInfo(p_host_api);

	Dictionary api_info;

	/** this is struct version 1 */
	api_info["struct_version"] = pa_api_info->structVersion;

	/** The well known unique identifier of this host API @see PaHostApiTypeId */
	api_info["type"] = (int)pa_api_info->type; // TODO enum

	/** A textual description of the host API for display on user interfaces. */
	api_info["name"] = String(pa_api_info->name);

	/**  The number of devices belonging to this host API. This field may be
     used in conjunction with Pa_HostApiDeviceIndexToDeviceIndex() to enumerate
     all devices for this host API.
     @see Pa_HostApiDeviceIndexToDeviceIndex
    */
	api_info["device_count"] = pa_api_info->deviceCount;

	/** The default input device for this host API. The value will be a
     device index ranging from 0 to (Pa_GetDeviceCount()-1), or paNoDevice
     if no default input device is available.
    */
	api_info["default_input_device"] = pa_api_info->defaultInputDevice;

	/** The default output device for this host API. The value will be a
     device index ranging from 0 to (Pa_GetDeviceCount()-1), or paNoDevice
     if no default output device is available.
    */
	api_info["default_output_device"] = pa_api_info->defaultOutputDevice;

	return api_info;
}

/** Convert a static host API unique identifier, into a runtime
 host API index.

 @param type A unique host API identifier belonging to the PaHostApiTypeId
 enumeration.

 @return A valid PaHostApiIndex ranging from 0 to (Pa_GetHostApiCount()-1) or,
 a PaErrorCode (which are always negative) if PortAudio is not initialized
 or an error is encountered.
 
 The paHostApiNotFound error code indicates that the host API specified by the
 type parameter is not available.

 @see PaHostApiTypeId
*/
int PortAudio::host_api_type_id_to_host_api_index(int p_host_api_type_id) {
	return Pa_HostApiTypeIdToHostApiIndex((PaHostApiTypeId)p_host_api_type_id);
}

/** Convert a host-API-specific device index to standard PortAudio device index.
 This function may be used in conjunction with the deviceCount field of
 PaHostApiInfo to enumerate all devices for the specified host API.

 @param hostApi A valid host API index ranging from 0 to (Pa_GetHostApiCount()-1)

 @param hostApiDeviceIndex A valid per-host device index in the range
 0 to (Pa_GetHostApiInfo(hostApi)->deviceCount-1)

 @return A non-negative PaDeviceIndex ranging from 0 to (Pa_GetDeviceCount()-1)
 or, a PaErrorCode (which are always negative) if PortAudio is not initialized
 or an error is encountered.

 A paInvalidHostApi error code indicates that the host API index specified by
 the hostApi parameter is out of range.

 A paInvalidDevice error code indicates that the hostApiDeviceIndex parameter
 is out of range.
 
 @see PaHostApiInfo
*/
int PortAudio::host_api_device_index_to_device_index(int p_host_api, int p_host_api_device_index) {
	return Pa_HostApiDeviceIndexToDeviceIndex(p_host_api, p_host_api_device_index);
}

/** Return information about the last host error encountered. The error
 information returned by Pa_GetLastHostErrorInfo() will never be modified
 asynchronously by errors occurring in other PortAudio owned threads
 (such as the thread that manages the stream callback.)

 This function is provided as a last resort, primarily to enhance debugging
 by providing clients with access to all available error information.

 @return A pointer to an immutable structure constraining information about
 the host error. The values in this structure will only be valid if a
 PortAudio function has previously returned the paUnanticipatedHostError
 error code.
*/
Dictionary PortAudio::get_last_host_error_info() {
	const PaHostErrorInfo *pa_host_error_info = Pa_GetLastHostErrorInfo();
	Dictionary host_error_info;
	/**< the host API which returned the error code */
	host_error_info["host_api_type"] = (int)pa_host_error_info->hostApiType;
	/**< the error code returned */
	host_error_info["error_code"] = (int64_t)pa_host_error_info->errorCode;
	/**< a textual description of the error if available, otherwise a zero-length string */
	host_error_info["error_text"] = String(pa_host_error_info->errorText);
	return host_error_info;
}

/* Device enumeration and capabilities */

/** Retrieve the number of available devices. The number of available devices
 may be zero.

 @return A non-negative value indicating the number of available devices or,
 a PaErrorCode (which are always negative) if PortAudio is not initialized
 or an error is encountered.
*/
int PortAudio::get_device_count() {
	return Pa_GetDeviceCount();
}

/** Retrieve the index of the default input device. The result can be
 used in the inputDevice parameter to Pa_OpenStream().

 @return The default input device index for the default host API, or paNoDevice
 if no default input device is available or an error was encountered.
*/
int PortAudio::get_default_input_device() {
	return Pa_GetDefaultInputDevice();
}

/** Retrieve the index of the default output device. The result can be
 used in the outputDevice parameter to Pa_OpenStream().

 @return The default output device index for the default host API, or paNoDevice
 if no default output device is available or an error was encountered.

 @note
 On the PC, the user can specify a default device by
 setting an environment variable. For example, to use device #1.
<pre>
 set PA_RECOMMENDED_OUTPUT_DEVICE=1
</pre>
 The user should first determine the available device ids by using
 the supplied application "pa_devs".
*/
int PortAudio::get_default_output_device() {
	return Pa_GetDefaultOutputDevice();
}

Dictionary PortAudio::get_device_info(int p_device_index) {

	const PaDeviceInfo *pa_device_info = Pa_GetDeviceInfo((PaDeviceIndex)p_device_index);

	Dictionary device_info;

	/* this is struct version 2 */
	device_info["struct_version"] = pa_device_info->structVersion;

	device_info["name"] = String(pa_device_info->name);

	/**< note this is a host API index, not a type id*/
	device_info["host_api"] = pa_device_info->hostApi;
	device_info["max_input_channels"] = pa_device_info->maxInputChannels;
	device_info["max_output_channels"] = pa_device_info->maxOutputChannels;

	/** Default latency values for interactive performance. */
	device_info["default_low_input_latency"] = pa_device_info->defaultLowInputLatency;
	device_info["default_low_output_latency"] = pa_device_info->defaultLowOutputLatency;

	/** Default latency values for robust non-interactive applications (eg. playing sound files). */
	device_info["default_high_input_latency"] = pa_device_info->defaultHighInputLatency;
	device_info["default_high_output_latency"] = pa_device_info->defaultHighOutputLatency;

	device_info["default_sample_rate"] = pa_device_info->defaultSampleRate;

	return device_info;
}

PortAudio::PortAudioError PortAudio::is_format_supported(Ref<PortAudioStreamParameter> p_input_stream_parameter, Ref<PortAudioStreamParameter> p_output_stream_parameter, double p_sample_rate) {

	const PaStreamParameters pa_input_parameter = {
		p_input_stream_parameter->get_device_index(),
		p_input_stream_parameter->get_channel_count(),
		// TODO warning C4838: conversion from 'uint64_t' to 'PaSampleFormat' requires a narrowing conversion
		p_input_stream_parameter->get_sample_format(),
		p_input_stream_parameter->get_suggested_latency(),
		p_input_stream_parameter->get_host_api_specific_stream_info(),
	};

	const PaStreamParameters pa_output_parameter = {
		p_output_stream_parameter->get_device_index(),
		p_output_stream_parameter->get_channel_count(),
		// TODO warning C4838 : conversion from 'uint64_t' to 'PaSampleFormat' requires a narrowing conversion
		p_output_stream_parameter->get_sample_format(),
		p_output_stream_parameter->get_suggested_latency(),
		p_output_stream_parameter->get_host_api_specific_stream_info(),
	};

	PaError err = Pa_IsFormatSupported(&pa_input_parameter, &pa_output_parameter, p_sample_rate);
	return get_error(err);
}

PortAudio::PortAudioError PortAudio::open_stream(Ref<PortAudioStream> p_stream, AudioCallback *p_audio_callback, void *p_user_data) {

	Ref<PortAudioStreamParameter> input_parameter = p_stream->get_input_stream_parameter();
	const PaStreamParameters pa_input_parameter = {
		input_parameter->get_device_index(),
		input_parameter->get_channel_count(),
		// TODO warning C4838: conversion from 'uint64_t' to 'PaSampleFormat' requires a narrowing conversion
		input_parameter->get_sample_format(),
		input_parameter->get_suggested_latency(),
		input_parameter->get_host_api_specific_stream_info(),
	};

	Ref<PortAudioStreamParameter> output_parameter = p_stream->get_output_stream_parameter();
	const PaStreamParameters pa_output_parameter = {
		output_parameter->get_device_index(),
		output_parameter->get_channel_count(),
		// TODO warning C4838: conversion from 'uint64_t' to 'PaSampleFormat' requires a narrowing conversion
		output_parameter->get_sample_format(),
		output_parameter->get_suggested_latency(),
		output_parameter->get_host_api_specific_stream_info(),
	};

	// TODO delete(p_audio_callback) - maybe add to list & add finished flag and clean up periodically
	PaCallbackUserData *user_data = new PaCallbackUserData();
	user_data->port_audio = this;
	user_data->audio_callback = p_audio_callback;
	user_data->audio_callback_user_data = p_user_data;
	user_data->output_buffer.resize(output_buffer_size);

	PaStream *stream;
	PaError err = Pa_OpenStream(&stream,
			&pa_input_parameter,
			&pa_output_parameter,
			p_stream->get_sample_rate(),
			p_stream->get_frames_per_buffer(),
			p_stream->get_stream_flags(),
			&port_audio_callback_converter,
			user_data);
	if (err == PaErrorCode::paNoError) {
		p_stream->set_stream(stream);
	}
	return get_error(err);
}

PortAudio::PortAudioError PortAudio::open_default_stream(Ref<PortAudioStream> p_stream, AudioCallback *p_audio_callback, void *p_user_data) {

	PaCallbackUserData *user_data = new PaCallbackUserData();
	user_data->port_audio = this;
	user_data->audio_callback = p_audio_callback;
	user_data->audio_callback_user_data = p_user_data;
	user_data->output_buffer.resize(output_buffer_size);

	PaStream *stream;
	PaError err = Pa_OpenDefaultStream(&stream,
			p_stream->get_input_channel_count(),
			p_stream->get_output_channel_count(),
			paFloat32,
			p_stream->get_sample_rate(),
			p_stream->get_frames_per_buffer(),
			&port_audio_callback_converter,
			user_data);
	if (err == PaErrorCode::paNoError) {
		p_stream->set_stream(stream);
	}
	return get_error(err);
}

/**
Commences audio processing.
*/
PortAudio::PortAudioError PortAudio::start_stream(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaError err = Pa_StartStream(stream);
	return get_error(err);
}

/**
Terminates audio processing. It waits until all pending
 audio buffers have been played before it returns.
*/
PortAudio::PortAudioError PortAudio::stop_stream(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaError err = Pa_StopStream(stream);
	return get_error(err);
}

/** Terminates audio processing immediately without waiting for pending
 buffers to complete.
*/
PortAudio::PortAudioError PortAudio::abort_stream(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaError err = Pa_AbortStream(stream);
	return get_error(err);
}

/** Closes an audio stream. If the audio stream is active it
 discards any pending buffers as if Pa_AbortStream() had been called.
*/
PortAudio::PortAudioError PortAudio::close_stream(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaError err = Pa_CloseStream(stream);
	return get_error(err);
}

/** Register a stream finished callback function which will be called when the 
 stream becomes inactive. See the description of PaStreamFinishedCallback for 
 further details about when the callback will be called.

 @param stream a pointer to a PaStream that is in the stopped state - if the
 stream is not stopped, the stream's finished callback will remain unchanged 
 and an error code will be returned.

 @param streamFinishedCallback a pointer to a function with the same signature
 as PaStreamFinishedCallback, that will be called when the stream becomes
 inactive. Passing NULL for this parameter will un-register a previously
 registered stream finished callback function.

 @return on success returns paNoError, otherwise an error code indicating the cause
 of the error.

 @see PaStreamFinishedCallback
*/
PortAudio::PortAudioError PortAudio::set_stream_finished_callback(Ref<PortAudioStream> p_stream, StreamFinishedCallback *p_stream_finished_callback) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaError err = Pa_SetStreamFinishedCallback(stream, p_stream_finished_callback);
	return get_error(err);
}

/** Determine whether the stream is stopped.
 A stream is considered to be stopped prior to a successful call to
 Pa_StartStream and after a successful call to Pa_StopStream or Pa_AbortStream.
 If a stream callback returns a value other than paContinue the stream is NOT
 considered to be stopped.

 @return Returns one (1) when the stream is stopped, zero (0) when
 the stream is running or, a PaErrorCode (which are always negative) if
 PortAudio is not initialized or an error is encountered.

 @see Pa_StopStream, Pa_AbortStream, Pa_IsStreamActive
*/
PortAudio::PortAudioError PortAudio::is_stream_stopped(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaError err = Pa_IsStreamStopped(stream);
	return get_error(err);
}

/** Determine whether the stream is active.
 A stream is active after a successful call to Pa_StartStream(), until it
 becomes inactive either as a result of a call to Pa_StopStream() or
 Pa_AbortStream(), or as a result of a return value other than paContinue from
 the stream callback. In the latter case, the stream is considered inactive
 after the last buffer has finished playing.

 @return Returns one (1) when the stream is active (ie playing or recording
 audio), zero (0) when not playing or, a PaErrorCode (which are always negative)
 if PortAudio is not initialized or an error is encountered.

 @see Pa_StopStream, Pa_AbortStream, Pa_IsStreamStopped
*/
PortAudio::PortAudioError PortAudio::is_stream_active(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaError err = Pa_IsStreamActive(stream);
	return get_error(err);
}

/** Returns the current time in seconds for a stream according to the same clock used
 to generate callback PaStreamCallbackTimeInfo timestamps. The time values are
 monotonically increasing and have unspecified origin. 
 
 Pa_GetStreamTime returns valid time values for the entire life of the stream,
 from when the stream is opened until it is closed. Starting and stopping the stream
 does not affect the passage of time returned by Pa_GetStreamTime.

 This time may be used for synchronizing other events to the audio stream, for 
 example synchronizing audio to MIDI.
                                        
 @return The stream's current time in seconds, or 0 if an error occurred.

 @see PaTime, PaStreamCallback, PaStreamCallbackTimeInfo
*/
double PortAudio::get_stream_time(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PaTime time = Pa_GetStreamTime(stream);
	return time;
}

/** Retrieve a pointer to a PaStreamInfo structure containing information
 about the specified stream.
 @return A pointer to an immutable PaStreamInfo structure. If the stream
 parameter is invalid, or an error is encountered, the function returns NULL.

 @param stream A pointer to an open stream previously created with Pa_OpenStream.

 @note PortAudio manages the memory referenced by the returned pointer,
 the client must not manipulate or free the memory. The pointer is only
 guaranteed to be valid until the specified stream is closed.

 @see PaStreamInfo
*/
Dictionary PortAudio::get_stream_info(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	const PaStreamInfo *pa_stream_info = Pa_GetStreamInfo(stream);

	Dictionary stream_info;
	/** this is struct version 1 */
	stream_info["struct_version"] = pa_stream_info->structVersion;

	/** The input latency of the stream in seconds. This value provides the most
     accurate estimate of input latency available to the implementation. It may
     differ significantly from the suggestedLatency value passed to Pa_OpenStream().
     The value of this field will be zero (0.) for output-only streams.
     @see PaTime
    */
	stream_info["input_latency"] = pa_stream_info->inputLatency;

	/** The output latency of the stream in seconds. This value provides the most
     accurate estimate of output latency available to the implementation. It may
     differ significantly from the suggestedLatency value passed to Pa_OpenStream().
     The value of this field will be zero (0.) for input-only streams.
     @see PaTime
    */
	stream_info["output_latency"] = pa_stream_info->outputLatency;

	/** The sample rate of the stream in Hertz (samples per second). In cases
     where the hardware sample rate is inaccurate and PortAudio is aware of it,
     the value of this field may be different from the sampleRate parameter
     passed to Pa_OpenStream(). If information about the actual hardware sample
     rate is not available, this field will have the same value as the sampleRate
     parameter passed to Pa_OpenStream().
    */
	stream_info["sample_rate"] = pa_stream_info->sampleRate;

	return stream_info;
}

double PortAudio::get_stream_cpu_load(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	return Pa_GetStreamCpuLoad(stream);
}

/** Read samples from an input stream. The function doesn't return until
 the entire buffer has been filled - this may involve waiting for the operating
 system to supply the data.

 @param stream A pointer to an open stream previously created with Pa_OpenStream.
 
 @param buffer A pointer to a buffer of sample frames. The buffer contains
 samples in the format specified by the inputParameters->sampleFormat field
 used to open the stream, and the number of channels specified by
 inputParameters->numChannels. If non-interleaved samples were requested using
 the paNonInterleaved sample format flag, buffer is a pointer to the first element 
 of an array of buffer pointers, one non-interleaved buffer for each channel.

 @param frames The number of frames to be read into buffer. This parameter
 is not constrained to a specific range, however high performance applications
 will want to match this parameter to the framesPerBuffer parameter used
 when opening the stream.

 @return On success PaNoError will be returned, or PaInputOverflowed if input
 data was discarded by PortAudio after the previous call and before this call.
*/
PortAudio::PortAudioError PortAudio::read_stream(Ref<PortAudioStream> p_stream, PoolVector<uint8_t> p_buffer, uint64_t p_frames) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PoolVector<uint8_t>::Write buffer_write = p_buffer.write();
	void *buffer = buffer_write.ptr();
	PaError err = Pa_ReadStream(stream, buffer, p_frames);
	buffer_write.release();
	return get_error(err);
}

/** Write samples to an output stream. This function doesn't return until the
 entire buffer has been written - this may involve waiting for the operating
 system to consume the data.

 @param stream A pointer to an open stream previously created with Pa_OpenStream.

 @param buffer A pointer to a buffer of sample frames. The buffer contains
 samples in the format specified by the outputParameters->sampleFormat field
 used to open the stream, and the number of channels specified by
 outputParameters->numChannels. If non-interleaved samples were requested using
 the paNonInterleaved sample format flag, buffer is a pointer to the first element 
 of an array of buffer pointers, one non-interleaved buffer for each channel.

 @param frames The number of frames to be written from buffer. This parameter
 is not constrained to a specific range, however high performance applications
 will want to match this parameter to the framesPerBuffer parameter used
 when opening the stream.

 @return On success PaNoError will be returned, or paOutputUnderflowed if
 additional output data was inserted after the previous call and before this
 call.
*/
PortAudio::PortAudioError PortAudio::write_stream(Ref<PortAudioStream> p_stream, PoolVector<uint8_t> p_buffer, uint64_t p_frames) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	PoolVector<uint8_t>::Read buffer_read = p_buffer.read();
	void *buffer = (void *)buffer_read.ptr();
	PaError err = Pa_WriteStream(stream, buffer, p_frames);
	buffer_read.release();
	return get_error(err);
}

/** Retrieve the number of frames that can be read from the stream without
 waiting.

 @return Returns a non-negative value representing the maximum number of frames
 that can be read from the stream without blocking or busy waiting or, a
 PaErrorCode (which are always negative) if PortAudio is not initialized or an
 error is encountered.
*/
int64_t PortAudio::get_stream_read_available(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	signed long available = Pa_GetStreamReadAvailable(stream);
	return available;
}

/** Retrieve the number of frames that can be written to the stream without
 waiting.

 @return Returns a non-negative value representing the maximum number of frames
 that can be written to the stream without blocking or busy waiting or, a
 PaErrorCode (which are always negative) if PortAudio is not initialized or an
 error is encountered.
*/
int64_t PortAudio::get_stream_write_available(Ref<PortAudioStream> p_stream) {
	PaStream *stream = (PaStream *)p_stream->get_stream();
	signed long available = Pa_GetStreamWriteAvailable(stream);
	return available;
}

/** Retrieve the size of a given sample format in bytes.

 @return The size in bytes of a single sample in the specified format,
 or paSampleFormatNotSupported if the format is not supported.
*/
PortAudio::PortAudioError PortAudio::get_sample_size(PortAudio::PortAudioSampleFormat p_sample_format) {
	PaSampleFormat pa_sample_format = get_sample_format(p_sample_format);
	PaError err = Pa_GetSampleSize(pa_sample_format);
	return get_error(err);
}

/** Put the caller to sleep for at least 'msec' milliseconds. This function is
 provided only as a convenience for authors of portable code (such as the tests
 and examples in the PortAudio distribution.)

 The function may sleep longer than requested so don't rely on this for accurate
 musical timing.
*/
void PortAudio::sleep(unsigned int p_ms) {
	Pa_Sleep(p_ms);
}

int PortAudio::get_output_buffer_size() {
	return output_buffer_size;
}

void PortAudio::set_output_buffer_size(int p_output_buffer_size) {
	output_buffer_size = p_output_buffer_size;
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
	//ClassDB::bind_method(D_METHOD("open_stream", "stream", "audio_callback", "user_data"), &PortAudio::open_stream);
	//ClassDB::bind_method(D_METHOD("open_default_stream", "stream", "audio_callback", "user_data"), &PortAudio::open_default_stream);
	ClassDB::bind_method(D_METHOD("close_stream", "stream"), &PortAudio::close_stream);
	//ClassDB::bind_method(D_METHOD("set_stream_finished_callback", "stream", "stream_finished_callback"), &PortAudio::set_stream_finished_callback);
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
	// TODO understand port audio better and frames_per_buffer parameter, if it is the same can be combined.
	ClassDB::bind_method(D_METHOD("get_output_buffer_size"), &PortAudio::get_output_buffer_size);
	ClassDB::bind_method(D_METHOD("set_output_buffer_size", "output_buffer_size"), &PortAudio::set_output_buffer_size);

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

	// PortAudioSampleSize
	BIND_ENUM_CONSTANT(FLOAT_32);
	BIND_ENUM_CONSTANT(INT_32);
	BIND_ENUM_CONSTANT(INT_24);
	BIND_ENUM_CONSTANT(INT_16);
	BIND_ENUM_CONSTANT(INT_8);
	BIND_ENUM_CONSTANT(U_INT_8);
	BIND_ENUM_CONSTANT(CUSTOM_FORMAT);
	BIND_ENUM_CONSTANT(NON_INTERLEAVED);
}

PortAudio::PortAudio() {
	singleton = this;
	output_buffer_size = 1024;
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
}
