#ifndef PORT_AUDIO_H
#define PORT_AUDIO_H

#include "port_audio_stream.h"

#include "core/object/object.h"
#include "core/io/stream_peer.h"

#include <map>

class PortAudio : public Object {
	GDCLASS(PortAudio, Object);

public:
	enum PortAudioError {
		// Custom Error
		UNDEFINED = -1,
		NOT_PORT_AUDIO_NODE = -2,
		INVALID_FUNC_REF = -3,
		STREAM_NOT_FOUND = -4,
		STREAM_USER_DATA_NOT_FOUND = -5,
		// PortAudio Library Error
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

	enum PortAudioCallbackResult {
		CONTINUE = 0,
		COMPLETE = 1,
		ABORT = 2,
	};

private:
	static PortAudio *singleton;

	std::map<Ref<PortAudioStream>, void *> data_map;

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
	PortAudio::PortAudioError open_stream(Ref<PortAudioStream> p_stream, Callable p_audio_callback, Variant p_user_data);
	PortAudio::PortAudioError open_default_stream(Ref<PortAudioStream> p_stream, PortAudioStreamParameter::PortAudioSampleFormat p_sample_format, Callable p_audio_callback, Variant p_user_data);
	PortAudio::PortAudioError close_stream(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError set_stream_finished_callback(Ref<PortAudioStream> p_stream, Callable p_stream_finished_callback);
	PortAudio::PortAudioError start_stream(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError stop_stream(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError abort_stream(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError is_stream_stopped(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError is_stream_active(Ref<PortAudioStream> p_stream);
	Dictionary get_stream_info(Ref<PortAudioStream> p_stream);
	double get_stream_time(Ref<PortAudioStream> p_stream);
	double get_stream_cpu_load(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError read_stream(Ref<PortAudioStream> p_stream, PackedByteArray p_buffer, uint64_t p_frames);
	PortAudio::PortAudioError write_stream(Ref<PortAudioStream> p_stream, PackedByteArray p_buffer, uint64_t p_frames);
	int64_t get_stream_read_available(Ref<PortAudioStream> p_stream);
	int64_t get_stream_write_available(Ref<PortAudioStream> p_stream);
	PortAudio::PortAudioError get_sample_size(PortAudioStreamParameter::PortAudioSampleFormat p_sample_format);
	void sleep(unsigned int p_ms);

	PortAudio::PortAudioError util_device_index_to_host_api_index(int p_device_index);
	PortAudio::PortAudioError util_enable_exclusive_mode(Ref<PortAudioStreamParameter> p_stream_parameter);
	void util_insert_buffer(Ref<StreamPeerBuffer> p_source, int p_source_offset, Ref<StreamPeerBuffer> p_destination, int p_destination_offset, int p_length);
	void util_write_buffer(Ref<StreamPeerBuffer> p_source, Ref<StreamPeerBuffer> p_destination, int p_length);

	PortAudio();
	~PortAudio();
};

VARIANT_ENUM_CAST(PortAudio::PortAudioError);
VARIANT_ENUM_CAST(PortAudio::PortAudioCallbackResult);

#endif
