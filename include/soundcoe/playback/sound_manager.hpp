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

        mutable std::mutex m_mutex;
        std::unordered_map<SoundHandle, ActiveAudio> m_activeSounds;
        std::unordered_map<MusicHandle, ActiveAudio> m_activeMusic;

        float m_masterVolume        = 1.0f;
        float m_masterSoundVolume   = 1.0f;
        float m_masterMusicVolume   = 1.0f;
        float m_masterPitch         = 1.0f;
        float m_masterSoundPitch    = 1.0f;
        float m_masterMusicPitch    = 1.0f;

        bool m_mute         = false;
        bool m_soundsMute   = false;
        bool m_musicMute    = false;

        Vec3 m_listenerPosition;
        Vec3 m_listenerVelocity;
        Vec3 m_listenerForward;
        Vec3 m_listenerUp;

        std::chrono::steady_clock::time_point m_lastUpdate;
        bool m_firstUpdate = true;

        std::string m_lastError = "";
        bool m_hasError         = false;

    public:
        SoundManager();
        SoundManager(const std::string &audioRootDirectory);
        ~SoundManager();

        void initialize(const std::string &audioRootDirectory);
        void shutdown();
        bool isInitialized() const;

        bool preloadScene(const std::string &sceneName);
        bool unloadScene(const std::string &sceneName);
        bool isSceneLoaded(const std::string &sceneName);

        void update();

        SoundHandle playSound(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                              SoundPriority priority = SoundPriority::Medium);
        SoundHandle playSound3D(const std::string &filename, const Vec3 &position, const Vec3 &velocity = Vec3::zero(),
                                float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                SoundPriority priority = SoundPriority::Medium);
        MusicHandle playMusic(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                              SoundPriority priority = SoundPriority::Critical);
        MusicHandle streamMusic(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = true,
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

        bool isSoundPlaying(SoundHandle handle) const;
        bool isMusicPlaying(MusicHandle handle) const;
        bool isSoundPaused(SoundHandle handle) const;
        bool isMusicPaused(MusicHandle handle) const;
        bool isSoundStopped(SoundHandle handle) const;
        bool isMusicStopped(MusicHandle handle) const;

        size_t getActiveSoundCount() const;
        size_t getActiveMusicCount() const;

        bool fadeInSound(SoundHandle handle, float duration);
        bool fadeInMusic(MusicHandle handle, float duration);
        bool fadeOutSound(SoundHandle handle, float duration);
        bool fadeOutMusic(MusicHandle handle, float duration);
        bool fadeToVolumeSound(SoundHandle handle, float targetVolume, float duration);
        bool fadeToVolumeMusic(MusicHandle handle, float targetVolume, float duration);

        bool setMasterVolume(float volume);
        bool setMasterSoundVolume(float volume);
        bool setMasterMusicVolume(float volume);
        bool muteAllSounds();
        bool muteAllMusic();
        bool muteAll();

        bool isMuted() const;
        bool isSoundsMuted() const;
        bool isMusicMuted() const;

        bool setMasterPitch(float pitch);
        bool setMasterSoundPitch(float pitch);
        bool setMasterMusicPitch(float pitch);

        float getMasterVolume() const;
        float getMasterSoundVolume() const;
        float getMasterMusicVolume() const;
        float getMasterPitch() const;
        float getMasterSoundPitch() const;
        float getMasterMusicPitch() const;

        bool updateListener(const Vec3 &position, const Vec3 &velocity, const Vec3 &forward, const Vec3 &up = Vec3::up());
        bool setListenerPosition(const Vec3 &position);
        bool setListenerVelocity(const Vec3 &velocity);
        bool setListenerForward(const Vec3 &forward);
        bool setListenerUp(const Vec3 &up = Vec3::up());

        const Vec3 getListenerPosition() const;
        const Vec3 getListenerVelocity() const;
        const Vec3 getListenerForward() const;
        const Vec3 getListenerUp() const;
        Vec3 getListenerPosition();
        Vec3 getListenerVelocity();
        Vec3 getListenerForward();
        Vec3 getListenerUp();

        bool isHandleValid(SoundHandle handle) const;
        bool isHandleValid(MusicHandle handle) const;

        const std::string getError();
        void clearError();

        // more function to come?
    };
} // namespace soundcoe