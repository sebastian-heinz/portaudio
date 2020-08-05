#ifndef PORT_AUDIO_H
#define PORT_AUDIO_H

#include "port_audio_stream.h"

#include <core/object.h>

class PortAudio : public Object {
	GDCLASS(PortAudio, Object);

public:
	typedef int AudioCallback(
			const PoolVector<uint8_t> &p_input_buffer,
			PoolVector<uint8_t> &p_output_buffer,
			unsigned long p_frames_per_buffer,
			Dictionary p_time_info,
			unsigned long p_status_flags,
			void *user_data);

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
	static PortAudio *singleton;

	int output_buffer_size;


protected:
	static void _bind_methods();

public:
	static PortAudio *get_singleton();

	int get_version();
	String get_version_text();
	// Pa_GetVersionInfo
	String get_error_text(PortAudio::PortAudioError p_error);
	PortAudio::PortAudioError initialize();
	PortAudio::PortAudioError terminate();
	int get_host_api_count();
	int get_default_host_api();
	Dictionary get_host_api_info(int p_host_api);
	int host_api_type_id_to_host_api_index(int p_host_api_type_id);
	int host_api_device_index_to_device_index(int p_host_api, int p_host_api_device_index);
	// Pa_GetLastHostErrorInfo
	int get_device_count();
	int get_default_input_device();
	int get_default_output_device();
	Dictionary get_device_info(int p_device_index);
	PortAudio::PortAudioError is_format_supported(Ref<PortAudioStreamParameter> p_input_stream_parameter, Ref<PortAudioStreamParameter> p_output_stream_parameter, double p_sample_rate);
	PortAudio::PortAudioError open_stream(Ref<PortAudioStream> p_stream, AudioCallback *p_audio_callback, void *p_user_data);
	PortAudio::PortAudioError open_default_stream(Ref<PortAudioStream> p_stream, AudioCallback *p_audio_callback, void *p_user_data);
	PortAudio::PortAudioError close_stream(Ref<PortAudioStream> p_stream);
	//Pa_SetStreamFinishedCallback
	PortAudio::PortAudioError start_stream(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError stop_stream(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError abort_stream(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError is_stream_stopped(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError is_stream_active(Ref<PortAudioStream> p_stream);
	Dictionary get_stream_info(Ref<PortAudioStream> p_stream);
	double get_stream_time(Ref<PortAudioStream> p_stream);
	double get_stream_cpu_load(Ref<PortAudioStream> p_stream);
	// Pa_ReadStream
	// Pa_WriteStream
	// Pa_GetStreamReadAvailable
	// Pa_GetStreamWriteAvailable
	// Pa_GetSampleSize
	void sleep(unsigned int p_ms);

	int get_output_buffer_size();
	void set_output_buffer_size(int p_output_buffer_size);

	PortAudio();
	~PortAudio();
};

VARIANT_ENUM_CAST(PortAudio::PortAudioError);

#endif
