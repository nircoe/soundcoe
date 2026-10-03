#include <soundcoe/resources/sound_buffer.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <soundcoe/core/types.hpp>
#include <filesystem>
#include <utility>
#include <soundcoe_config.hpp>
#if SOUNDCOE_USE_LOGCOE
#include <logcoe.hpp>
#endif

namespace soundcoe
{
    namespace internal
    {
        std::expected<void, error> sound_buffer::load_from_audio_data(audio_data &&audio_data_)
        {
            const void *data = audio_data_.get_pcm_data();
            m_format = audio_data_.get_openal_format();
            m_size = audio_data_.get_pcm_data_size();
            m_sample_rate = audio_data_.get_sample_rate();
            m_duration = audio_data_.get_duration();

            if (auto r = generate_buffer(data); !r)
                return r;

            m_loaded = true;
            return {};
        }

        std::expected<void, error> sound_buffer::generate_buffer(const void* data)
        {
            alGenBuffers(1, &m_buffer_id);
            if (auto r = error_handler::check_openal_error("Generate buffer"); !r)
                return r;

            alBufferData(m_buffer_id, m_format, data, m_size, m_sample_rate);
            if (auto r = error_handler::check_openal_error("Buffer Data"); !r)
            {
                alDeleteBuffers(1, &m_buffer_id);
                m_buffer_id = 0;
                return r;
            }

            return {};
        }

        sound_buffer::sound_buffer() { }

        sound_buffer::~sound_buffer()
        {
            unload();
        }

        sound_buffer::sound_buffer(sound_buffer &&other) noexcept : m_buffer_id(other.m_buffer_id),
                                                                m_format(other.m_format),
                                                                m_size(other.m_size),
                                                                m_sample_rate(other.m_sample_rate),
                                                                m_duration(other.m_duration),
                                                                m_loaded(other.m_loaded),
                                                                m_filename(std::move(other.m_filename))
        {
            other.m_buffer_id = 0;
            other.m_loaded = false;
            other.m_format = AL_NONE;
            other.m_size = 0;
            other.m_sample_rate = 0;
            other.m_duration = 0.0f;
        }

        sound_buffer &sound_buffer::operator=(sound_buffer &&other) noexcept
        {
            if (this == &other) return *this;

            unload();

            m_buffer_id = other.m_buffer_id;
            m_filename = std::move(other.m_filename);
            m_loaded = other.m_loaded;
            m_format = other.m_format;
            m_size = other.m_size;
            m_sample_rate = other.m_sample_rate;
            m_duration = other.m_duration;

            other.m_buffer_id = 0;
            other.m_loaded = false;
            other.m_format = AL_NONE;
            other.m_size = 0;
            other.m_sample_rate = 0;
            other.m_duration = 0.0f;

            return *this;
        }

        std::expected<void, error> sound_buffer::load_from_file(const std::string &filename)
        {
            unload();

            std::filesystem::path file_path(filename);
            if(!std::filesystem::exists(file_path))
                return std::unexpected(error_handler::make_error(error_code::file_not_found,
                    "sound_buffer::load_from_file: File does not exist: \"" + filename + "\""));

            if(!std::filesystem::is_regular_file(file_path))
                return std::unexpected(error_handler::make_error(error_code::invalid_argument,
                    "sound_buffer::load_from_file: Not a regular file: \"" + filename + "\""));

            m_filename = filename;

            auto data = [&]() -> std::expected<audio_data, error>
            {
                switch (audio_data::detect_format(filename))
                {
                    case audio_format::wav: return audio_data::load_from_wav(filename);
                    case audio_format::mp3: return audio_data::load_from_mp3(filename);
                    case audio_format::ogg: return audio_data::load_from_ogg(filename);
                    case audio_format::unsupported:
                        return std::unexpected(error_handler::make_error(error_code::unsupported_format,
                            "sound_buffer::load_from_file: Unsupported audio format: " + filename));
                }
                std::unreachable();
            }();
            if (!data)
                return std::unexpected(data.error());
            if (auto r = load_from_audio_data(std::move(*data)); !r)
                return r;

            logcoe::info("sound_buffer::load_from_file: sound_buffer loaded successfully");
            return {};
        }

        std::expected<void, error> sound_buffer::load_from_memory(const void *data, ALenum format, ALsizei size,
                                                                  ALsizei sample_rate)
        {
            unload();

            m_format = format;
            m_size = size;
            m_sample_rate = sample_rate;

            float bytes_per_sample;
            switch (m_format)
            {
            case AL_FORMAT_MONO8:
                bytes_per_sample = 1.0f;
                break;
            case AL_FORMAT_MONO16:
                bytes_per_sample = 2.0f;
                break;
            case AL_FORMAT_STEREO8:
                bytes_per_sample = 2.0f;
                break;
            case AL_FORMAT_STEREO16:
                bytes_per_sample = 4.0f;
                break;
            default:
                bytes_per_sample = 0.0f;
                break;
            }
            if (bytes_per_sample == 0.0f)
                return std::unexpected(error_handler::make_error(error_code::invalid_argument,
                    "sound_buffer::load_from_memory: Unsupported ALenum format"));
            m_duration = m_size / (bytes_per_sample * m_sample_rate);

            if (auto r = generate_buffer(data); !r)
                return r;

            m_loaded = true;
            logcoe::info("sound_buffer::load_from_memory: sound_buffer loaded successfully");
            return {};
        }

        void sound_buffer::unload()
        {
            if (!m_loaded || !m_buffer_id)
                return;

            alDeleteBuffers(1, &m_buffer_id);
            m_buffer_id = 0;
            m_loaded = false;
        }

        ALuint sound_buffer::get_buffer_id() const { return m_buffer_id; }

        ALenum sound_buffer::get_format() const { return m_format; }

        ALsizei sound_buffer::get_size() const { return m_size; }

        ALsizei sound_buffer::get_sample_rate() const { return m_sample_rate; }

        ALfloat sound_buffer::get_duration() const { return m_duration; }

        bool sound_buffer::is_loaded() const { return m_loaded; }

        bool sound_buffer::is_streaming() const { return m_stream; }

        const std::string &sound_buffer::get_filename() const { return m_filename; }
    } // namespace internal
} // namespace soundcoe
