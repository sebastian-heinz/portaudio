#include "port_audio_node.h"

int PortAudioNode::node_audio_callback(const PoolVector<uint8_t> &p_input_buffer, PoolVector<uint8_t> &p_output_buffer,
		unsigned long p_frames_per_buffer, Dictionary p_time_info,
		unsigned long p_status_flags, void *p_user_data) {
	PortAudioNode *port_audio_node = (PortAudioNode *)p_user_data;
	if (!port_audio_node) {
		print_line("PortAudioNode::node_audio_callback: !port_audio_node");
		return 0;
	}
	return port_audio_node->audio_callback(p_input_buffer, p_output_buffer, p_frames_per_buffer, p_time_info, p_status_flags);
}

PortAudio::PortAudioError PortAudioNode::open_default_stream() {
	PortAudio::PortAudioError err = PortAudio::get_singleton()->open_default_stream(stream, &PortAudioNode::node_audio_callback, this);
	return err;
}

PortAudio::PortAudioError PortAudioNode::open_stream() {
	PortAudio::PortAudioError err = PortAudio::get_singleton()->open_stream(stream, &PortAudioNode::node_audio_callback, this);
	return err;
}

PortAudio::PortAudioError PortAudioNode::start_stream() {
	return PortAudio::get_singleton()->start_stream(stream);
}

PortAudio::PortAudioError PortAudioNode::stop_stream() {
	return PortAudio::get_singleton()->stop_stream(stream);
}

PortAudio::PortAudioError PortAudioNode::close_stream() {
	return PortAudio::get_singleton()->close_stream(stream);
}

PortAudio::PortAudioError PortAudioNode::is_stream_stopped() {
	return PortAudio::get_singleton()->is_stream_stopped(stream);
}

PortAudio::PortAudioError PortAudioNode::is_stream_active() {
	return PortAudio::get_singleton()->is_stream_active(stream);
}

double PortAudioNode::get_stream_time() {
	return PortAudio::get_singleton()->get_stream_time(stream);
}

Dictionary PortAudioNode::get_stream_info() {
	return PortAudio::get_singleton()->get_stream_info(stream);
}

void PortAudioNode::_bind_methods() {

	ClassDB::bind_method(D_METHOD("open_default_stream"), &PortAudioNode::open_default_stream);
	ClassDB::bind_method(D_METHOD("open_stream"), &PortAudioNode::open_stream);
	ClassDB::bind_method(D_METHOD("start_stream"), &PortAudioNode::start_stream);
	ClassDB::bind_method(D_METHOD("stop_stream"), &PortAudioNode::stop_stream);
	ClassDB::bind_method(D_METHOD("close_stream"), &PortAudioNode::close_stream);
	ClassDB::bind_method(D_METHOD("is_stream_active"), &PortAudioNode::is_stream_active);
	ClassDB::bind_method(D_METHOD("is_stream_stopped"), &PortAudioNode::is_stream_stopped);
	ClassDB::bind_method(D_METHOD("get_stream_time"), &PortAudioNode::get_stream_time);
	ClassDB::bind_method(D_METHOD("get_stream_info"), &PortAudioNode::get_stream_info);
	ClassDB::bind_method(D_METHOD("get_stream"), &PortAudioNode::get_stream);
	ClassDB::bind_method(D_METHOD("set_stream", "stream"), &PortAudioNode::set_stream);
	ClassDB::bind_method(D_METHOD("audio_callback"), &PortAudioNode::audio_callback);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "stream", PROPERTY_HINT_RESOURCE_TYPE, "PortAudioStream"), "set_stream", "get_stream");
}

int PortAudioNode::audio_callback(const PoolByteArray p_input_buffer, PoolByteArray p_output_buffer, int p_frames_per_buffer, Dictionary p_time_info, int p_status_flags) {
	return 0;
}

Ref<PortAudioStream> PortAudioNode::get_stream() {
	return stream;
}

void PortAudioNode::set_stream(Ref<PortAudioStream> p_stream) {
	stream = p_stream;
}

PortAudioNode::PortAudioNode() {
	stream = Ref<PortAudioStream>();
}

PortAudioNode::~PortAudioNode() {
}
