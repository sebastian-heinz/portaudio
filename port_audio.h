#ifndef PORT_AUDIO_H
#define PORT_AUDIO_H

#include "port_audio_node.h"
#include "portaudio/include/portaudio.h"

#include <core/object.h>

class PortAudio : public Object {
	GDCLASS(PortAudio, Object);

public:
	enum PortAudioError {
		UNDEFINED = -1,
		NO_ERROR = 0,
		NOT_INITIALIZED = -10000,
		UNANTICIPATED_HOST_ERROR,
		INVALID_CHANNEL_COUNT,
		INVALID_SAMPLE_RATE,
		INVALID_DEVICE,
		INVALID_FLAG,
		SAMPLE_FORMAT_NOT_SUPPORTED,
		BAD_IO_DEVICE_COMBINATION,
		INSUFFICIENT_MEMORY,
		BUFFER_TO_BIG,
		BUFFER_TOO_SMALL,
		NULL_CALLBACK,
		BAD_STREAM_PTR,
		TIMED_OUT,
		INTERNAL_ERROR,
		DEVICE_UNAVAILABLE,
		INCOMPATIBLE_HOST_API_SPECIFIC_STREAM_INFO,
		STREAM_IS_STOPPED,
		STREAM_IS_NOT_STOPPED,
		INPUT_OVERFLOWED,
		OUTPUT_UNDERFLOWED,
		HOST_API_NOT_FOUND,
		INVALID_HOST_API,
		CAN_NOT_READ_FROM_A_CALLBACK_STREAM,
		CAN_NOT_WRITE_TO_A_CALLBACK_STREAM,
		CAN_NOT_READ_FROM_AN_OUT_PUTONLY_STREAM,
		CAN_NOT_WRITE_TO_AN_INPUT_ONLY_STREAM,
		INCOMPATIBLE_STREAM_HOST_API,
		BAD_BUFFER_PTR
	};

private:
	static PortAudio *singleton;
	static int port_audio_callback(const void *p_input_buffer, void *p_output_buffer,
			unsigned long p_frames_per_buffer, const PaStreamCallbackTimeInfo *p_time_info,
			PaStreamCallbackFlags p_status_flags, void *p_user_data);
	static int port_audio_node_callback(const void *p_input_buffer, void *p_output_buffer,
			unsigned long p_frames_per_buffer, const PaStreamCallbackTimeInfo *p_time_info,
			PaStreamCallbackFlags p_status_flags, void *p_user_data);

	PortAudio::PortAudioError get_error(PaError p_error);

protected:
	static void _bind_methods();

public:
	static PortAudio *get_singleton();

	int version();
	String version_text();
	PortAudio::PortAudioError initialize();
	PortAudio::PortAudioError terminate();
	PortAudio::PortAudioError register_node(PortAudioNode *p_port_audio_node);

	PortAudio();
	~PortAudio();
};

VARIANT_ENUM_CAST(PortAudio::PortAudioError);

#endif
