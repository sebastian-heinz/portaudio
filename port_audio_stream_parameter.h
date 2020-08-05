#ifndef PORT_AUDIO_STREAM_PARAMETER_H
#define PORT_AUDIO_STREAM_PARAMETER_H

#include <core/resource.h>

class PortAudioStreamParameter : public Resource {
	GDCLASS(PortAudioStreamParameter, Resource);

private:
	int device_index;
	int channel_count;
	uint64_t sample_format;
	double suggested_latency;
	void *host_api_specific_stream_info;

protected:
	static void _bind_methods();

public:
	void set_device_index(int p_device_index);
	int get_device_index();
	void set_channel_count(int p_channel_count);
	int get_channel_count();
	void set_sample_format(uint64_t p_sample_format);
	uint64_t get_sample_format();
	void set_suggested_latency(double p_suggested_latency);
	double get_suggested_latency();
	void set_host_api_specific_stream_info(void *p_host_api_specific_stream_info);
	void *get_host_api_specific_stream_info();
	PortAudioStreamParameter();
	~PortAudioStreamParameter();
};

#endif
