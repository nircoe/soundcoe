#include <soundcoe/resources/audio_data.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <logcoe.hpp>
#include <exception>

#define DR_WAV_IMPLEMENTATION
#include <dr_libs/dr_wav.h>
#define DR_MP3_IMPLEMENTATION
#include <dr_libs/dr_mp3.h>
#include <stb/stb_vorbis.c>

namespace soundcoe
{
    namespace detail
    {
        AudioData::AudioData() : m_pcmData(nullptr), m_pcmDataSize(0), m_channels(0), m_bitsPerSample(0), m_sampleRate(0), 
                                    m_duration(0.0f), m_openALFormat(AL_NONE), m_sourceFormat(AudioFormat::Unsupported) { }

        AudioData::AudioData(ALvoid *pcmData, ALsizei pcmDataSize, ALsizei channels, ALsizei bitsPerSample,
                             ALsizei sampleRate, AudioFormat sourceFormat) : m_pcmData(pcmData), m_pcmDataSize(pcmDataSize), m_channels(channels), m_bitsPerSample(bitsPerSample),
                                                                             m_sampleRate(sampleRate), m_sourceFormat(sourceFormat)
        {
            m_openALFormat = calculateOpenALFormat(m_channels, m_bitsPerSample);
            ALsizei bytesPerSample = (m_bitsPerSample / 8) * m_channels;
            m_duration = static_cast<ALfloat>(m_pcmDataSize) / (bytesPerSample * m_sampleRate);
        }

        void AudioData::cleanup()
        {
            if (!m_pcmData)
                return;

            switch (m_sourceFormat)
            {
            case AudioFormat::Wav:
                drwav_free(m_pcmData, nullptr);
                break;
            case AudioFormat::Mp3:
                drmp3_free(m_pcmData, nullptr);
                break;
            case AudioFormat::Ogg:
            default:
                free(m_pcmData);
                break;
            }
        }

        ALenum AudioData::calculateOpenALFormat(ALsizei channels, ALsizei bitsPerSample)
        {
            if (channels == 1)
            {
                if (bitsPerSample == 8)
                    return AL_FORMAT_MONO8;
                if (bitsPerSample == 16)
                    return AL_FORMAT_MONO16;
            }
            else if (channels == 2)
            {
                if (bitsPerSample == 8)
                    return AL_FORMAT_STEREO8;
                if (bitsPerSample == 16)
                    return AL_FORMAT_STEREO16;
            }

            return AL_NONE;
        }

        AudioData::AudioData(AudioData &&other) noexcept : m_pcmData(other.m_pcmData), m_pcmDataSize(other.m_pcmDataSize), m_channels(other.m_channels),
                                                        m_bitsPerSample(other.m_bitsPerSample), m_sampleRate(other.m_sampleRate), m_duration(other.m_duration),
                                                        m_openALFormat(other.m_openALFormat), m_sourceFormat(other.m_sourceFormat)
        {
            other.m_pcmData = nullptr;
        }

        AudioData &AudioData::operator=(AudioData &&other) noexcept
        {
            if (this == &other)
                return *this;

            cleanup();

            m_pcmData = other.m_pcmData;
            m_pcmDataSize = other.m_pcmDataSize;
            m_channels = other.m_channels;
            m_bitsPerSample = other.m_bitsPerSample;
            m_sampleRate = other.m_sampleRate;
            m_duration = other.m_duration;
            m_openALFormat = other.m_openALFormat;
            m_sourceFormat = other.m_sourceFormat;

            other.m_pcmData = nullptr;
            return *this;
        }

        AudioData::~AudioData() { cleanup(); }

