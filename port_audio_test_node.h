#ifndef PORT_AUDIO_TEST_NODE_H
#define PORT_AUDIO_TEST_NODE_H

#include "port_audio.h"

#include <scene/2d/node_2d.h>

class PortAudioTestNode : public Node2D {
	GDCLASS(PortAudioTestNode, Node2D);


protected:
	static void _bind_methods();

public:
	PortAudioTestNode();
	~PortAudioTestNode();
};

#endif
