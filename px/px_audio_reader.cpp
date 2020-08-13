#include "px_audio_reader.h"

#include "core/os/file_access.h"

PxAudioReader::PxAudioReaderError PxAudioReader::read_file(const String &p_path) {
	file_path = p_path;
	file_extension = p_path.get_extension();
	Error err;
	Vector<uint8_t> data_array = FileAccess::get_file_as_array(file_path, &err);
	if (err) {
		return PxAudioReaderError::READ_FILE_FAILED;
	}
	Ref<StreamPeerBuffer> buffer;
	buffer.instance();
	buffer->put_data(data_array.ptr(), data_array.size());
	buffer->seek(0);
	return read(buffer);
}

PxAudioReader::PxAudioReaderError PxAudioReader::read_buffer(Ref<StreamPeerBuffer> p_buffer) {
	p_buffer->seek(0);
	return read(p_buffer);
}

PxAudioReader::PxAudioReaderError PxAudioReader::read_ptr(void *p_buffer, uint32_t p_length) {
	uint8_t *ptr = (uint8_t *)p_buffer;
	Ref<StreamPeerBuffer> buffer;
	buffer.instance();
	buffer->put_data(ptr, p_length);
	buffer->seek(0);
	return read(buffer);
}

double PxAudioReader::get_sample_rate() {
	return sample_rate;
}

int PxAudioReader::get_channels() {
	return channels;
}

PortAudioStreamParameter::PortAudioSampleFormat PxAudioReader::get_format() {
	return format;
}

Ref<StreamPeerBuffer> PxAudioReader::get_sample_buffer() {
	return sample_buffer;
}

void PxAudioReader::_bind_methods() {
	ClassDB::bind_method(D_METHOD("read_file", "file_path"), &PxAudioReader::read_file);
	ClassDB::bind_method(D_METHOD("read_buffer", "stream_peer_buffer"), &PxAudioReader::read_buffer);
	ClassDB::bind_method(D_METHOD("get_sample_rate"), &PxAudioReader::get_sample_rate);
	ClassDB::bind_method(D_METHOD("get_channels"), &PxAudioReader::get_channels);
	ClassDB::bind_method(D_METHOD("get_format"), &PxAudioReader::get_format);
	ClassDB::bind_method(D_METHOD("get_sample_buffer"), &PxAudioReader::get_sample_buffer);

	// PortAudioReaderError
	BIND_ENUM_CONSTANT(NO_ERROR);
	BIND_ENUM_CONSTANT(ERROR);
	BIND_ENUM_CONSTANT(READ_FILE_FAILED);
}

PxAudioReader::PxAudioReader() {
	sample_buffer = Ref<StreamPeerBuffer>();
	format = PortAudioStreamParameter::PortAudioSampleFormat::CUSTOM_FORMAT;
	sample_rate = 0;
	channels = 0;
	file_path = String();
	file_extension = String();
}

PxAudioReader::~PxAudioReader() {
}
