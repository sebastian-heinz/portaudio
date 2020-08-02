#ifndef PORT_AUDIO_H
#define PORT_AUDIO_H

#include "portaudio/include/portaudio.h"

#include <core/object.h>

class PortAudio : public Object {
	GDCLASS(PortAudio, Object);

public:
	typedef int AudioCallback(const PoolVector<float> &p_input_buffer, PoolVector<float> &p_output_buffer,
			unsigned long p_frames_per_buffer, Variant p_time_info,
			unsigned long p_status_flags, void *user_data);

	enum PortAudioError {
		// Custom Error
		UNDEFINED = -1,
		NOT_PORT_AUDIO_NODE = -2,
		// PaError
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
	class PaCallbackUserData {
	public:
		void *port_audio;
		void *audio_callback;
		void *audio_callback_user_data;
		PaCallbackUserData();
	};

	static PortAudio *singleton;

protected:
	static void _bind_methods();

public:
	static PortAudio *get_singleton();
	static PortAudio::PortAudioError get_error(PaError p_error);
	static int port_audio_callback_converter(const void *p_input_buffer, void *p_output_buffer,
			unsigned long p_frames_per_buffer, const PaStreamCallbackTimeInfo *p_time_info,
			PaStreamCallbackFlags p_status_flags, void *p_user_data);

	int version();
	String version_text();
	PortAudio::PortAudioError initialize();
	PortAudio::PortAudioError terminate();
	PortAudio::PortAudioError open_default_stream(void **p_stream, int p_input_channel_count,
			int p_output_channel_count, double p_sample_rate, unsigned long p_frames_per_buffer,
			AudioCallback *p_audio_callback, void *p_user_data);
	PortAudio::PortAudioError start_stream(void *p_stream);
	PortAudio::PortAudioError stop_stream(void *p_stream);
	PortAudio::PortAudioError close_stream(void *p_stream);
	PortAudio::PortAudioError is_stream_stopped(void *p_stream);
	PortAudio::PortAudioError is_stream_active(void *p_stream);
	double get_stream_time(void *p_stream);
	Dictionary get_stream_info(void *p_stream);
	double get_stream_cpu_load(void *p_stream);
	void sleep(unsigned int p_ms);

	PortAudio();
	~PortAudio();
};

VARIANT_ENUM_CAST(PortAudio::PortAudioError);

#endif
