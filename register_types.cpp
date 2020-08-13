#include "register_types.h"

#include "./port_audio.h"
#include "./port_audio_callback_data.h"
#include "./port_audio_stream.h"
#include "./port_audio_stream_parameter.h"

#include "./px/px_audio_reader.h"
#include "./px/px_audio_reader_wav.h"
#include "./px/px_audio_player.h"

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

	// Port Audio Extended
	ClassDB::register_class<PxAudioPlayer>();
	ClassDB::register_virtual_class<PxAudioReader>();
	ClassDB::register_class<PxAudioReaderWav>();

	// Nodes
	ClassDB::register_class<PortAudioTestNode>();
}

void unregister_portaudio_types() {
	memdelete(port_audio);
}
