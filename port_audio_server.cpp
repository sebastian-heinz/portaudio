#include "port_audio_server.h"

#include "portaudio/include/portaudio.h"

PortAudioServer::PortAudioServer() {
		PaError err = Pa_Initialize();
	if( err != paNoError ) {
	//	return 1;
	}

	//return 0;
}

PortAudioServer::~PortAudioServer() {
}
