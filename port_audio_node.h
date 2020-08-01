#ifndef PORT_AUDIO_NODE_H
#define PORT_AUDIO_NODE_H

#include <scene/main/node.h>

class PortAudioNode : public Node {
	GDCLASS(PortAudioNode, Node);

private:
	double sample_rate;
	int input_channel_count;
	int output_channel_count;
	unsigned long frames_per_buffer;

protected:
	static void _bind_methods();

public:
	void audio_callback(const PoolVector<float> &p_input_buffer, PoolVector<float> p_output_buffer,
			unsigned long p_frames_per_buffer, Variant p_time_info, unsigned long p_status_flags);
	double get_sample_rate();
	void set_sample_rate(double p_sample_rate);
	int get_input_channel_count();
	void set_input_channel_count(int p_input_channel_count);
	int get_output_channel_count();
	void set_output_channel_count(int p_output_channel_count);
	unsigned long get_frames_per_buffer();
	void set_frames_per_buffer(unsigned long p_frames_per_buffer);
	PortAudioNode();
	~PortAudioNode();
};

#endif
