#include "port_audio.h"

#include "port_audio_node.h"

PortAudio *PortAudio::singleton = NULL;

PortAudio *PortAudio::get_singleton() {
	return singleton;
}

int PortAudio::port_audio_callback_converter(const void *p_input_buffer, void *p_output_buffer,
		unsigned long p_frames_per_buffer, const PaStreamCallbackTimeInfo *p_time_info,
		PaStreamCallbackFlags p_status_flags, void *p_user_data) {

	PaCallbackUserData *user_data = (PaCallbackUserData *)p_user_data;
	if (!user_data) {
		print_line("PortAudio::port_audio_callback_converter: !user_data");
		return 0;
	}
	// copy input buffer to godot type (TODO might need to find a faster way)
	PoolVector<float> pool_vector_in;
	if (p_input_buffer) {
		Error err = pool_vector_in.resize(p_frames_per_buffer);
		if (err != OK) {
			print_line(vformat("PortAudio::port_audio_node_callback: can not resize pool_vector_in to: %d", (unsigned int)p_frames_per_buffer));
			return 0;
		}
		float *in = (float *)p_input_buffer;
		PoolVector<float>::Write write_in = pool_vector_in.write();
		for (unsigned int i = 0; i < p_frames_per_buffer; i++) {
			write_in[i] = in[i];
		}
		write_in.release();
	}
	// provide time info
	Dictionary time_info;
	time_info["input_buffer_adc_time"] = p_time_info->inputBufferAdcTime;
	time_info["current_time"] = p_time_info->currentTime;
	time_info["output_buffer_dac_time"] = p_time_info->outputBufferDacTime;
	// perform callback
	PoolVector<float> pool_vector_out;
	AudioCallback *audio_callback = (AudioCallback *)user_data->audio_callback;
	audio_callback(pool_vector_in, pool_vector_out, p_frames_per_buffer,
			time_info, p_status_flags, user_data->audio_callback_user_data);
	// copy result into buffer (TODO might need to find a faster way)
	float *out = (float *)p_output_buffer;
	unsigned int size_out = pool_vector_out.size();
	PoolVector<float>::Read read_out = pool_vector_out.read();
	for (unsigned int i = 0; i < size_out; i++) {
		out[i] = read_out[i];
	}
	read_out.release();
	return 0;
}

/** Retrieve the release number of the currently running PortAudio build.
 For example, for version "19.5.1" this will return 0x00130501.

 @see paMakeVersionNumber
*/
int PortAudio::version() {
	return Pa_GetVersion();
}

/** Retrieve a textual description of the current PortAudio build,
 e.g. "PortAudio V19.5.0-devel, revision 1952M".
 The format of the text may change in the future. Do not try to parse the
 returned string.

 @deprecated As of 19.5.0, use Pa_GetVersionInfo()->versionText instead.
*/
String PortAudio::version_text() {
	return String(Pa_GetVersionText());
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

PortAudio::PortAudioError PortAudio::open_default_stream(void **p_stream, int p_input_channel_count,
		int p_output_channel_count, double p_sample_rate, unsigned long p_frames_per_buffer,
		AudioCallback *p_audio_callback, void *p_user_data) {
	// TODO delete(p_audio_callback) - maybe add to list & add finished flag and clean up periodically
	PaCallbackUserData *user_data = new PaCallbackUserData();
	user_data->port_audio = this;
	user_data->audio_callback = p_audio_callback;
	user_data->audio_callback_user_data = p_user_data;
	PaError err = Pa_OpenDefaultStream(p_stream,
			p_input_channel_count,
			p_output_channel_count,
			paFloat32,
			p_sample_rate,
			p_frames_per_buffer,
			&PortAudio::port_audio_callback_converter,
			user_data);
	return get_error(err);
}

/**
Commences audio processing.
*/
PortAudio::PortAudioError PortAudio::start_stream(void *p_stream) {
	PaError err = Pa_StartStream(p_stream);
	return get_error(err);
}

/**
Terminates audio processing. It waits until all pending
 audio buffers have been played before it returns.
*/
PortAudio::PortAudioError PortAudio::stop_stream(void *p_stream) {
	PaError err = Pa_StopStream(p_stream);
	return get_error(err);
}

/** Closes an audio stream. If the audio stream is active it
 discards any pending buffers as if Pa_AbortStream() had been called.
*/
PortAudio::PortAudioError PortAudio::close_stream(void *p_stream) {
	PaError err = Pa_CloseStream(p_stream);
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
PortAudio::PortAudioError PortAudio::is_stream_stopped(void *p_stream) {
	PaError err = Pa_IsStreamStopped(p_stream);
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
PortAudio::PortAudioError PortAudio::is_stream_active(void *p_stream) {
	PaError err = Pa_IsStreamActive(p_stream);
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
double PortAudio::get_stream_time(void *p_stream) {
	PaTime time = Pa_GetStreamTime(p_stream);
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
Dictionary PortAudio::get_stream_info(void *p_stream) {
	const PaStreamInfo *pa_stream_info = Pa_GetStreamInfo(p_stream);

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

double PortAudio::get_stream_cpu_load(void *p_stream) {
	return Pa_GetStreamCpuLoad(p_stream);
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

PortAudio::PortAudioError PortAudio::get_error(PaError p_error) {
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
	print_error(vformat("PortAudio::get_error: UNDEFINED error code: %d", p_error));
	return PortAudio::PortAudioError::UNDEFINED;
}

void PortAudio::_bind_methods() {
	ClassDB::bind_method(D_METHOD("version"), &PortAudio::version);
	ClassDB::bind_method(D_METHOD("version_text"), &PortAudio::version_text);
	ClassDB::bind_method(D_METHOD("initialize"), &PortAudio::initialize);
	ClassDB::bind_method(D_METHOD("terminate"), &PortAudio::terminate);
	ClassDB::bind_method(D_METHOD("sleep", "ms"), &PortAudio::sleep);

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
}

PortAudio::PaCallbackUserData::PaCallbackUserData() {
	port_audio = NULL;
	audio_callback = NULL;
	audio_callback_user_data = NULL;
}
