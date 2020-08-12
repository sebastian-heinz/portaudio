#include "audio_reader_wav.h"

void AudioReaderWav::_bind_methods() {
}

AudioReader::AudioReaderError AudioReaderWav::read(Ref<StreamPeerBuffer> p_buffer) {
	// WAV references:
	// http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
	// http://soundfile.sapp.org/doc/WaveFormat/

	bool is_riff = p_buffer->get_u8() == 'R' && p_buffer->get_u8() == 'I' && p_buffer->get_u8() == 'F' && p_buffer->get_u8() == 'F';
	if (!is_riff) {
		return AudioReaderError::ERROR;
	}

	// 36 + SubChunk2Size, or more precisely: 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
	// This is the size of the rest of the chunk following this number. This is the size of the
	// entire file in bytes minus 8 bytes for the two fields not included in this count: ChunkID and ChunkSize.
	uint32_t chunk_size = p_buffer->get_u32();

	bool is_wave = p_buffer->get_u8() == 'W' && p_buffer->get_u8() == 'A' && p_buffer->get_u8() == 'V' && p_buffer->get_u8() == 'E';
	if (!is_wave) {
		return AudioReaderError::ERROR;
	}

	bool has_fmt = false;
	bool has_data = false;
	while (p_buffer->get_available_bytes() > 0) {
		char chunk_id[4];
		p_buffer->get_data((uint8_t *)&chunk_id, 4);

		uint32_t chunk_size = p_buffer->get_u32();
		uint32_t chunk_position = p_buffer->get_position();

		if (!has_fmt && chunk_id[0] == 'f' && chunk_id[1] == 'm' && chunk_id[2] == 't' && chunk_id[3] == ' ') {
			// PCM = 1 (i.e. Linear quantization) Values other than 1 indicate some form of compression.
			//0x0001 WAVE_FORMAT_PCM PCM
			//0x0003 WAVE_FORMAT_IEEE_FLOAT IEEE float
			//0x0006 WAVE_FORMAT_ALAW 8 - bit ITU - T G .711 A - law
			//0x0007 WAVE_FORMAT_MULAW 8 - bit ITU - T G .711 µ- law
			//0xFFFE WAVE_FORMAT_EXTENSIBLE Determined by SubFormat
			uint16_t audio_format = p_buffer->get_u16();

			// Mono = 1, Stereo = 2, etc.
			uint16_t num_channels = p_buffer->get_u16();

			// 8000, 44100, etc.
			uint32_t sample_rate_file = p_buffer->get_u32();

			// == SampleRate * NumChannels * BitsPerSample/8
			uint32_t byte_range = p_buffer->get_u32();

			// == NumChannels * BitsPerSample/8 The number of bytes for one sample including all channels.
			uint16_t block_align = p_buffer->get_u16();

			// 8 bits = 8, 16 bits = 16, etc.
			uint16_t bits_per_sample = p_buffer->get_u16();

			if (bits_per_sample == 8) {
				format = PortAudioStreamParameter::PortAudioSampleFormat::U_INT_8;
			} else if (bits_per_sample == 16) {
				format = PortAudioStreamParameter::PortAudioSampleFormat::INT_16;
			} else if (audio_format == 3 && bits_per_sample == 32) {
				format = PortAudioStreamParameter::PortAudioSampleFormat::FLOAT_32;
			} else {
				return AudioReaderError::ERROR;
			}
			sample_rate = sample_rate_file;
			channels = num_channels;
			has_fmt = true;
		}

		if (!has_data && chunk_id[0] == 'd' && chunk_id[1] == 'a' && chunk_id[2] == 't' && chunk_id[3] == 'a') {
			PoolByteArray data = p_buffer->get_data_array();
			PoolByteArray::Read r = data.read();
			const uint8_t *ptr = r.ptr();
			sample_buffer.instance();
			sample_buffer->put_data(&ptr[chunk_position], chunk_size);
			sample_buffer->seek(0);
			has_data = true;
		}

		p_buffer->seek(chunk_position + chunk_size);
	}

	return AudioReaderError::NO_ERROR;
}

AudioReaderWav::AudioReaderWav() {
}

AudioReaderWav::~AudioReaderWav() {
}
