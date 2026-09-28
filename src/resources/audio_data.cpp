#include <soundcoe/resources/audio_data.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <exception>

#include <dr_libs/dr_wav.h>
#include <dr_libs/dr_mp3.h>
#include <stb/stb_vorbis.h>

#include <soundcoe_config.hpp>
#if SOUNDCOE_USE_LOGCOE
#include <logcoe.hpp>
#endif

namespace soundcoe
{
    namespace detail
    {
        audio_data::audio_data() : m_pcm_data(nullptr), m_pcm_data_size(0), m_channels(0), m_bits_per_sample(0), m_sample_rate(0),
                                    m_duration(0.0f), m_openal_format(AL_NONE), m_source_format(audio_format::unsupported) { }

        audio_data::audio_data(ALvoid *pcm_data, ALsizei pcm_data_size, ALsizei channels, ALsizei bits_per_sample,
                             ALsizei sample_rate, audio_format source_format) : m_pcm_data(pcm_data), m_pcm_data_size(pcm_data_size), m_channels(channels), m_bits_per_sample(bits_per_sample),
                                                                             m_sample_rate(sample_rate), m_source_format(source_format)
        {
            m_openal_format = calculate_openal_format(m_channels, m_bits_per_sample);
            ALsizei bytes_per_sample = (m_bits_per_sample / 8) * m_channels;
            m_duration = static_cast<ALfloat>(m_pcm_data_size) / (bytes_per_sample * m_sample_rate);
        }

        void audio_data::cleanup()
        {
            if (!m_pcm_data)
                return;

            switch (m_source_format)
            {
            case audio_format::wav:
                drwav_free(m_pcm_data, nullptr);
                break;
            case audio_format::mp3:
                drmp3_free(m_pcm_data, nullptr);
                break;
            case audio_format::ogg:
            default:
                free(m_pcm_data);
                break;
            }
        }

        ALenum audio_data::calculate_openal_format(ALsizei channels, ALsizei bits_per_sample)
        {
            if (channels == 1)
            {
                if (bits_per_sample == 8)
                    return AL_FORMAT_MONO8;
                if (bits_per_sample == 16)
                    return AL_FORMAT_MONO16;
            }
            else if (channels == 2)
            {
                if (bits_per_sample == 8)
                    return AL_FORMAT_STEREO8;
                if (bits_per_sample == 16)
                    return AL_FORMAT_STEREO16;
            }

            return AL_NONE;
        }

        audio_data::audio_data(audio_data &&other) noexcept : m_pcm_data(other.m_pcm_data), m_pcm_data_size(other.m_pcm_data_size), m_channels(other.m_channels),
                                                        m_bits_per_sample(other.m_bits_per_sample), m_sample_rate(other.m_sample_rate), m_duration(other.m_duration),
                                                        m_openal_format(other.m_openal_format), m_source_format(other.m_source_format)
        {
            other.m_pcm_data = nullptr;
        }

        audio_data &audio_data::operator=(audio_data &&other) noexcept
        {
            if (this == &other)
                return *this;

            cleanup();

            m_pcm_data = other.m_pcm_data;
            m_pcm_data_size = other.m_pcm_data_size;
            m_channels = other.m_channels;
            m_bits_per_sample = other.m_bits_per_sample;
            m_sample_rate = other.m_sample_rate;
            m_duration = other.m_duration;
            m_openal_format = other.m_openal_format;
            m_source_format = other.m_source_format;

            other.m_pcm_data = nullptr;
            return *this;
        }

        audio_data::~audio_data() { cleanup(); }

