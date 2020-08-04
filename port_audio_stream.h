#ifndef PORT_AUDIO_STREAM_H
#define PORT_AUDIO_STREAM_H

#include <core/reference.h>

class PortAudioStream : public Reference {
	GDCLASS(PortAudioStream, Reference);


protected:
	static void _bind_methods();


public:
	PortAudioStream();
	~PortAudioStream();
};

#endif
