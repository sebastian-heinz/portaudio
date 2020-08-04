#ifndef PORT_AUDIO_NODE_H
#define PORT_AUDIO_NODE_H

#include "port_audio.h"

#include <scene/main/node.h>
#include <servers/audio/audio_stream.h>

class PortAudioNode : public Node {
	GDCLASS(PortAudioNode, Node);

private:
	static int node_audio_callback(const PoolVector<float> &p_input_buffer, PoolVector<float> &p_output_buffer,
			unsigned long p_frames_per_buffer, Variant p_time_info,
			unsigned long p_status_flags, void *p_user_data);

	unsigned int data_position;
	void *stream;
	PoolVector<uint8_t> data;
	double sample_rate;
	int input_channel_count;
	int output_channel_count;
	unsigned int frames_per_buffer;

protected:
	static void _bind_methods();
	int audio_callback(const PoolVector<float> &p_input_buffer, PoolVector<float> &p_output_buffer,
			unsigned long p_frames_per_buffer, Variant p_time_info, unsigned long p_status_flags);

public:
	PortAudio::PortAudioError open_default_stream();
	PortAudio::PortAudioError start_stream();
	PortAudio::PortAudioError stop_stream();
	PortAudio::PortAudioError close_stream();
	PortAudio::PortAudioError is_stream_stopped();
	PortAudio::PortAudioError is_stream_active();
	double get_stream_time();
	Dictionary get_stream_info();

	void set_data(const PoolVector<uint8_t> &p_data);
	PoolVector<uint8_t> get_data();
	double get_sample_rate();
	void set_sample_rate(double p_sample_rate);
	int get_input_channel_count();
	void set_input_channel_count(int p_input_channel_count);
	int get_output_channel_count();
	void set_output_channel_count(int p_output_channel_count);
	unsigned int get_frames_per_buffer();
	void set_frames_per_buffer(unsigned int p_frames_per_buffer);
	void *get_stream();
	void set_stream(void *p_stream);
	PortAudioNode();
	~PortAudioNode();
};

#endif
