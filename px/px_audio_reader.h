#ifndef PX_AUDIO_READER_H
#define PX_AUDIO_READER_H

#include "./../port_audio_stream_parameter.h"

#include <core/io/stream_peer.h>
#include <core/reference.h>

class PxAudioReader : public Reference {
	GDCLASS(PxAudioReader, Reference);

public:
	enum PxAudioReaderError {
		NO_ERROR = 0,
		ERROR = -1000,
		READ_FILE_FAILED = -1001,
	};

private:
	String file_path;
	String file_extension;

protected:
	static void _bind_methods();

	virtual PxAudioReaderError read(Ref<StreamPeerBuffer> p_buffer) = 0;

	Ref<StreamPeerBuffer> sample_buffer;
	PortAudioStreamParameter::PortAudioSampleFormat format;
	double sample_rate;
	int channels;

public:
	PxAudioReaderError read_file(const String &p_path);
	PxAudioReaderError read_buffer(Ref<StreamPeerBuffer> p_buffer);
	PxAudioReaderError read_ptr(void *p_buffer, uint32_t p_length);
	double get_sample_rate();
	int get_channels();
	PortAudioStreamParameter::PortAudioSampleFormat get_format();
	Ref<StreamPeerBuffer> get_sample_buffer();

	PxAudioReader();
	~PxAudioReader();
};

VARIANT_ENUM_CAST(PxAudioReader::PxAudioReaderError);

#endif
