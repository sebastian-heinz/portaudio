#include "register_types.h"

#include "./port_audio.h"
#include "./port_audio_callback_data.h"
#include "./port_audio_stream.h"
#include "./port_audio_stream_parameter.h"

#include "./port_audio_test_node.h"

#include "core/object/class_db.h"
#include "core/config/engine.h"

static PortAudio *port_audio = nullptr;

void register_portaudio_types() {
	// Singleton
	port_audio = memnew(PortAudio);
	ClassDB::register_class<PortAudio>();
	Engine::get_singleton()->add_singleton(Engine::Singleton("PortAudio", PortAudio::get_singleton()));

	// Port Audio
	ClassDB::register_class<PortAudioStream>();
	ClassDB::register_class<PortAudioStreamParameter>();
	ClassDB::register_class<PortAudioCallbackData>();

	// Nodes
	ClassDB::register_class<PortAudioTestNode>();
}

void unregister_portaudio_types() {
	memdelete(port_audio);
}
