#ifndef PORT_AUDIO_NODE_H
#define PORT_AUDIO_NODE_H

#include "port_audio.h"
#include "port_audio_stream.h"

#include <scene/main/node.h>
#include <servers/audio/audio_stream.h>

class PortAudioNode : public Node {
	GDCLASS(PortAudioNode, Node);

private:
	static int node_audio_callback(const PoolVector<uint8_t> &p_input_buffer, PoolVector<uint8_t> &p_output_buffer,
			unsigned long p_frames_per_buffer, Dictionary p_time_info,
			unsigned long p_status_flags, void *p_user_data);

	Ref<PortAudioStream> stream;

protected:
	static void _bind_methods();

	// TODO Godot does not support binding unsigned long
	int audio_callback(const PoolByteArray p_input_buffer, PoolByteArray p_output_buffer,
			int p_frames_per_buffer, Dictionary p_time_info, int p_status_flags);

public:
	PortAudio::PortAudioError open_default_stream();
	PortAudio::PortAudioError open_stream();
	PortAudio::PortAudioError start_stream();
	PortAudio::PortAudioError stop_stream();
	PortAudio::PortAudioError close_stream();
	PortAudio::PortAudioError is_stream_active();
	PortAudio::PortAudioError is_stream_stopped();
	double get_stream_time();
	Dictionary get_stream_info();
	Ref<PortAudioStream> get_stream();
	void set_stream(Ref<PortAudioStream> p_stream);

	PortAudioNode();
	~PortAudioNode();
};

#endif
