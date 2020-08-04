#include "port_audio_node.h"

int PortAudioNode::node_audio_callback(const PoolVector<float> &p_input_buffer, PoolVector<float> &p_output_buffer,
		unsigned long p_frames_per_buffer, Variant p_time_info,
		unsigned long p_status_flags, void *p_user_data) {
	PortAudioNode *port_audio_node = (PortAudioNode *)p_user_data;
	if (!port_audio_node) {
		print_line("PortAudioNode::audio_callback: !port_audio_node");
		return 0;
	}
	return port_audio_node->audio_callback(p_input_buffer, p_output_buffer, p_frames_per_buffer, p_time_info, p_status_flags);
}

void PortAudioNode::set_data(const PoolVector<uint8_t> &p_data) {
	data_position = 0;
	data = p_data;
}

PoolVector<uint8_t> PortAudioNode::get_data() {
	return data;
}

PortAudio::PortAudioError PortAudioNode::open_default_stream() {
	void *pa_stream;
	PortAudio::PortAudioError err = PortAudio::get_singleton()->open_default_stream(&pa_stream,
			input_channel_count,
			output_channel_count,
			sample_rate,
			frames_per_buffer,
			&PortAudioNode::node_audio_callback,
			this);
	if (err == PortAudio::PortAudioError::NO_ERROR) {
		stream = pa_stream;
	}
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

	ClassDB::bind_method(D_METHOD("get_sample_rate"), &PortAudioNode::get_sample_rate);
	ClassDB::bind_method(D_METHOD("set_sample_rate", "sample_rate"), &PortAudioNode::set_sample_rate);
	ClassDB::bind_method(D_METHOD("get_input_channel_count"), &PortAudioNode::get_input_channel_count);
	ClassDB::bind_method(D_METHOD("set_input_channel_count", "input_channel_count"), &PortAudioNode::set_input_channel_count);
	ClassDB::bind_method(D_METHOD("get_output_channel_count"), &PortAudioNode::get_output_channel_count);
	ClassDB::bind_method(D_METHOD("set_output_channel_count", "output_channel_count"), &PortAudioNode::set_output_channel_count);
	ClassDB::bind_method(D_METHOD("get_frames_per_buffer"), &PortAudioNode::get_frames_per_buffer);
	ClassDB::bind_method(D_METHOD("set_frames_per_buffer", "frames_per_buffer"), &PortAudioNode::set_frames_per_buffer);
	ClassDB::bind_method(D_METHOD("get_data"), &PortAudioNode::get_data);
	ClassDB::bind_method(D_METHOD("set_data", "data"), &PortAudioNode::set_data);
	ClassDB::bind_method(D_METHOD("is_stream_active"), &PortAudioNode::is_stream_active);
	ClassDB::bind_method(D_METHOD("is_stream_stopped"), &PortAudioNode::is_stream_stopped);
	ClassDB::bind_method(D_METHOD("close_stream"), &PortAudioNode::close_stream);
	ClassDB::bind_method(D_METHOD("stop_stream"), &PortAudioNode::stop_stream);
	ClassDB::bind_method(D_METHOD("start_stream"), &PortAudioNode::start_stream);
	ClassDB::bind_method(D_METHOD("open_default_stream"), &PortAudioNode::open_default_stream);
	ClassDB::bind_method(D_METHOD("get_stream_time"), &PortAudioNode::get_stream_time);
	ClassDB::bind_method(D_METHOD("get_stream_info"), &PortAudioNode::get_stream_info);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_rate"), "set_sample_rate", "get_sample_rate");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "input_channel_count"), "set_input_channel_count", "get_input_channel_count");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "output_channel_count"), "set_output_channel_count", "get_output_channel_count");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frames_per_buffer"), "set_frames_per_buffer", "get_frames_per_buffer");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_BYTE_ARRAY, "data"), "set_data", "get_data");

	ADD_SIGNAL(MethodInfo("audio_callback",
			PropertyInfo(Variant::POOL_REAL_ARRAY, "input_buffer"),
			PropertyInfo(Variant::POOL_REAL_ARRAY, "output_buffer"),
			PropertyInfo(Variant::INT, "frames_per_buffer"),
			PropertyInfo(Variant::DICTIONARY, "time_info"),
			PropertyInfo(Variant::INT, "status_flags")));
}

int PortAudioNode::audio_callback(const PoolVector<float> &p_input_buffer, PoolVector<float> &p_output_buffer, unsigned long p_frames_per_buffer, Variant p_time_info, unsigned long p_status_flags) {

	PoolVector<uint8_t>::Read r = data.read();

	Error err = p_output_buffer.resize(p_frames_per_buffer);
	if (err != OK) {
		print_line(vformat("PortAudio::port_audio_node_callback: can not resize pool_vector_out to: %d", (unsigned int)p_frames_per_buffer));
		return 0;
	}

	PoolVector<float>::Write w = p_output_buffer.write();
	for (int i = 0; i < p_frames_per_buffer; i++) {
		w[i] = r[data_position];
		data_position++;
	}

	//	emit_signal("audio_callback", p_input_buffer, p_output_buffer,
	//			(unsigned int)p_frames_per_buffer, p_time_info, (unsigned int)p_status_flags);
	return 0;
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

unsigned int PortAudioNode::get_frames_per_buffer() {
	return frames_per_buffer;
}

void PortAudioNode::set_frames_per_buffer(unsigned int p_frames_per_buffer) {
	frames_per_buffer = p_frames_per_buffer;
}

void *PortAudioNode::get_stream() {
	return stream;
}

void PortAudioNode::set_stream(void *p_stream) {
	stream = p_stream;
}

PortAudioNode::PortAudioNode() {
	input_channel_count = 0;
	output_channel_count = 2;
	sample_rate = 44100;
	frames_per_buffer = 0; //paFramesPerBufferUnspecified;
	stream = NULL;
	data_position = 0;
	data = PoolVector<uint8_t>();
}

PortAudioNode::~PortAudioNode() {
}
