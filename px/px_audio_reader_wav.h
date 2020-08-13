#ifndef AUDIO_READER_WAV_H
#define AUDIO_READER_WAV_H

#include "px_audio_reader.h"

class PxAudioReaderWav : public PxAudioReader {
	GDCLASS(PxAudioReaderWav, PxAudioReader);

protected:
	static void _bind_methods();
	PxAudioReaderError read(Ref<StreamPeerBuffer> p_buffer) override;

public:
	PxAudioReaderWav();
	~PxAudioReaderWav();
};

#endif
