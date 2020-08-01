#include "port_audio_node.h"

void PortAudioNode::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_sample_rate"), &PortAudioNode::get_sample_rate);
	ClassDB::bind_method(D_METHOD("set_sample_rate", "sample_rate"), &PortAudioNode::set_sample_rate);
	ClassDB::bind_method(D_METHOD("get_input_channel_count"), &PortAudioNode::get_input_channel_count);
	ClassDB::bind_method(D_METHOD("set_input_channel_count", "input_channel_count"), &PortAudioNode::set_input_channel_count);
	ClassDB::bind_method(D_METHOD("get_output_channel_count"), &PortAudioNode::get_output_channel_count);
	ClassDB::bind_method(D_METHOD("set_output_channel_count", "output_channel_count"), &PortAudioNode::set_output_channel_count);
	ClassDB::bind_method(D_METHOD("get_frames_per_buffer"), &PortAudioNode::get_frames_per_buffer);
	ClassDB::bind_method(D_METHOD("set_frames_per_buffer", "frames_per_buffer"), &PortAudioNode::set_frames_per_buffer);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_rate"), "set_sample_rate", "get_sample_rate");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "input_channel_count"), "set_input_channel_count", "get_input_channel_count");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "output_channel_count"), "set_output_channel_count", "get_output_channel_count");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frames_per_buffer"), "set_frames_per_buffer", "get_frames_per_buffer");

	ADD_SIGNAL(MethodInfo("audio_callback",
					   PropertyInfo(Variant::POOL_BYTE_ARRAY, "input_buffer"),
					   PropertyInfo(Variant::POOL_BYTE_ARRAY, "output_buffer"),
					   PropertyInfo(Variant::DICTIONARY, "frames_per_buffer"),
					   PropertyInfo(Variant::OBJECT, "time_info")),
			PropertyInfo(Variant::OBJECT, "status_flags"));
}

void PortAudioNode::audio_callback(const PoolVector<float> &p_input_buffer, PoolVector<float> p_output_buffer,
		unsigned long p_frames_per_buffer, Variant p_time_info, unsigned long p_status_flags) {
	emit_signal("audio_callback", p_input_buffer, p_output_buffer,
			(unsigned int)p_frames_per_buffer, p_time_info, (unsigned int)p_status_flags);
}

double PortAudioNode::get_sample_rate() {
	return sample_rate;
}

void PortAudioNode::set_sample_rate(double p_sample_rate) {
	sample_rate = p_sample_rate;
}

int PortAudioNode::get_input_channel_count() {
	return input_channel_count;
}

void PortAudioNode::set_input_channel_count(int p_input_channel_count) {
	input_channel_count = p_input_channel_count;
}

int PortAudioNode::get_output_channel_count() {
	return output_channel_count;
}

void PortAudioNode::set_output_channel_count(int p_output_channel_count) {
	output_channel_count = p_output_channel_count;
}

unsigned long PortAudioNode::get_frames_per_buffer() {
	return frames_per_buffer;
}

void PortAudioNode::set_frames_per_buffer(unsigned long p_frames_per_buffer) {
	frames_per_buffer = p_frames_per_buffer;
}

PortAudioNode::PortAudioNode() {
	input_channel_count = 0;
	output_channel_count = 2;
	sample_rate = 44100;
	frames_per_buffer = 0; //paFramesPerBufferUnspecified;
}

PortAudioNode::~PortAudioNode() {
}
