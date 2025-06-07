#include <soundcoe/resources/sound_buffer.hpp>
#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <soundcoe/utils/audio_file.hpp>
#include <logcoe.hpp>
#include <iostream>

namespace soundcoe
{
    bool SoundBuffer::loadWavFile()
    {
        // TODO: implement
        logcoe::error("WAV loading not implemented yet");
        return false;
    }

    bool SoundBuffer::loadOggFile()
    {
        // TODO: implement
        logcoe::error("OGG loading not implemented yet");
        return false;
    }

    bool SoundBuffer::loadMp3File()
    {
        // TODO: implement
        logcoe::error("MP3 loading not implemented yet");
        return false;
    }

    SoundBuffer::SoundBuffer() : m_bufferId(0),
                                 m_filename(""),
                                 m_loaded(false),
                                 m_format(AL_NONE),
                                 m_size(0),
                                 m_samplerate(0),
                                 m_duration(0.0f) {}

    SoundBuffer::SoundBuffer(const std::string &filename) : SoundBuffer()
    {
        loadFromFile(filename);
    }

    SoundBuffer::SoundBuffer(const void *data, ALsizei size, ALenum format, ALsizei samplerate) : SoundBuffer()
    {
        loadFromMemory(data, size, format, samplerate);
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
                                                             m_samplerate(other.m_samplerate),
                                                             m_duration(other.m_duration)
    {
        other = SoundBuffer();
    }

    SoundBuffer &SoundBuffer::operator=(SoundBuffer &&other) noexcept
    {
        if (this == &other)
            return *this;

        unload();
        *this = SoundBuffer(std::move(other));
        other = SoundBuffer();
        return *this;
    }

    bool SoundBuffer::loadFromFile(const std::string &filename)
    {
        unload();

        m_filename = filename;
        std::size_t dotPos = m_filename.find_last_of('.');
        if (dotPos == std::string::npos)
        {
            logcoe::error("Unknown file format: " + m_filename);
            return false;
        }

        std::string extension = m_filename.substr(dotPos + 1);
        if (extension == "wav")
            return loadWavFile();
        if (extension == "ogg")
            return loadOggFile();
        if (extension == "mp3")
            return loadMp3File();

        logcoe::error("Unsupported audio format: " + extension);
        return false;
    }

    bool SoundBuffer::loadFromMemory(const void *data, ALsizei size, ALenum format, ALsizei samplerate)
    {
        unload();

        alGenBuffers(1, &m_bufferId);
        if (ErrorHandler::checkOpenALError("Generate buffer"))
            return false;

        alBufferData(m_bufferId, format, data, size, samplerate);
        if (ErrorHandler::checkOpenALError("Buffer data"))
        {
            alDeleteBuffers(1, &m_bufferId);
            m_bufferId = 0;
            return false;
        }

        m_format = format;
        m_size = size;
        m_samplerate = samplerate;

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
        default: // not supposed to get here (will cause divide by zero error)
            bytesPerSample = 0.0f;
            break;
        }
        // TODO: add assert instead of the divide by zero error
        m_duration = m_size / (bytesPerSample * m_samplerate);
        m_loaded = true;

        return true;
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

    const std::string &SoundBuffer::getFileName() const { return m_filename; }

    bool SoundBuffer::isLoaded() const { return m_loaded; }

    ALenum SoundBuffer::getFormat() const { return m_format; }

    ALsizei SoundBuffer::getSize() const { return m_size; }

    ALsizei SoundBuffer::getSampleRate() const { return m_samplerate; }

    float SoundBuffer::getDuration() const { return m_duration; }

} // namespace soundcoe