        AudioData AudioData::loadFromWav(const std::string &filename)
        {
            unsigned int channels, sampleRate, bitsPerSample;
            drwav_uint64 totalFrameCount;
            void *pcmData;
            drwav wav;
            if (!drwav_init_file(&wav, filename.c_str(), nullptr))
                ErrorHandler::throwOnAudioError(filename, AudioFormat::Wav, AudioDecoderOperation::OpenFile);

            bitsPerSample = wav.bitsPerSample;
            drwav_uninit(&wav);

            pcmData = (bitsPerSample <= 16) ? (void *)drwav_open_file_and_read_pcm_frames_s16(filename.c_str(), &channels, &sampleRate,
                                                                                            &totalFrameCount, nullptr)
                                            : (void *)drwav_open_file_and_read_pcm_frames_s32(filename.c_str(), &channels, &sampleRate,
                                                                                            &totalFrameCount, nullptr);

            if (!pcmData)
                ErrorHandler::throwOnAudioError(filename, AudioFormat::Wav, AudioDecoderOperation::DecodeAudio);

            ALsizei bytesPerSample = (bitsPerSample <= 16) ? sizeof(drwav_int16) : sizeof(drwav_int32);
            ALsizei pcmDataSize = static_cast<ALsizei>(totalFrameCount * channels * bytesPerSample);

            return AudioData(pcmData, pcmDataSize, static_cast<ALsizei>(channels), static_cast<ALsizei>(bitsPerSample),
                            static_cast<ALsizei>(sampleRate), AudioFormat::Wav);
        }

        AudioData AudioData::loadFromOgg(const std::string &filename)
        {
            int channels, sampleRate;
            short *pcmData;
            int totalSamples = stb_vorbis_decode_filename(filename.c_str(), &channels, &sampleRate, &pcmData);
            if (totalSamples <= 0 || !pcmData)
                ErrorHandler::throwOnAudioError(filename, AudioFormat::Ogg, AudioDecoderOperation::DecodeAudio);

            ALsizei pcmDataSize = static_cast<ALsizei>(totalSamples * sizeof(short));

            return AudioData(pcmData, pcmDataSize, static_cast<ALsizei>(channels), 16,
                            static_cast<ALsizei>(sampleRate), AudioFormat::Ogg);
        }

        AudioData AudioData::loadFromMp3(const std::string &filename)
        {
            drmp3_config config;
            drmp3_uint64 totalFrameCount;

            drmp3_int16 *pcmData = drmp3_open_file_and_read_pcm_frames_s16(filename.c_str(), &config, &totalFrameCount, nullptr);
            if (!pcmData)
                ErrorHandler::throwOnAudioError(filename, AudioFormat::Mp3, AudioDecoderOperation::DecodeAudio);

            ALsizei pcmDataSize = static_cast<ALsizei>(totalFrameCount * config.channels * sizeof(drmp3_int16));

            return AudioData(pcmData, pcmDataSize, static_cast<ALsizei>(config.channels), 16,
                            static_cast<ALsizei>(config.sampleRate), AudioFormat::Mp3);
        }

        ALvoid *AudioData::getPcmData() const { return m_pcmData; }

        ALsizei AudioData::getPcmDataSize() const { return m_pcmDataSize; }

        ALsizei AudioData::getChannels() const { return m_channels; }

        ALsizei AudioData::getBitsPerSample() const { return m_bitsPerSample; }

        ALsizei AudioData::getSampleRate() const { return m_sampleRate; }

        ALfloat AudioData::getDuration() const { return m_duration; }

        ALenum AudioData::getOpenALFormat() const { return m_openALFormat; }

        AudioFormat AudioData::getSourceFormat() const { return m_sourceFormat; }

        ALboolean AudioData::isValid() const { return m_pcmData != nullptr && m_pcmDataSize > 0; }

        bool AudioData::isValidWav(const std::string &filename)
        {
            drwav wav;
            bool valid = drwav_init_file(&wav, filename.c_str(), nullptr);
            if (valid) 
                drwav_uninit(&wav);
            return valid;
        }

        bool AudioData::isValidMp3(const std::string &filename)
        {
            drmp3 mp3;
            bool valid = drmp3_init_file(&mp3, filename.c_str(), nullptr);
            if (valid)
                drmp3_uninit(&mp3);
            return valid;
        }

        bool AudioData::isValidOgg(const std::string &filename)
        {
            stb_vorbis* vorbis = stb_vorbis_open_filename(filename.c_str(), nullptr, nullptr);
            if (!vorbis) return false;
            
            stb_vorbis_info info = stb_vorbis_get_info(vorbis);
            stb_vorbis_close(vorbis);
            return info.channels > 0 && info.sample_rate > 0;
        }

        AudioFormat AudioData::detectFormat(const std::string &filename)
        {
            if (isValidWav(filename)) return AudioFormat::Wav;
            if (isValidMp3(filename)) return AudioFormat::Mp3;
            if (isValidOgg(filename)) return AudioFormat::Ogg;
            return AudioFormat::Unsupported;
        }
    } // namespace detail
} // namespace soundcoe