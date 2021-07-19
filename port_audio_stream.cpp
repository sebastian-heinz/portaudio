#include "port_audio_stream.h"

int PortAudioStream::get_input_channel_count() {
	if (input_stream_parameter.is_null()) {
		return 0;
	}
	return input_stream_parameter->get_channel_count();
}

void PortAudioStream::set_input_channel_count(int p_input_channel_count) {
	if (p_input_channel_count <= 0) {
		if (input_stream_parameter.is_valid()) {
			input_stream_parameter.unref();
		}
	}
	if (input_stream_parameter.is_null()) {
		input_stream_parameter.instantiate();
	}
	input_stream_parameter->set_channel_count(p_input_channel_count);
}

int PortAudioStream::get_output_channel_count() {
	if (output_stream_parameter.is_null()) {
		return 0;
	}
	return output_stream_parameter->get_channel_count();
}

void PortAudioStream::set_output_channel_count(int p_output_channel_count) {
	if (p_output_channel_count <= 0) {
		if (output_stream_parameter.is_valid()) {
			output_stream_parameter.unref();
		}
	}
	if (output_stream_parameter.is_null()) {
		output_stream_parameter.instantiate();
	}
	output_stream_parameter->set_channel_count(p_output_channel_count);
}

double PortAudioStream::get_sample_rate() {
	return sample_rate;
}

void PortAudioStream::set_sample_rate(double p_sample_rate) {
	sample_rate = p_sample_rate;
}

unsigned int PortAudioStream::get_frames_per_buffer() {
	return frames_per_buffer;
}

void PortAudioStream::set_frames_per_buffer(unsigned int p_frames_per_buffer) {
	frames_per_buffer = p_frames_per_buffer;
}

Ref<PortAudioStreamParameter> PortAudioStream::get_input_stream_parameter() {
	return input_stream_parameter;
}

void PortAudioStream::set_input_stream_parameter(Ref<PortAudioStreamParameter> p_input_stream_parameter) {
	input_stream_parameter = p_input_stream_parameter;
}

Ref<PortAudioStreamParameter> PortAudioStream::get_output_stream_parameter() {
	return output_stream_parameter;
}

void PortAudioStream::set_output_stream_parameter(Ref<PortAudioStreamParameter> p_output_stream_parameter) {
	output_stream_parameter = p_output_stream_parameter;
}

PortAudioStream::PortAudioStreamFlag PortAudioStream::get_stream_flags() {
	return stream_flags;
}

void PortAudioStream::set_stream_flags(PortAudioStreamFlag p_stream_flags) {
	stream_flags = p_stream_flags;
}

void *PortAudioStream::get_stream() {
	return stream;
}

void PortAudioStream::set_stream(void *p_stream) {
	stream = p_stream;
}

void PortAudioStream::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_input_channel_count"), &PortAudioStream::get_input_channel_count);
	ClassDB::bind_method(D_METHOD("set_input_channel_count", "input_channel_count"), &PortAudioStream::set_input_channel_count);
	ClassDB::bind_method(D_METHOD("get_output_channel_count"), &PortAudioStream::get_output_channel_count);
	ClassDB::bind_method(D_METHOD("set_output_channel_count", "output_channel_count"), &PortAudioStream::set_output_channel_count);
	ClassDB::bind_method(D_METHOD("get_sample_rate"), &PortAudioStream::get_sample_rate);
	ClassDB::bind_method(D_METHOD("set_sample_rate", "sample_rate"), &PortAudioStream::set_sample_rate);
	ClassDB::bind_method(D_METHOD("get_frames_per_buffer"), &PortAudioStream::get_frames_per_buffer);
	ClassDB::bind_method(D_METHOD("set_frames_per_buffer", "frames_per_buffer"), &PortAudioStream::set_frames_per_buffer);
	ClassDB::bind_method(D_METHOD("get_input_stream_parameter"), &PortAudioStream::get_input_stream_parameter);
	ClassDB::bind_method(D_METHOD("set_input_stream_parameter", "input_stream_parameter"), &PortAudioStream::set_input_stream_parameter);
	ClassDB::bind_method(D_METHOD("get_output_stream_parameter"), &PortAudioStream::get_output_stream_parameter);
	ClassDB::bind_method(D_METHOD("set_output_stream_parameter", "output_stream_parameter"), &PortAudioStream::set_output_stream_parameter);
	ClassDB::bind_method(D_METHOD("get_stream_flags"), &PortAudioStream::get_stream_flags);
	ClassDB::bind_method(D_METHOD("set_stream_flags", "stream_flags"), &PortAudioStream::set_stream_flags);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "input_channel_count"), "set_input_channel_count", "get_input_channel_count");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "output_channel_count"), "set_output_channel_count", "get_output_channel_count");
	//ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_format", PROPERTY_HINT_ENUM, "FLOAT_32,INT_32,INT_24,INT_16,INT_8,U_INT_8,CUSTOM_FORMAT,NON_INTERLEAVED"), "set_sample_format", "get_sample_format");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sample_rate"), "set_sample_rate", "get_sample_rate");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frames_per_buffer"), "set_frames_per_buffer", "get_frames_per_buffer");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "input_stream_parameter", PROPERTY_HINT_RESOURCE_TYPE, "PortAudioStreamParameter"), "set_input_stream_parameter", "get_input_stream_parameter");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "output_stream_parameter", PROPERTY_HINT_RESOURCE_TYPE, "PortAudioStreamParameter"), "set_output_stream_parameter", "get_output_stream_parameter");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "stream_flags", PROPERTY_HINT_FLAGS, "NO_FLAG, CLIP_OFF, DITHER_OFF, NEVER_DROP_INPUT, PRIME_OOUTPUT_BUFFERS_USING_STREAM_CALLBACK, PLATFORM_SPECIFIC_FLAGS"), "set_stream_flags", "get_stream_flags");

	// PortAudioStreamFlag
	BIND_ENUM_CONSTANT(NO_FLAG);
	BIND_ENUM_CONSTANT(CLIP_OFF);
	BIND_ENUM_CONSTANT(DITHER_OFF);
	BIND_ENUM_CONSTANT(NEVER_DROP_INPUT);
	BIND_ENUM_CONSTANT(PRIME_OOUTPUT_BUFFERS_USING_STREAM_CALLBACK);
	BIND_ENUM_CONSTANT(PLATFORM_SPECIFIC_FLAGS);
}

PortAudioStream::PortAudioStream() {
	stream = nullptr;
	sample_rate = 44100.0;
	frames_per_buffer = 0; // paFramesPerBufferUnspecified (0)
	input_stream_parameter = Ref<PortAudioStreamParameter>();
	output_stream_parameter = Ref<PortAudioStreamParameter>();
	stream_flags = NO_FLAG;
}

PortAudioStream::~PortAudioStream() {
}
