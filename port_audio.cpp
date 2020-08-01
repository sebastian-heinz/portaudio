#include "port_audio.h"

PortAudio *PortAudio::singleton = NULL;

PortAudio *PortAudio::get_singleton() {
	return singleton;
}

int PortAudio::port_audio_callback(const void *p_input_buffer, void *p_output_buffer,
		unsigned long p_frames_per_buffer, const PaStreamCallbackTimeInfo *p_time_info,
		PaStreamCallbackFlags p_status_flags, void *p_user_data) {
	PortAudio *port_audio = (PortAudio *)p_user_data;
	if (!port_audio) {
		print_line("PortAudio::port_audio_callback: !port_audio");
		return 0;
	}
	return 0;
}

int PortAudio::port_audio_node_callback(const void *p_input_buffer, void *p_output_buffer,
		unsigned long p_frames_per_buffer, const PaStreamCallbackTimeInfo *p_time_info,
		PaStreamCallbackFlags p_status_flags, void *p_user_data) {
	PortAudioNode *node = (PortAudioNode *)p_user_data;
	if (!node) {
		print_line("PortAudio::port_audio_node_callback: !node");
		return 0;
	}
	// copy input buffer to godot type (might need to find a faster way)
	PoolVector<float> pool_vector_in;
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
	PoolVector<float> pool_vector_out;
	// provide time info
	Dictionary time_info;
	time_info["input_buffer_adc_time"] = p_time_info->inputBufferAdcTime;
	time_info["current_time"] = p_time_info->currentTime;
	time_info["output_buffer_dac_time"] = p_time_info->outputBufferDacTime;
	// perform callback
	node->audio_callback(pool_vector_in, pool_vector_out, p_frames_per_buffer, time_info, p_status_flags);
	// copy result into buffer (might need to find a faster way)
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

PortAudio::PortAudioError PortAudio::register_node(PortAudioNode *p_port_audio_node) {
	PaStream *stream;
	PaError err = Pa_OpenDefaultStream(&stream,
			p_port_audio_node->get_input_channel_count(),
			p_port_audio_node->get_output_channel_count(),
			paFloat32,
			p_port_audio_node->get_sample_rate(),
			p_port_audio_node->get_frames_per_buffer(),
			&PortAudio::port_audio_node_callback,
			p_port_audio_node);
	return get_error(err);
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
	return PortAudio::PortAudioError::UNDEFINED;
}

void PortAudio::_bind_methods() {
	ClassDB::bind_method(D_METHOD("version"), &PortAudio::version);
	ClassDB::bind_method(D_METHOD("version_text"), &PortAudio::version_text);
	ClassDB::bind_method(D_METHOD("initialize"), &PortAudio::initialize);
	ClassDB::bind_method(D_METHOD("terminate"), &PortAudio::terminate);
	ClassDB::bind_method(D_METHOD("register_node", "port_audio_node"), &PortAudio::register_node);

	BIND_ENUM_CONSTANT(UNDEFINED);
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
}

PortAudio::~PortAudio() {
}
