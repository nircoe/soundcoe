#include <soundcoe/resources/sound_buffer.hpp>
#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <soundcoe/core/types.hpp>
#include <logcoe.hpp>
#include <iostream>
#include <exception>
#include <cassert>
#include <filesystem>

namespace soundcoe
{
    namespace detail
    {
        void SoundBuffer::loadFromAudioData(AudioData &&audioData)
        {
            const void *data = audioData.getPcmData();
            m_format = audioData.getOpenALFormat();
            m_size = audioData.getPcmDataSize();
            m_sampleRate = audioData.getSampleRate();
            m_duration = audioData.getDuration();

            generateBuffer(data);

            m_loaded = true;
        }

        void SoundBuffer::generateBuffer(const void* data)
        {
            alGenBuffers(1, &m_bufferId);
            ErrorHandler::throwOnOpenALError("Generate buffer");

            alBufferData(m_bufferId, m_format, data, m_size, m_sampleRate);
            try { ErrorHandler::throwOnOpenALError("Buffer Data"); }
            catch(const std::runtime_error&)
            {
                alDeleteBuffers(1, &m_bufferId);
                m_bufferId = 0;
                throw;
            }
        }

        SoundBuffer::SoundBuffer() { }

        SoundBuffer::SoundBuffer(const std::string &filename) : SoundBuffer()
        {
            loadFromFile(filename);
        }

        SoundBuffer::SoundBuffer(const void *data, ALenum format, ALsizei size, ALsizei sampleRate) : SoundBuffer()
        {
            loadFromMemory(data, format, size, sampleRate);
        }

        SoundBuffer::~SoundBuffer()
        {
            unload();
        }

        SoundBuffer::SoundBuffer(SoundBuffer &&other) noexcept : m_bufferId(other.m_bufferId),
                                                                m_filename(std::move(other.m_filename)),
                                                                m_loaded(other.m_loaded),
                                                                m_format(other.m_format),
                                                                m_size(other.m_size),
                                                                m_sampleRate(other.m_sampleRate),
                                                                m_duration(other.m_duration)
        {
            other.m_bufferId = 0;
            other.m_loaded = false;
            other.m_format = AL_NONE;
            other.m_size = 0;
            other.m_sampleRate = 0;
            other.m_duration = 0.0f;
        }

        SoundBuffer &SoundBuffer::operator=(SoundBuffer &&other) noexcept
        {
            if (this == &other) return *this;

            unload();

            m_bufferId = other.m_bufferId;
            m_filename = std::move(other.m_filename);
            m_loaded = other.m_loaded;
            m_format = other.m_format;
            m_size = other.m_size;
            m_sampleRate = other.m_sampleRate;
            m_duration = other.m_duration;

            other.m_bufferId = 0;
            other.m_loaded = false;
            other.m_format = AL_NONE;
            other.m_size = 0;
            other.m_sampleRate = 0;
            other.m_duration = 0.0f;

            return *this;
        }

        void SoundBuffer::loadFromFile(const std::string &filename)
        {
            unload();

            std::filesystem::path filePath(filename);
            if(!std::filesystem::exists(filePath))
            {
                std::string message = "SoundBuffer::loadFromFile: File does not exist: \"" + filename + "\"";
                logcoe::error(message);
                throw std::runtime_error(message);
            }
            
            if(!std::filesystem::is_regular_file(filePath))
            {
                std::string message = "SoundBuffer::loadFromFile: Not a regular file: \"" + filename + "\"";
                logcoe::error(message);
                throw std::runtime_error(message);
            }

            m_filename = filename;

            AudioFormat format = AudioData::detectFormat(filename);
            switch(format)
            {
                case AudioFormat::Wav:
                    loadFromAudioData(std::move(AudioData::loadFromWav(filename)));
                    break;
                case AudioFormat::Mp3:
                    loadFromAudioData(std::move(AudioData::loadFromMp3(filename)));
                    break;
                case AudioFormat::Ogg:
                    loadFromAudioData(std::move(AudioData::loadFromOgg(filename)));
                    break;
                default:
                    std::string message = "SoundBuffer::loadFromFile: Unsupported audio format: " + filename;
                    logcoe::error(message);
                    throw std::runtime_error(message);
            }
            logcoe::info("SoundBuffer::loadFromFile: SoundBuffer loaded successfully");
        }

        void SoundBuffer::loadFromMemory(const void *data, ALenum format, ALsizei size, ALsizei sampleRate)
        {
            unload();

            m_format = format;
            m_size = size;
            m_sampleRate = sampleRate;

            float bytesPerSample;
            switch (m_format)
            {
            case AL_FORMAT_MONO8:
                bytesPerSample = 1.0f;
                break;
            case AL_FORMAT_MONO16:
                bytesPerSample = 2.0f;
                break;
            case AL_FORMAT_STEREO8:
                bytesPerSample = 2.0f;
                break;
            case AL_FORMAT_STEREO16:
                bytesPerSample = 4.0f;
                break;
            default:
                bytesPerSample = 0.0f;
                break;
            }
            assert(bytesPerSample != 0.0f);
            m_duration = m_size / (bytesPerSample * m_sampleRate);

            generateBuffer(data);

            m_loaded = true;
            logcoe::info("SoundBuffer::loadFromMemory: SoundBuffer loaded successfully");
        }

        void SoundBuffer::unload()
        {
            if (!m_loaded || !m_bufferId)
                return;

            alDeleteBuffers(1, &m_bufferId);
            m_bufferId = 0;
            m_loaded = false;
        }

        ALuint SoundBuffer::getBufferId() const { return m_bufferId; }

        ALenum SoundBuffer::getFormat() const { return m_format; }

        ALsizei SoundBuffer::getSize() const { return m_size; }

        ALsizei SoundBuffer::getSampleRate() const { return m_sampleRate; }

        ALfloat SoundBuffer::getDuration() const { return m_duration; }

        bool SoundBuffer::isLoaded() const { return m_loaded; }

        bool SoundBuffer::isStreaming() const { return m_stream; }

        const std::string &SoundBuffer::getFileName() const { return m_filename; }
    } // namespace detail
} // namespace soundcoe