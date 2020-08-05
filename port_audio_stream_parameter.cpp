#include "port_audio_stream_parameter.h"

void PortAudioStreamParameter::set_device_index(int p_device_index) {
	device_index = p_device_index;
}

int PortAudioStreamParameter::get_device_index() {
	return device_index;
}

void PortAudioStreamParameter::set_channel_count(int p_channel_count) {
	channel_count = p_channel_count;
}

int PortAudioStreamParameter::get_channel_count() {
	return channel_count;
}

void PortAudioStreamParameter::set_sample_format(uint64_t p_sample_format) {
	sample_format = p_sample_format;
}

uint64_t PortAudioStreamParameter::get_sample_format() {
	return sample_format;
}

void PortAudioStreamParameter::set_suggested_latency(double p_suggested_latency) {
	suggested_latency = p_suggested_latency;
}

double PortAudioStreamParameter::get_suggested_latency() {
	return suggested_latency;
}

void PortAudioStreamParameter::set_host_api_specific_stream_info(void *p_host_api_specific_stream_info) {
	host_api_specific_stream_info = p_host_api_specific_stream_info;
}

void *PortAudioStreamParameter::get_host_api_specific_stream_info() {
	return host_api_specific_stream_info;
}

void PortAudioStreamParameter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_device_index"), &PortAudioStreamParameter::get_device_index);
	ClassDB::bind_method(D_METHOD("set_device_index", "device_index"), &PortAudioStreamParameter::set_device_index);
	ClassDB::bind_method(D_METHOD("get_channel_count"), &PortAudioStreamParameter::get_channel_count);
	ClassDB::bind_method(D_METHOD("set_channel_count", "channel_count"), &PortAudioStreamParameter::set_channel_count);
	ClassDB::bind_method(D_METHOD("get_sample_format"), &PortAudioStreamParameter::get_sample_format);
	ClassDB::bind_method(D_METHOD("set_sample_format", "sample_format"), &PortAudioStreamParameter::set_sample_format);
	ClassDB::bind_method(D_METHOD("get_suggested_latency"), &PortAudioStreamParameter::get_suggested_latency);
	ClassDB::bind_method(D_METHOD("set_suggested_latency", "suggested_latency"), &PortAudioStreamParameter::set_suggested_latency);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "device_index"), "set_device_index", "get_device_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "channel_count"), "set_channel_count", "get_channel_count");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_format"), "set_sample_format", "get_sample_format");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "suggested_latency"), "set_suggested_latency", "get_suggested_latency");
}

PortAudioStreamParameter::PortAudioStreamParameter() {
	device_index = 0;
	channel_count = 0;
	sample_format = 0;
	suggested_latency = 0;
	host_api_specific_stream_info = NULL;
}

PortAudioStreamParameter::~PortAudioStreamParameter() {
}
