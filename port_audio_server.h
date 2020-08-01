#ifndef PORTAUDIO_SERVER_H
#define PORTAUDIO_SERVER_H

#include <core/object.h>

class PortAudioServer : public Object {
	GDCLASS(PortAudioServer, Object);

public:
	PortAudioServer();
	~PortAudioServer();
};

#endif
