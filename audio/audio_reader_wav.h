#ifndef AUDIO_READER_WAV_H
#define AUDIO_READER_WAV_H

#include "audio_reader.h"

class AudioReaderWav : public AudioReader {
	GDCLASS(AudioReaderWav, AudioReader);

protected:
	static void _bind_methods();
	AudioReaderError read(Ref<StreamPeerBuffer> p_buffer) override;

public:
	AudioReaderWav();
	~AudioReaderWav();
};

#endif
