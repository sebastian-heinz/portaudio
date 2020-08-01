#include "register_types.h"

#include "./port_audio.h"
#include "./port_audio_node.h"

#include <core/class_db.h>
#include <core/engine.h>

static PortAudio *port_audio = NULL;

void register_portaudio_types() {
	// Singleton
	port_audio = memnew(PortAudio);
	ClassDB::register_class<PortAudio>();
	Engine::get_singleton()->add_singleton(Engine::Singleton("PortAudio", PortAudio::get_singleton()));

	// Classes
	ClassDB::register_class<PortAudioNode>();
}

void unregister_portaudio_types() {
	memdelete(port_audio);
}
