#include "port_audio_stream_parameter.h"

#include "core/os/memory.h"

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

void PortAudioStreamParameter::set_sample_format(PortAudioSampleFormat p_sample_format) {
	sample_format = p_sample_format;
}

PortAudioStreamParameter::PortAudioSampleFormat PortAudioStreamParameter::get_sample_format() {
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
	ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_format", PROPERTY_HINT_ENUM, "FLOAT_32, INT_32, INT_24, INT_16, INT_8, U_INT_8, CUSTOM_FORMAT, NON_INTERLEAVED"), "set_sample_format", "get_sample_format");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "suggested_latency"), "set_suggested_latency", "get_suggested_latency");

	// PortAudioSampleSize
	BIND_ENUM_CONSTANT(FLOAT_32);
	BIND_ENUM_CONSTANT(INT_32);
	BIND_ENUM_CONSTANT(INT_24);
	BIND_ENUM_CONSTANT(INT_16);
	BIND_ENUM_CONSTANT(INT_8);
	BIND_ENUM_CONSTANT(U_INT_8);
	BIND_ENUM_CONSTANT(CUSTOM_FORMAT);
	BIND_ENUM_CONSTANT(NON_INTERLEAVED);
}

PortAudioStreamParameter::PortAudioStreamParameter() {
	device_index = 0;
	channel_count = 0;
	sample_format = PortAudioStreamParameter::PortAudioSampleFormat::FLOAT_32;
	suggested_latency = 0;
	host_api_specific_stream_info = nullptr;
}

PortAudioStreamParameter::~PortAudioStreamParameter() {
	if (host_api_specific_stream_info != nullptr) {
		memfree(host_api_specific_stream_info);
	}
}
