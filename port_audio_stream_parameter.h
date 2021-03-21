#ifndef PORT_AUDIO_STREAM_PARAMETER_H
#define PORT_AUDIO_STREAM_PARAMETER_H

#include "core/io/resource.h"

class PortAudioStreamParameter : public Resource {
	GDCLASS(PortAudioStreamParameter, Resource);

public:
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
	int device_index;
	int channel_count;
	PortAudioSampleFormat sample_format;
	double suggested_latency;
	void *host_api_specific_stream_info;

protected:
	static void _bind_methods();

public:
	void set_device_index(int p_device_index);
	int get_device_index();
	void set_channel_count(int p_channel_count);
	int get_channel_count();
	void set_sample_format(PortAudioSampleFormat p_sample_format);
	PortAudioSampleFormat get_sample_format();
	void set_suggested_latency(double p_suggested_latency);
	double get_suggested_latency();
	void set_host_api_specific_stream_info(void *p_host_api_specific_stream_info);
	void *get_host_api_specific_stream_info();
	PortAudioStreamParameter();
	~PortAudioStreamParameter();
};

VARIANT_ENUM_CAST(PortAudioStreamParameter::PortAudioSampleFormat);
#endif