        audio_data audio_data::load_from_wav(const std::string &filename)
        {
            unsigned int channels, sample_rate, bits_per_sample;
            drwav_uint64 total_frame_count;
            void *pcm_data;
            drwav wav;
            if (!drwav_init_file(&wav, filename.c_str(), nullptr))
                error_handler::throw_on_audio_error(filename, audio_format::wav, audio_decoder_operation::open_file);

            bits_per_sample = wav.bitsPerSample;
            drwav_uninit(&wav);

            pcm_data = (bits_per_sample <= 16) ? (void *)drwav_open_file_and_read_pcm_frames_s16(filename.c_str(), &channels, &sample_rate,
                                                                                            &total_frame_count, nullptr)
                                            : (void *)drwav_open_file_and_read_pcm_frames_s32(filename.c_str(), &channels, &sample_rate,
                                                                                            &total_frame_count, nullptr);

            if (!pcm_data)
                error_handler::throw_on_audio_error(filename, audio_format::wav, audio_decoder_operation::decode_audio);

            ALsizei bytes_per_sample = (bits_per_sample <= 16) ? sizeof(drwav_int16) : sizeof(drwav_int32);
            ALsizei pcm_data_size = static_cast<ALsizei>(total_frame_count * channels * bytes_per_sample);

            return audio_data(pcm_data, pcm_data_size, static_cast<ALsizei>(channels), static_cast<ALsizei>(bits_per_sample),
                            static_cast<ALsizei>(sample_rate), audio_format::wav);
        }

        audio_data audio_data::load_from_ogg(const std::string &filename)
        {
            int channels, sample_rate;
            short *pcm_data;
            int total_samples = stb_vorbis_decode_filename(filename.c_str(), &channels, &sample_rate, &pcm_data);
            if (total_samples <= 0 || !pcm_data)
                error_handler::throw_on_audio_error(filename, audio_format::ogg, audio_decoder_operation::decode_audio);

            ALsizei pcm_data_size = static_cast<ALsizei>(total_samples * sizeof(short));

            return audio_data(pcm_data, pcm_data_size, static_cast<ALsizei>(channels), 16,
                            static_cast<ALsizei>(sample_rate), audio_format::ogg);
        }

        audio_data audio_data::load_from_mp3(const std::string &filename)
        {
            drmp3_config config;
            drmp3_uint64 total_frame_count;

            drmp3_int16 *pcm_data = drmp3_open_file_and_read_pcm_frames_s16(filename.c_str(), &config, &total_frame_count, nullptr);
            if (!pcm_data)
                error_handler::throw_on_audio_error(filename, audio_format::mp3, audio_decoder_operation::decode_audio);

            ALsizei pcm_data_size = static_cast<ALsizei>(total_frame_count * config.channels * sizeof(drmp3_int16));

            return audio_data(pcm_data, pcm_data_size, static_cast<ALsizei>(config.channels), 16,
                            static_cast<ALsizei>(config.sampleRate), audio_format::mp3);
        }

        ALvoid *audio_data::get_pcm_data() const { return m_pcm_data; }

        ALsizei audio_data::get_pcm_data_size() const { return m_pcm_data_size; }

        ALsizei audio_data::get_channels() const { return m_channels; }

        ALsizei audio_data::get_bits_per_sample() const { return m_bits_per_sample; }

        ALsizei audio_data::get_sample_rate() const { return m_sample_rate; }

        ALfloat audio_data::get_duration() const { return m_duration; }

        ALenum audio_data::get_openal_format() const { return m_openal_format; }

        audio_format audio_data::get_source_format() const { return m_source_format; }

        ALboolean audio_data::is_valid() const { return m_pcm_data != nullptr && m_pcm_data_size > 0; }

        bool audio_data::is_valid_wav(const std::string &filename)
        {
            drwav wav;
            bool valid = drwav_init_file(&wav, filename.c_str(), nullptr);
            if (valid)
                drwav_uninit(&wav);
            return valid;
        }

        bool audio_data::is_valid_mp3(const std::string &filename)
        {
            drmp3 mp3;
            bool valid = drmp3_init_file(&mp3, filename.c_str(), nullptr);
            if (valid)
                drmp3_uninit(&mp3);
            return valid;
        }

        bool audio_data::is_valid_ogg(const std::string &filename)
        {
            stb_vorbis* vorbis = stb_vorbis_open_filename(filename.c_str(), nullptr, nullptr);
            if (!vorbis) return false;

            stb_vorbis_info info = stb_vorbis_get_info(vorbis);
            stb_vorbis_close(vorbis);
            return info.channels > 0 && info.sample_rate > 0;
        }

        audio_format audio_data::detect_format(const std::string &filename)
        {
            if (is_valid_wav(filename)) return audio_format::wav;
            if (is_valid_mp3(filename)) return audio_format::mp3;
            if (is_valid_ogg(filename)) return audio_format::ogg;
            return audio_format::unsupported;
        }
    } // namespace detail
} // namespace soundcoe