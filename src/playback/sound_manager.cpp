#include <soundcoe/playback/sound_manager.hpp>

namespace soundcoe
{
    SoundManager::SoundManager() { }

    SoundManager::SoundManager(const std::string &audioRootDirectory) { }

    SoundManager::~SoundManager() { }

    void SoundManager::initialize(const std::string &audioRootDirectory) { }

    void SoundManager::shutdown() { }

    bool SoundManager::isInitialized() const { return false; }

    bool SoundManager::preloadScene(const std::string &sceneName) { return false; }

    bool SoundManager::unloadScene(const std::string &sceneName) { return false; }

    bool SoundManager::isSceneLoaded(const std::string &sceneName) { return false; }

    void SoundManager::update() { }

    SoundHandle SoundManager::playSound(const std::string &filename, float volume, float pitch, bool loop, SoundPriority priority) { return INVALID_SOUND_HANDLE; }

    SoundHandle SoundManager::playSound3D(const std::string &filename, const Vec3 &position, const Vec3 &velocity, float volume, float pitch, bool loop, SoundPriority priority) { return INVALID_SOUND_HANDLE; }

    MusicHandle SoundManager::playMusic(const std::string &filename, float volume, float pitch, bool loop, SoundPriority priority) { return INVALID_MUSIC_HANDLE; }

    MusicHandle SoundManager::streamMusic(const std::string &filename, float volume, float pitch, bool loop, SoundPriority priority) { return INVALID_MUSIC_HANDLE; }

    bool SoundManager::pauseSound(SoundHandle handle) { return false; }

    bool SoundManager::pauseMusic(MusicHandle handle) { return false; }

    bool SoundManager::pauseAllSounds() { return false; }

    bool SoundManager::pauseAllMusic() { return false; }

    bool SoundManager::pauseAll() { return false; }

    bool SoundManager::resumeSound(SoundHandle handle) { return false; }

    bool SoundManager::resumeMusic(MusicHandle handle) { return false; }

    bool SoundManager::resumeAllSounds() { return false; }

    bool SoundManager::resumeAllMusic() { return false; }

    bool SoundManager::resumeAll() { return false; }

    bool SoundManager::stopSound(SoundHandle handle) { return false; }

    bool SoundManager::stopMusic(MusicHandle handle) { return false; }

    bool SoundManager::stopAllSounds() { return false; }

    bool SoundManager::stopAllMusic() { return false; }

    bool SoundManager::stopAll() { return false; }

    bool SoundManager::setSoundVolume(SoundHandle handle, float volume) { return false; }

    bool SoundManager::setMusicVolume(MusicHandle handle, float volume) { return false; }

    bool SoundManager::setSoundPitch(SoundHandle handle, float pitch) { return false; }

    bool SoundManager::setMusicPitch(MusicHandle handle, float pitch) { return false; }

    bool SoundManager::setSoundPosition(SoundHandle handle, const Vec3 &position) { return false; }

    bool SoundManager::setSoundVelocity(SoundHandle handle, const Vec3 &velocity) { return false; }

    bool SoundManager::isSoundPlaying(SoundHandle handle) const { return false; }

    bool SoundManager::isMusicPlaying(MusicHandle handle) const { return false; }

    bool SoundManager::isSoundPaused(SoundHandle handle) const { return false; }

    bool SoundManager::isMusicPaused(MusicHandle handle) const { return false; }

    bool SoundManager::isSoundStopped(SoundHandle handle) const { return false; }

    bool SoundManager::isMusicStopped(MusicHandle handle) const { return false; }

    size_t SoundManager::getActiveSoundCount() const { return 0; }

    size_t SoundManager::getActiveMusicCount() const { return 0; }

    bool SoundManager::fadeInSound(SoundHandle handle, float duration) { return false; }

    bool SoundManager::fadeInMusic(MusicHandle handle, float duration) { return false; }

    bool SoundManager::fadeOutSound(SoundHandle handle, float duration) { return false; }

    bool SoundManager::fadeOutMusic(MusicHandle handle, float duration) { return false; }

    bool SoundManager::fadeToVolumeSound(SoundHandle handle, float targetVolume, float duration) { return false; }

    bool SoundManager::fadeToVolumeMusic(MusicHandle handle, float targetVolume, float duration) { return false; }

    bool SoundManager::setMasterVolume(float volume) { return false; }

    bool SoundManager::setMasterSoundVolume(float volume) { return false; }

    bool SoundManager::setMasterMusicVolume(float volume) { return false; }

    bool SoundManager::muteAllSounds() { return false; }

    bool SoundManager::muteAllMusic() { return false; }

    bool SoundManager::muteAll() { return false; }

    bool SoundManager::isMuted() const { return false; }

    bool SoundManager::isSoundsMuted() const { return false; }

    bool SoundManager::isMusicMuted() const { return false; }

    bool SoundManager::setMasterPitch(float pitch) { return false; }

    bool SoundManager::setMasterSoundPitch(float pitch) { return false; }

    bool SoundManager::setMasterMusicPitch(float pitch) { return false; }

    float SoundManager::getMasterVolume() const { return 0.0f; }

    float SoundManager::getMasterSoundVolume() const { return 0.0f; }

    float SoundManager::getMasterMusicVolume() const { return 0.0f; }

    float SoundManager::getMasterPitch() const { return 0.0f; }

    float SoundManager::getMasterSoundPitch() const { return 0.0f; }

    float SoundManager::getMasterMusicPitch() const { return 0.0f; }

    bool SoundManager::updateListener(const Vec3 &position, const Vec3 &velocity, const Vec3 &forward, const Vec3 &up) { return false; }

    bool SoundManager::setListenerPosition(const Vec3 &position) { return false; }

    bool SoundManager::setListenerVelocity(const Vec3 &velocity) { return false; }

    bool SoundManager::setListenerForward(const Vec3 &forward) { return false; }

    bool SoundManager::setListenerUp(const Vec3 &up) { return false; }

    const Vec3 SoundManager::getListenerPosition() const { return Vec3::zero(); }

    const Vec3 SoundManager::getListenerVelocity() const { return Vec3::zero(); }

    const Vec3 SoundManager::getListenerForward() const { return Vec3::zero(); }

    const Vec3 SoundManager::getListenerUp() const { return Vec3::zero(); }

    Vec3 SoundManager::getListenerPosition() { return Vec3::zero(); }

    Vec3 SoundManager::getListenerVelocity() { return Vec3::zero(); }

    Vec3 SoundManager::getListenerForward() { return Vec3::zero(); }

    Vec3 SoundManager::getListenerUp() { return Vec3::zero(); }

    bool SoundManager::isHandleValid(SoundHandle handle) const { return false; }

    bool SoundManager::isHandleValid(MusicHandle handle) const { return false; }

    const std::string SoundManager::getError() { return ""; }

    void SoundManager::clearError() { }

    
} // namespace soundcoe