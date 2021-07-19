#ifndef PORT_AUDIO_CALLBACK_DATA_H
#define PORT_AUDIO_CALLBACK_DATA_H

#include "core/io/stream_peer.h"
#include "core/object/ref_counted.h"

class PortAudioCallbackData : public RefCounted {
	GDCLASS(PortAudioCallbackData, RefCounted);

private:
	double input_buffer_adc_time;
	double current_time;
	double output_buffer_dac_time;
	Ref<StreamPeerBuffer> output_buffer;
	Ref<StreamPeerBuffer> input_buffer;
	uint64_t frames_per_buffer;
	uint64_t status_flags;
	Variant user_data;
	uint64_t last_call_duration;

protected:
	static void _bind_methods();

public:
	void set_input_buffer_adc_time(double p_input_buffer_adc_time);
	double get_input_buffer_adc_time();
	void set_current_time(double p_current_time);
	double get_current_time();
	void set_output_buffer_dac_time(double p_output_buffer_dac_time);
	double get_output_buffer_dac_time();
	void set_output_buffer(const Ref<StreamPeerBuffer> &p_output_buffer);
	Ref<StreamPeerBuffer> get_output_buffer();
	void set_input_buffer(const Ref<StreamPeerBuffer> &p_input_buffer);
	Ref<StreamPeerBuffer> get_input_buffer();
	void set_frames_per_buffer(uint64_t p_frames_per_buffer);
	uint64_t get_frames_per_buffer();
	void set_status_flags(uint64_t p_status_flags);
	uint64_t get_status_flags();
	void set_user_data(const Variant &p_user_data);
	Variant get_user_data();
	void set_last_call_duration(uint64_t p_last_call_duration);
	uint64_t get_last_call_duration();

	PortAudioCallbackData();
	~PortAudioCallbackData();
};

#endif
