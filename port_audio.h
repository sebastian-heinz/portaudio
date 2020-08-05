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

	/** Functions of type PaStreamFinishedCallback are implemented by PortAudio 
 clients. They can be registered with a stream using the Pa_SetStreamFinishedCallback
 function. Once registered they are called when the stream becomes inactive
 (ie once a call to Pa_StopStream() will not block).
 A stream will become inactive after the stream callback returns non-zero,
 or when Pa_StopStream or Pa_AbortStream is called. For a stream providing audio
 output, if the stream callback returns paComplete, or Pa_StopStream() is called,
 the stream finished callback will not be called until all generated sample data
 has been played.
 
 @param userData The userData parameter supplied to Pa_OpenStream()

 @see Pa_SetStreamFinishedCallback
*/
	typedef void StreamFinishedCallback(void *userData);

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

	enum PortAudioSampleFormat {
		FLOAT_32 = 0x00000001,
		INT_32 = 0x00000002,
		INT_24 = 0x00000004,
		INT_16 = 0x00000008,
		INT_8 = 0x00000010,
		U_INT_8 = 0x00000020,
		CUSTOM_FORMAT = 0x00010000,
		NON_INTERLEAVED = 0x80000000,
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
	Dictionary get_version_info();
	String get_error_text(PortAudio::PortAudioError p_error);
	PortAudio::PortAudioError initialize();
	PortAudio::PortAudioError terminate();
	int get_host_api_count();
	int get_default_host_api();
	Dictionary get_host_api_info(int p_host_api);
	int host_api_type_id_to_host_api_index(int p_host_api_type_id);
	int host_api_device_index_to_device_index(int p_host_api, int p_host_api_device_index);
	Dictionary get_last_host_error_info();
	int get_device_count();
	int get_default_input_device();
	int get_default_output_device();
	Dictionary get_device_info(int p_device_index);
	PortAudio::PortAudioError is_format_supported(Ref<PortAudioStreamParameter> p_input_stream_parameter, Ref<PortAudioStreamParameter> p_output_stream_parameter, double p_sample_rate);
	PortAudio::PortAudioError open_stream(Ref<PortAudioStream> p_stream, AudioCallback *p_audio_callback, void *p_user_data);
	PortAudio::PortAudioError open_default_stream(Ref<PortAudioStream> p_stream, AudioCallback *p_audio_callback, void *p_user_data);
	PortAudio::PortAudioError close_stream(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError set_stream_finished_callback(Ref<PortAudioStream> p_stream, StreamFinishedCallback *p_stream_finished_callback);
	PortAudio::PortAudioError start_stream(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError stop_stream(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError abort_stream(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError is_stream_stopped(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError is_stream_active(Ref<PortAudioStream> p_stream);
	Dictionary get_stream_info(Ref<PortAudioStream> p_stream);
	double get_stream_time(Ref<PortAudioStream> p_stream);
	double get_stream_cpu_load(Ref<PortAudioStream> p_stream);

	// TODO gotot types
	PortAudio::PortAudioError read_stream(Ref<PortAudioStream> p_stream, void *p_buffer, unsigned long p_frames);
	PortAudio::PortAudioError write_stream(Ref<PortAudioStream> p_stream, void *p_buffer, unsigned long p_frames);
	signed long get_stream_read_available(Ref<PortAudioStream> p_stream);
	signed long get_stream_write_available(Ref<PortAudioStream> p_stream);

	PortAudio::PortAudioError get_sample_size(PortAudioSampleFormat p_sample_format);
	void sleep(unsigned int p_ms);

	int get_output_buffer_size();
	void set_output_buffer_size(int p_output_buffer_size);

	PortAudio();
	~PortAudio();
};

VARIANT_ENUM_CAST(PortAudio::PortAudioError);
VARIANT_ENUM_CAST(PortAudio::PortAudioSampleFormat);

#endif
