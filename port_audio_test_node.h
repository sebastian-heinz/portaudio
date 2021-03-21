#ifndef PORT_AUDIO_TEST_NODE_H
#define PORT_AUDIO_TEST_NODE_H

#include "scene/gui/control.h"

class PortAudioTestNode : public Control {
GDCLASS(PortAudioTestNode, Control);

private:
    int device_count;
    int selected_host_api;
    Node *current_device_info;

    void option_host_api_selected(int p_index);

protected:
    static void _bind_methods();

public:
    PortAudioTestNode();

    ~PortAudioTestNode();
};

#endif
