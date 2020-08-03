#include "port_audio_test_node.h"

#include "port_audio.h"

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

	int version = PortAudio::get_singleton()->get_version();
	Label *label_version = memnew(Label);
	label_version->set_text(vformat("Version: %d", version));
	root->add_child(label_version);

	String version_text = PortAudio::get_singleton()->get_version_text();
	Label *label_version_text = memnew(Label);
	label_version_text->set_text(vformat("Version Text: %s", version_text));
	root->add_child(label_version_text);

	int host_api_count = PortAudio::get_singleton()->get_host_api_count();
	Label *label_host_api_count = memnew(Label);
	label_host_api_count->set_text(vformat("Host API Count: %d", host_api_count));
	root->add_child(label_host_api_count);

	int default_host_api = PortAudio::get_singleton()->get_default_host_api();
	Label *label_default_host_api = memnew(Label);
	label_default_host_api->set_text(vformat("Default Host API: %d", default_host_api));
	root->add_child(label_default_host_api);

	for (int host_api = 0; host_api < host_api_count; host_api++) {
		Dictionary host_api_info = PortAudio::get_singleton()->get_host_api_info(host_api);
		int host_api_info_count = host_api_info.size();
		for (int host_api_info_index = 0; host_api_info_index < host_api_info_count; host_api_info_index++) {
			String key = host_api_info.get_key_at_index(host_api_info_index);
			String value = host_api_info.get_value_at_index(host_api_info_index);
			Label *label_host_api_info = memnew(Label);
			label_host_api_info->set_text(vformat("%s: %s", key, value));
			// TODO sub container
			root->add_child(label_host_api_info);
		}
	}

	int device_count = PortAudio::get_singleton()->get_device_count();
	Label *label_device_count = memnew(Label);
	label_device_count->set_text(vformat("Device Count: %d", device_count));
	root->add_child(label_device_count);

	int default_input_device = PortAudio::get_singleton()->get_default_input_device();
	Label *label_default_input_device = memnew(Label);
	label_default_input_device->set_text(vformat("Default Input Device: %d", default_input_device));
	root->add_child(label_default_input_device);

	int default_output_device = PortAudio::get_singleton()->get_default_output_device();
	Label *label_default_output_device = memnew(Label);
	label_default_output_device->set_text(vformat("Default Output Device: %d", default_output_device));
	root->add_child(label_default_output_device);

	for (int device = 0; device < device_count; device++) {
		Dictionary device_info = PortAudio::get_singleton()->get_device_info(device);
		int device_info_count = device_info.size();
		for (int device_info_index = 0; device_info_index < device_info_count; device_info_index++) {
			String key = device_info.get_key_at_index(device_info_index);
			String value = device_info.get_value_at_index(device_info_index);
			Label *label_device_info = memnew(Label);
			label_device_info->set_text(vformat("%s: %s", key, value));
			// TODO sub container
			root->add_child(label_device_info);
		}
	}

	// TODO file select / start / stop stream etc..

	add_child(root);
}

PortAudioTestNode::~PortAudioTestNode() {
}
