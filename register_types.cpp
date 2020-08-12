#include "register_types.h"

#include "./port_audio.h"
#include "./port_audio_callback_data.h"
#include "./port_audio_stream.h"
#include "./port_audio_stream_parameter.h"

#include "./audio/audio_reader.h"
#include "./audio/audio_reader_wav.h"

#include "./port_audio_test_node.h"

#include <core/class_db.h>
#include <core/engine.h>

static PortAudio *port_audio = NULL;

void register_portaudio_types() {
	// Singleton
	port_audio = memnew(PortAudio);
	ClassDB::register_class<PortAudio>();
	Engine::get_singleton()->add_singleton(Engine::Singleton("PortAudio", PortAudio::get_singleton()));

	// Port Audio
	ClassDB::register_class<PortAudioStream>();
	ClassDB::register_class<PortAudioStreamParameter>();
	ClassDB::register_class<PortAudioCallbackData>();

	// Audio Reader
	ClassDB::register_virtual_class<AudioReader>();
	ClassDB::register_class<AudioReaderWav>();

	// Custom Nodes
	ClassDB::register_class<PortAudioTestNode>();
}

void unregister_portaudio_types() {
	memdelete(port_audio);
}
