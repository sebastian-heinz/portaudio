#ifndef PX_AUDIO_PLAYER_H
#define PX_AUDIO_PLAYER_H

#include <core/reference.h>

class PxAudioPlayer : public Reference {
	GDCLASS(PxAudioPlayer, Reference);

protected:
	static void _bind_methods();

public:
	PxAudioPlayer();
	~PxAudioPlayer();
};

#endif
