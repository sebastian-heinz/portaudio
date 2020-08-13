#include "port_audio_test_node.h"

#include "port_audio.h"

#include <scene/gui/box_container.h>
#include <scene/gui/label.h>
#include <scene/gui/option_button.h>
#include <scene/gui/scroll_container.h>
#include <scene/gui/separator.h>

PortAudioTestNode::PortAudioTestNode() {

	set_anchor(Margin::MARGIN_BOTTOM, 1.f);
	set_anchor(Margin::MARGIN_RIGHT, 1.f);
	set_anchor(Margin::MARGIN_TOP, .0f);
	set_anchor(Margin::MARGIN_LEFT, .0f);

	VBoxContainer *root = memnew(VBoxContainer);
	root->set_anchor(Margin::MARGIN_BOTTOM, 1.f);
	root->set_anchor(Margin::MARGIN_RIGHT, .0f);
	root->set_anchor(Margin::MARGIN_TOP, .0f);
	root->set_anchor(Margin::MARGIN_LEFT, .0f);
	root->set_v_size_flags(Control::SizeFlags::SIZE_FILL | Control::SizeFlags::SIZE_EXPAND);

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

	device_count = PortAudio::get_singleton()->get_device_count();
	Label *label_device_count = memnew(Label);
	label_device_count->set_text(vformat("Device Count: %d", device_count));
	root->add_child(label_device_count);

	int default_host_api = PortAudio::get_singleton()->get_default_host_api();
	selected_host_api = default_host_api;
	Label *label_default_host_api = memnew(Label);
	label_default_host_api->set_text(vformat("Default Host API: %d", default_host_api));
	root->add_child(label_default_host_api);

	int default_input_device = PortAudio::get_singleton()->get_default_input_device();
	Label *label_default_input_device = memnew(Label);
	label_default_input_device->set_text(vformat("Default Input Device: %d", default_input_device));
	root->add_child(label_default_input_device);

	int default_output_device = PortAudio::get_singleton()->get_default_output_device();
	Label *label_default_output_device = memnew(Label);
	label_default_output_device->set_text(vformat("Default Output Device: %d", default_output_device));
	root->add_child(label_default_output_device);

	OptionButton *option_host_api = memnew(OptionButton);
	for (int host_api = 0; host_api < host_api_count; host_api++) {
		Dictionary host_api_info = PortAudio::get_singleton()->get_host_api_info(host_api);
		option_host_api->add_item(vformat("%s Devices: %d", host_api_info["name"], host_api_info["device_count"]));
	}
	option_host_api->select(selected_host_api);
	option_host_api->connect("item_selected", this, "option_host_api_selected");
	Label *label_host_api_info = memnew(Label);
	label_host_api_info->set_text("Host API");
	HBoxContainer *container_host_api_info = memnew(HBoxContainer);
	container_host_api_info->add_child(label_host_api_info);
	container_host_api_info->add_child(option_host_api);
	root->add_child(container_host_api_info);

	root->add_child(memnew(HSeparator));

	VBoxContainer *container_device_info = memnew(VBoxContainer);
	container_device_info->set_v_size_flags(Control::SizeFlags::SIZE_FILL | Control::SizeFlags::SIZE_EXPAND);
	current_device_info = container_device_info;
	root->add_child(current_device_info);

	add_child(root);

	option_host_api_selected(selected_host_api);
}

void PortAudioTestNode::option_host_api_selected(int p_index) {
	selected_host_api = p_index;

	int child_index = current_device_info->get_child_count();
	child_index--;
	for (; child_index >= 0; child_index--) {
		Node *child = current_device_info->get_child(child_index);
		child->queue_delete();
		current_device_info->remove_child(child);
	}

	Dictionary host_api_info = PortAudio::get_singleton()->get_host_api_info(selected_host_api);
	int host_api_info_count = host_api_info.size();
	VBoxContainer *container_host_api_info = memnew(VBoxContainer);
	for (int host_api_info_index = 0; host_api_info_index < host_api_info_count; host_api_info_index++) {
		String key = host_api_info.get_key_at_index(host_api_info_index);
		String value = host_api_info.get_value_at_index(host_api_info_index);
		Label *label_host_api_info = memnew(Label);
		label_host_api_info->set_text(vformat("%s: %s", key, value));
		container_host_api_info->add_child(label_host_api_info);
	}
	current_device_info->add_child(container_host_api_info);

	current_device_info->add_child(memnew(HSeparator));

	VBoxContainer *container_device_info_collection = memnew(VBoxContainer);
	container_device_info_collection->set_v_size_flags(Control::SizeFlags::SIZE_FILL | Control::SizeFlags::SIZE_EXPAND);
	for (int device = 0; device < device_count; device++) {
		VBoxContainer *container_device_info = memnew(VBoxContainer);
		Label *label_device_index = memnew(Label);
		label_device_index->set_text(vformat("DeviceIndex: %s", device));
		container_device_info->add_child(label_device_index);

		Dictionary device_info = PortAudio::get_singleton()->get_device_info(device);
		int device_info_count = device_info.size();
		int host_api_index = device_info["host_api"];
		if (host_api_index != selected_host_api) {
			continue;
		}
		for (int device_info_index = 0; device_info_index < device_info_count; device_info_index++) {
			String key = device_info.get_key_at_index(device_info_index);
			String value = device_info.get_value_at_index(device_info_index);
			Label *label_device_info = memnew(Label);
			label_device_info->set_text(vformat("%s: %s", key, value));
			container_device_info->add_child(label_device_info);
		}
		container_device_info_collection->add_child(container_device_info);
		container_device_info_collection->add_child(memnew(HSeparator));
	}

	ScrollContainer *scroll_device_info_collection = memnew(ScrollContainer);
	scroll_device_info_collection->set_v_size_flags(Control::SizeFlags::SIZE_FILL | Control::SizeFlags::SIZE_EXPAND);
	scroll_device_info_collection->set_follow_focus(true);
	scroll_device_info_collection->set_enable_h_scroll(false);
	scroll_device_info_collection->set_enable_v_scroll(true);
	scroll_device_info_collection->add_child(container_device_info_collection);

	current_device_info->add_child(scroll_device_info_collection);
}

void PortAudioTestNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("option_host_api_selected", "index"), &PortAudioTestNode::option_host_api_selected);
}

PortAudioTestNode::~PortAudioTestNode() {
}
