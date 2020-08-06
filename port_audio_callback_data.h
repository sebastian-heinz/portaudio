#ifndef PORT_AUDIO_CALLBACK_DATA_H
#define PORT_AUDIO_CALLBACK_DATA_H

#include <core/reference.h>
#include <core/io/stream_peer.h>

class PortAudioCallbackData : public Reference {
	GDCLASS(PortAudioCallbackData, Reference);

private:
	double input_buffer_adc_time;
	double current_time;
	double output_buffer_dac_time;
	Ref<StreamPeerBuffer> output_buffer;
	PoolVector<uint8_t> input_buffer;
	uint64_t frames_per_buffer;
	uint64_t status_flags;
	Variant user_data;

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

	void set_input_buffer(const PoolVector<uint8_t> &p_input_buffer);
	PoolVector<uint8_t> get_input_buffer();
	PoolVector<uint8_t> *get_input_buffer_ptr();

	void set_frames_per_buffer(uint64_t p_frames_per_buffer);
	uint64_t get_frames_per_buffer();
	void set_status_flags(uint64_t p_status_flags);
	uint64_t get_status_flags();
	void set_user_data(const Variant &p_user_data);
	Variant get_user_data();

	PortAudioCallbackData();
	~PortAudioCallbackData();
};

#endif
