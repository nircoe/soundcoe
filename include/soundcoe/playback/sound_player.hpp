#pragma once

#include <soundcoe/resources/resource_manager.hpp>
#include <soundcoe/core/types.hpp>
#include <string>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <chrono>

namespace soundcoe
{
    using SoundHandle = size_t;
    using MusicHandle = size_t;
    constexpr SoundHandle INVALID_SOUND_HANDLE = 0;
    constexpr MusicHandle INVALID_MUSIC_HANDLE = 0;

    struct ActiveAudio
    {
        size_t m_sourceIndex;
        std::string m_filename;
        float m_baseVolume;
        float m_basePitch;
        bool m_loop;

        bool m_stream = false;
        size_t m_streamBufferSize = 0;
        float m_streamPosition = 0.0f;
        bool m_streamNeedsRefill = false;

        bool m_isFading = false;
        float m_fadeDuration = 0.0f;
        float m_fadeElapsed = 0.0f;
        float m_fadeStartVolume = 0.0f;
        float m_fadeTargetVolume = 0.0f;
    };

    class SoundManager
    {
        ResourceManager m_resourceManager;

        std::atomic<size_t> m_nextSoundHandle;
        std::atomic<size_t> m_nextMusicHandle;

        std::unordered_map<SoundHandle, ActiveAudio> m_activeSounds;
        std::unordered_map<MusicHandle, ActiveAudio> m_activeMusics;
        mutable std::mutex m_mutex;

        float m_masterVolume;
        float m_effectsVolume;
        float m_musicVolume;

        Vec3 m_listenerPosition;
        Vec3 m_listenerVelocity;
        Vec3 m_listenerForward;
        Vec3 m_listenerUp;

        std::chrono::steady_clock::time_point m_lastUpdate;
        bool m_firstUpdate;

        std::string m_lastError;
        bool m_hasError;

    public:
        SoundManager();
        SoundManager(const std::string &audioRootDirectory);
        ~SoundManager();

        void initialize(const std::string &audioRootDirectory);
        void shutdown();

        void update();

        SoundHandle playSound(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                              SoundPriority priority = SoundPriority::Medium);
        SoundHandle playSound3D(const std::string &filename, const Vec3 &position, float volume = 1.0f,
                                float pitch = 1.0f, bool loop = false, SoundPriority priority = SoundPriority::Medium);

        // more function to come
    };
} // namespace soundcoe