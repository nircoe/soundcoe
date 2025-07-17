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

    namespace detail
    {
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
        bool m_initialized = false;

        ResourceManager m_resourceManager;
        std::string m_soundSubdir;
        std::string m_musicSubdir;

        std::atomic<size_t> m_nextSoundHandle;
        std::atomic<size_t> m_nextMusicHandle;

        mutable std::mutex m_mutex;
        std::unordered_map<SoundHandle, ActiveAudio> m_activeSounds;
        std::unordered_map<MusicHandle, ActiveAudio> m_activeMusic;

        float m_masterVolume = 1.0f;
        float m_masterSoundsVolume = 1.0f;
        float m_masterMusicVolume = 1.0f;
        float m_masterPitch = 1.0f;
        float m_masterSoundsPitch = 1.0f;
        float m_masterMusicPitch = 1.0f;

        bool m_mute = false;
        bool m_soundsMute = false;
        bool m_musicMute = false;

        Vec3 m_listenerPosition;
        Vec3 m_listenerVelocity;
        Vec3 m_listenerForward;
        Vec3 m_listenerUp;

        std::chrono::steady_clock::time_point m_lastUpdate;
        bool m_firstUpdate = true;

        std::string m_lastError = "";
        bool m_hasError = false;

        template <typename Setter, typename Getter>
        void updateAllAudioProperty(std::unordered_map<size_t, ActiveAudio> &activeAudio,
                                    Setter setProperty, Getter getBaseProperty,
                                    float masterMultiplier, float categoryMultiplier)
        {
            for (auto it = activeAudio.begin(); it != activeAudio.end();)
            {
                auto sourceAllocation = m_resourceManager.getSourceAllocation(it->second.m_sourceIndex);
                if (!(sourceAllocation.has_value()) || !(sourceAllocation.value().get().m_active))
                {
                    it = activeAudio.erase(it);
                    continue;
                }

                float finalValue = getBaseProperty(it->second) * masterMultiplier * categoryMultiplier;
                setProperty(sourceAllocation.value().get().m_source, finalValue);
                ++it;
            }
        }

        void updateAllSoundsVolume();
        void updateAllMusicVolume();
        void updateAllVolume();
        void updateAllSoundsPitch();
        void updateAllMusicPitch();
        void updateAllPitch();

        bool setListenerPositionImpl(const Vec3 &position);
        bool setListenerVelocityImpl(const Vec3 &velocity);
        bool setListenerOrientationImpl(const Vec3 &forward, const Vec3 &up);

        bool setError(const std::string &error);

        bool fadeToVolume(std::unordered_map<size_t, ActiveAudio> &activeAudio, size_t handle,
                          float targetVolume, float duration, const std::string &method);
        bool fade(std::unordered_map<size_t, ActiveAudio> &activeAudio, size_t handle,
                  bool fadeIn, float duration, const std::string &method);

        bool checkAudioState(std::unordered_map<size_t, ActiveAudio> &activeAudio, size_t handle,
                             SoundState state, const std::string &method);

        bool setAudioProperty(std::unordered_map<size_t, ActiveAudio> &activeAudio, size_t handle,
                              PropertyType type, const std::string &method,
                              float value, float y = 0.0f, float z = 0.0f);

        bool audioOperation(std::unordered_map<size_t, ActiveAudio> &activeAudio, size_t handle,
                            SoundState operation, const std::string &method);
        bool audioOperationAll(std::unordered_map<size_t, ActiveAudio> &activeAudio, SoundState operation,
                               const std::string &method);

        size_t play(std::unordered_map<size_t, ActiveAudio> &activeAudio, const std::string &filename,
                    float volume, float pitch, bool loop, SoundPriority priority,
                    std::atomic<size_t> &nextHandle, const std::string &method,
                    float masterCategoryVolume, float masterCategoryPitch,
                    bool is3D = false, const Vec3 &position = Vec3::zero(), const Vec3 &velocity = Vec3::zero());

        void handleStreamingAudio();
        void handleFadeEffects(std::unordered_map<size_t, ActiveAudio> &activeAudio, 
                               float categoryMultiplier, float deltaTime);
        void handleInactiveAudio(std::unordered_map<size_t, ActiveAudio> &activeAudio);

    public:
        SoundManager();
        ~SoundManager();

        bool initialize(const std::string &audioRootDirectory, size_t maxSources = 32,
                        size_t maxCacheSizeMB = 64, const std::string &soundSubdir = "sfx",
                        const std::string &musicSubdir = "music", LogLevel level = LogLevel::INFO);
        void shutdown();
        bool isInitialized() const;

        bool preloadScene(const std::string &sceneName);
        bool unloadScene(const std::string &sceneName);
        bool isSceneLoaded(const std::string &sceneName) const;

        void update();

        SoundHandle playSound(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                              SoundPriority priority = SoundPriority::Medium);
        SoundHandle playSound3D(const std::string &filename, const Vec3 &position, const Vec3 &velocity = Vec3::zero(),
                                float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                SoundPriority priority = SoundPriority::Medium);
        MusicHandle playMusic(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                              SoundPriority priority = SoundPriority::Critical);

        bool pauseSound(SoundHandle handle);
        bool pauseMusic(MusicHandle handle);
        bool pauseAllSounds();
        bool pauseAllMusic();
        bool pauseAll();
        bool resumeSound(SoundHandle handle);
        bool resumeMusic(MusicHandle handle);
        bool resumeAllSounds();
        bool resumeAllMusic();
        bool resumeAll();
        bool stopSound(SoundHandle handle);
        bool stopMusic(MusicHandle handle);
        bool stopAllSounds();
        bool stopAllMusic();
        bool stopAll();
        bool setSoundVolume(SoundHandle handle, float volume);
        bool setMusicVolume(MusicHandle handle, float volume);
        bool setSoundPitch(SoundHandle handle, float pitch);
        bool setMusicPitch(MusicHandle handle, float pitch);
        bool setSoundPosition(SoundHandle handle, const Vec3 &position);
        bool setSoundVelocity(SoundHandle handle, const Vec3 &velocity);

        bool isSoundPlaying(SoundHandle handle);
        bool isMusicPlaying(MusicHandle handle);
        bool isSoundPaused(SoundHandle handle);
        bool isMusicPaused(MusicHandle handle);
        bool isSoundStopped(SoundHandle handle);
        bool isMusicStopped(MusicHandle handle);

        size_t getActiveSoundsCount() const;
        size_t getActiveMusicCount() const;

        SoundHandle fadeInSound(const std::string &filename, float duration,
                                float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                SoundPriority priority = SoundPriority::Medium);
        MusicHandle fadeInMusic(const std::string &filename, float duration,
                                float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                SoundPriority priority = SoundPriority::Critical);
        bool fadeOutSound(SoundHandle handle, float duration);
        bool fadeOutMusic(MusicHandle handle, float duration);
        bool fadeToVolumeSound(SoundHandle handle, float targetVolume, float duration);
        bool fadeToVolumeMusic(MusicHandle handle, float targetVolume, float duration);

        bool setMasterVolume(float volume);
        bool setMasterSoundsVolume(float volume);
        bool setMasterMusicVolume(float volume);
        bool setMasterPitch(float pitch);
        bool setMasterSoundsPitch(float pitch);
        bool setMasterMusicPitch(float pitch);
        float getMasterVolume() const;
        float getMasterSoundsVolume() const;
        float getMasterMusicVolume() const;
        float getMasterPitch() const;
        float getMasterSoundsPitch() const;
        float getMasterMusicPitch() const;

        bool muteAllSounds();
        bool muteAllMusic();
        bool muteAll();
        bool unmuteAllSounds();
        bool unmuteAllMusic();
        bool unmuteAll();
        bool isMuted() const;
        bool isSoundsMuted() const;
        bool isMusicMuted() const;

        bool updateListener(const Vec3 &position, const Vec3 &velocity, const Vec3 &forward, const Vec3 &up = Vec3::up());
        bool setListenerPosition(const Vec3 &position);
        bool setListenerVelocity(const Vec3 &velocity);
        bool setListenerForward(const Vec3 &forward);
        bool setListenerUp(const Vec3 &up = Vec3::up());

        Vec3 getListenerPosition();
        Vec3 getListenerVelocity();
        Vec3 getListenerForward();
        Vec3 getListenerUp();

        const std::string getError();
        void clearError();

        static bool isHandleValid(size_t handle);
    };
    } // namespace detail
} // namespace soundcoe