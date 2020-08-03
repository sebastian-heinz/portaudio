#include "port_audio_test_node.h"

#include <core/engine.h>
#include <scene/gui/box_container.h>
#include <scene/gui/button.h>
#include <scene/gui/check_button.h>
#include <scene/gui/control.h>
#include <scene/gui/label.h>
#include <scene/gui/option_button.h>
#include <scene/gui/scroll_container.h>
#include <scene/gui/separator.h>
#include <scene/gui/split_container.h>

void PortAudioTestNode::_bind_methods() {
}

PortAudioTestNode::PortAudioTestNode() {




	VBoxContainer *root = memnew(VBoxContainer);
	root->set_anchor(Margin::MARGIN_BOTTOM, 1.f);
	root->set_anchor(Margin::MARGIN_RIGHT, 1.f);
	root->set_anchor(Margin::MARGIN_TOP, .0f);
	root->set_anchor(Margin::MARGIN_LEFT, .0f);

	add_child(root);
}

PortAudioTestNode::~PortAudioTestNode() {
}
