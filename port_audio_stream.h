#ifndef PORT_AUDIO_STREAM_H
#define PORT_AUDIO_STREAM_H

#include "port_audio_stream_parameter.h"

#include "core/io/resource.h"

class PortAudioStream : public Resource {
	GDCLASS(PortAudioStream, Resource);

public:
	enum PortAudioStreamFlag {
		NO_FLAG = 0,
		CLIP_OFF = 0x00000001,
		DITHER_OFF = 0x00000002,
		NEVER_DROP_INPUT = 0x00000004,
		PRIME_OOUTPUT_BUFFERS_USING_STREAM_CALLBACK = 0x00000008,
		PLATFORM_SPECIFIC_FLAGS = 0xFFFF0000
	};

private:
	void *stream;
	double sample_rate;
	unsigned int frames_per_buffer;
	Ref<PortAudioStreamParameter> input_stream_parameter;
	Ref<PortAudioStreamParameter> output_stream_parameter;
	PortAudioStreamFlag stream_flags;

protected:
	static void _bind_methods();

public:
	int get_input_channel_count();
	void set_input_channel_count(int p_input_channel_count);
	int get_output_channel_count();
	void set_output_channel_count(int p_output_channel_count);
	double get_sample_rate();
	void set_sample_rate(double p_sample_rate);
	unsigned int get_frames_per_buffer();
	void set_frames_per_buffer(unsigned int p_frames_per_buffer);
	Ref<PortAudioStreamParameter> get_input_stream_parameter();
	void set_input_stream_parameter(Ref<PortAudioStreamParameter> p_input_stream_parameter);
	Ref<PortAudioStreamParameter> get_output_stream_parameter();
	void set_output_stream_parameter(Ref<PortAudioStreamParameter> p_output_stream_parameter);
	PortAudioStreamFlag get_stream_flags();
	void set_stream_flags(PortAudioStreamFlag p_stream_flags);
	void *get_stream();
	void set_stream(void *p_stream);

	PortAudioStream();
	~PortAudioStream();
};

VARIANT_ENUM_CAST(PortAudioStream::PortAudioStreamFlag);

#endif
