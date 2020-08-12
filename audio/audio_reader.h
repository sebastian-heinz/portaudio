#ifndef AUDIO_READER_H
#define AUDIO_READER_H

#include "./../port_audio_stream_parameter.h"

#include <core/io/stream_peer.h>
#include <core/reference.h>

class AudioReader : public Reference {
	GDCLASS(AudioReader, Reference);

public:
	enum AudioReaderError {
		NO_ERROR = 0,
		ERROR = -1000,
		READ_FILE_FAILED = -1001,
	};

private:
	String file_path;
	String file_extension;

protected:
	static void _bind_methods();

	virtual AudioReaderError read(Ref<StreamPeerBuffer> p_buffer) = 0;

	Ref<StreamPeerBuffer> sample_buffer;
	PortAudioStreamParameter::PortAudioSampleFormat format;
	double sample_rate;
	int channels;

public:
	AudioReaderError read_file(const String &p_path);
	AudioReaderError read_buffer(Ref<StreamPeerBuffer> p_buffer);
	AudioReaderError read_ptr(void *p_buffer, uint32_t p_length);
	double get_sample_rate();
	int get_channels();
	PortAudioStreamParameter::PortAudioSampleFormat get_format();
	Ref<StreamPeerBuffer> get_sample_buffer();

	AudioReader();
	~AudioReader();
};

VARIANT_ENUM_CAST(AudioReader::AudioReaderError);

#endif
