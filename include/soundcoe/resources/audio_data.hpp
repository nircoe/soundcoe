#pragma once

#include <soundcoe/core/types.hpp>
#include <AL/al.h>
#include <string>

namespace soundcoe
{
    class AudioData
    {
        ALvoid *m_pcmData;
        ALsizei m_pcmDataSize;

        ALsizei m_channels;
        ALsizei m_bitsPerSample;
        ALsizei m_sampleRate;
        ALfloat m_duration;

        ALenum m_openALFormat;
        AudioFormat m_sourceFormat;

        AudioData(ALvoid *pcmData, ALsizei pcmDataSize, ALsizei channels, ALsizei bitsPerSample, ALsizei sampleRate, 
            AudioFormat sourceFormat);

        void cleanup();

        ALenum calculateOpenALFormat(ALsizei channels, ALsizei bitsPerSample);

        static bool isValidWav(const std::string &filename);
        static bool isValidMp3(const std::string &filename);
        static bool isValidOgg(const std::string &filename);

    public:
        AudioData();
        AudioData(const AudioData &) = delete;
        AudioData &operator=(const AudioData &) = delete;
        AudioData(AudioData &&other) noexcept;
        AudioData &operator=(AudioData &&other) noexcept;
        ~AudioData();

        static AudioData loadFromWav(const std::string &filename);
        static AudioData loadFromOgg(const std::string &filename);
        static AudioData loadFromMp3(const std::string &filename);
        static AudioFormat detectFormat(const std::string &filename);

        ALvoid *getPcmData() const;
        ALsizei getPcmDataSize() const;
        ALsizei getChannels() const;
        ALsizei getBitsPerSample() const;
        ALsizei getSampleRate() const;
        ALfloat getDuration() const;
        ALenum getOpenALFormat() const;
        AudioFormat getSourceFormat() const;

        ALboolean isValid() const;
    };

} // namespace soundcoe