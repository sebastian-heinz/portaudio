#include "port_audio_callback_data.h"

void PortAudioCallbackData::set_input_buffer_adc_time(double p_input_buffer_adc_time) {
	input_buffer_adc_time = p_input_buffer_adc_time;
}

double PortAudioCallbackData::get_input_buffer_adc_time() {
	return input_buffer_adc_time;
}

void PortAudioCallbackData::set_current_time(double p_current_time) {
	current_time = p_current_time;
}

double PortAudioCallbackData::get_current_time() {
	return current_time;
}

void PortAudioCallbackData::set_output_buffer_dac_time(double p_output_buffer_dac_time) {
	output_buffer_dac_time = p_output_buffer_dac_time;
}

double PortAudioCallbackData::get_output_buffer_dac_time() {
	return output_buffer_dac_time;
}

void PortAudioCallbackData::set_output_buffer(const Ref<StreamPeerBuffer> &p_output_buffer) {
	output_buffer = p_output_buffer;
}

Ref<StreamPeerBuffer> PortAudioCallbackData::get_output_buffer() {
	return output_buffer;
}

void PortAudioCallbackData::set_input_buffer(const Ref<StreamPeerBuffer> &p_input_buffer) {
	input_buffer = p_input_buffer;
}

Ref<StreamPeerBuffer> PortAudioCallbackData::get_input_buffer() {
	return input_buffer;
}

void PortAudioCallbackData::set_frames_per_buffer(uint64_t p_frames_per_buffer) {
	frames_per_buffer = p_frames_per_buffer;
}

uint64_t PortAudioCallbackData::get_frames_per_buffer() {
	return frames_per_buffer;
}

void PortAudioCallbackData::set_status_flags(uint64_t p_status_flags) {
	status_flags = p_status_flags;
}

uint64_t PortAudioCallbackData::get_status_flags() {
	return status_flags;
}

void PortAudioCallbackData::set_user_data(const Variant &p_user_data) {
	user_data = p_user_data;
}

Variant PortAudioCallbackData::get_user_data() {
	return user_data;
}

void PortAudioCallbackData::set_last_call_duration(uint64_t p_last_call_duration) {
	last_call_duration = p_last_call_duration;
}

uint64_t PortAudioCallbackData::get_last_call_duration() {
	return last_call_duration;
}

void PortAudioCallbackData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_input_buffer_adc_time"), &PortAudioCallbackData::get_input_buffer_adc_time);
	ClassDB::bind_method(D_METHOD("set_input_buffer_adc_time", "input_buffer_adc_time"), &PortAudioCallbackData::set_input_buffer_adc_time);
	ClassDB::bind_method(D_METHOD("get_current_time"), &PortAudioCallbackData::get_current_time);
	ClassDB::bind_method(D_METHOD("set_current_time", "current_time"), &PortAudioCallbackData::set_current_time);
	ClassDB::bind_method(D_METHOD("get_output_buffer_dac_time"), &PortAudioCallbackData::get_output_buffer_dac_time);
	ClassDB::bind_method(D_METHOD("set_output_buffer_dac_time", "output_buffer_dac_time"), &PortAudioCallbackData::set_output_buffer_dac_time);
	ClassDB::bind_method(D_METHOD("get_output_buffer"), &PortAudioCallbackData::get_output_buffer);
	ClassDB::bind_method(D_METHOD("set_output_buffer", "output_buffer"), &PortAudioCallbackData::set_output_buffer);
	ClassDB::bind_method(D_METHOD("get_input_buffer"), &PortAudioCallbackData::get_input_buffer);
	ClassDB::bind_method(D_METHOD("set_input_buffer", "input_buffer"), &PortAudioCallbackData::set_input_buffer);
	ClassDB::bind_method(D_METHOD("get_frames_per_buffer"), &PortAudioCallbackData::get_frames_per_buffer);
	ClassDB::bind_method(D_METHOD("set_frames_per_buffer", "frames_per_buffer"), &PortAudioCallbackData::set_frames_per_buffer);
	ClassDB::bind_method(D_METHOD("get_status_flags"), &PortAudioCallbackData::get_status_flags);
	ClassDB::bind_method(D_METHOD("set_status_flags", "status_flags"), &PortAudioCallbackData::set_status_flags);
	ClassDB::bind_method(D_METHOD("get_user_data"), &PortAudioCallbackData::get_user_data);
	ClassDB::bind_method(D_METHOD("set_user_data", "user_data"), &PortAudioCallbackData::set_user_data);
	ClassDB::bind_method(D_METHOD("get_last_call_duration"), &PortAudioCallbackData::get_last_call_duration);
	ClassDB::bind_method(D_METHOD("set_last_call_duration", "last_call_duration"), &PortAudioCallbackData::set_last_call_duration);
}

PortAudioCallbackData::PortAudioCallbackData() {
	input_buffer_adc_time = 0;
	current_time = 0;
	output_buffer_dac_time = 0;
	output_buffer = Ref<StreamPeerBuffer>();
	input_buffer = Ref<StreamPeerBuffer>();
	frames_per_buffer = 0;
	status_flags = 0;
	user_data = Variant();
}

PortAudioCallbackData::~PortAudioCallbackData() {
}
