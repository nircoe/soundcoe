#include <gtest/gtest.h>
#include <soundcoe/playback/sound_manager.hpp>
#include <soundcoe/core/types.hpp>
#include "utils/test_audio_files.hpp"
#include <thread>
#include <chrono>
#include <vector>
#include <future>

using namespace soundcoe;
using namespace soundcoe::detail;

//==============================================================================
//               SoundManagerTests - SoundManager comprehensive tests
//==============================================================================

class SoundManagerTests : public ::testing::Test
{
protected:
    SoundManager m_soundManager;

    void SetUp() override
    {
        TestAudioFiles::createTestFiles();
    }

    void TearDown() override
    {
        try
        {
            m_soundManager.shutdown();
        }
        catch (...)
        {
        }
        TestAudioFiles::cleanup();
    }

    void initializeSoundManager()
    {
        ASSERT_TRUE(m_soundManager.initialize(TestAudioFiles::s_testRootDir.string(), 8, 32));
    }

    void waitForFade(float duration)
    {
        auto start = std::chrono::steady_clock::now();
        auto targetDuration = std::chrono::duration<float>(duration + 0.1f); // Small buffer

        while (std::chrono::steady_clock::now() - start < targetDuration)
        {
            m_soundManager.update();
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        }
    }
};

TEST_F(SoundManagerTests, InitializationAndShutdown)
{
    initializeSoundManager();
    EXPECT_TRUE(m_soundManager.isInitialized());

    m_soundManager.shutdown();
    EXPECT_FALSE(m_soundManager.isInitialized());

    EXPECT_TRUE(m_soundManager.initialize(TestAudioFiles::s_testRootDir.string()));
    EXPECT_TRUE(m_soundManager.isInitialized());
}

TEST_F(SoundManagerTests, InvalidInitialization)
{
    SoundManager manager;
    EXPECT_FALSE(manager.initialize(""));
    EXPECT_FALSE(manager.initialize("/nonexistent/path"));
    EXPECT_FALSE(manager.isInitialized());
}

TEST_F(SoundManagerTests, PlaySoundBasic)
{
    initializeSoundManager();

    auto handle = m_soundManager.playSound("general/sfx/beep.wav");
    EXPECT_NE(handle, INVALID_SOUND_HANDLE);
    EXPECT_TRUE(SoundManager::isHandleValid(handle));

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.isSoundPlaying(handle));
    EXPECT_EQ(m_soundManager.getActiveSoundsCount(), 1);
}

TEST_F(SoundManagerTests, PlayMusicBasic)
{
    initializeSoundManager();

    auto handle = m_soundManager.playMusic("general/sfx/click.wav");
    EXPECT_NE(handle, INVALID_MUSIC_HANDLE);
    EXPECT_TRUE(SoundManager::isHandleValid(handle));

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.isMusicPlaying(handle));
    EXPECT_EQ(m_soundManager.getActiveMusicCount(), 1);
}

TEST_F(SoundManagerTests, PlaySound3D)
{
    initializeSoundManager();

    Vec3 position(1.0f, 2.0f, 3.0f);
    Vec3 velocity(0.1f, 0.2f, 0.3f);

    auto handle = m_soundManager.playSound3D("general/sfx/beep.wav", position, velocity);
    EXPECT_NE(handle, INVALID_SOUND_HANDLE);

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.isSoundPlaying(handle));
}

TEST_F(SoundManagerTests, InvalidFileHandling)
{
    initializeSoundManager();

    auto handle = m_soundManager.playSound("nonexistent/file.wav");
    EXPECT_EQ(handle, INVALID_SOUND_HANDLE);
    EXPECT_FALSE(SoundManager::isHandleValid(handle));

    auto musicHandle = m_soundManager.playMusic("");
    EXPECT_EQ(musicHandle, INVALID_MUSIC_HANDLE);
}

TEST_F(SoundManagerTests, PlaybackControls)
{
    initializeSoundManager();

    auto handle = m_soundManager.playSound("general/sfx/beep.wav");
    ASSERT_NE(handle, INVALID_SOUND_HANDLE);

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.isSoundPlaying(handle));

    EXPECT_TRUE(m_soundManager.pauseSound(handle));
    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.isSoundPaused(handle));

    EXPECT_TRUE(m_soundManager.resumeSound(handle));
    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.isSoundPlaying(handle));

    EXPECT_TRUE(m_soundManager.stopSound(handle));
    m_soundManager.update();
    EXPECT_EQ(m_soundManager.getActiveSoundsCount(), 0);
}

TEST_F(SoundManagerTests, VolumeAndPitchControls)
{
    initializeSoundManager();

    auto handle = m_soundManager.playSound("general/sfx/beep.wav", 0.5f, 1.2f);
    ASSERT_NE(handle, INVALID_SOUND_HANDLE);

    m_soundManager.update();

    EXPECT_TRUE(m_soundManager.setSoundVolume(handle, 0.8f));
    EXPECT_TRUE(m_soundManager.setSoundPitch(handle, 0.9f));

    Vec3 newPos(5.0f, 6.0f, 7.0f);
    EXPECT_TRUE(m_soundManager.setSoundPosition(handle, newPos));

    Vec3 newVel(0.5f, 0.6f, 0.7f);
    EXPECT_TRUE(m_soundManager.setSoundVelocity(handle, newVel));
}

TEST_F(SoundManagerTests, MasterVolumeControls)
{
    initializeSoundManager();

    EXPECT_TRUE(m_soundManager.setMasterVolume(0.8f));
    EXPECT_EQ(m_soundManager.getMasterVolume(), 0.8f);

    EXPECT_TRUE(m_soundManager.setMasterSoundsVolume(0.6f));
    EXPECT_EQ(m_soundManager.getMasterSoundsVolume(), 0.6f);

    EXPECT_TRUE(m_soundManager.setMasterMusicVolume(0.4f));
    EXPECT_EQ(m_soundManager.getMasterMusicVolume(), 0.4f);
}

TEST_F(SoundManagerTests, MasterPitchControls)
{
    initializeSoundManager();

    EXPECT_TRUE(m_soundManager.setMasterPitch(1.2f));
    EXPECT_EQ(m_soundManager.getMasterPitch(), 1.2f);

    EXPECT_TRUE(m_soundManager.setMasterSoundsPitch(0.9f));
    EXPECT_EQ(m_soundManager.getMasterSoundsPitch(), 0.9f);

    EXPECT_TRUE(m_soundManager.setMasterMusicPitch(1.1f));
    EXPECT_EQ(m_soundManager.getMasterMusicPitch(), 1.1f);
}

TEST_F(SoundManagerTests, MuteControls)
{
    initializeSoundManager();

    EXPECT_FALSE(m_soundManager.isMuted());
    EXPECT_FALSE(m_soundManager.isSoundsMuted());
    EXPECT_FALSE(m_soundManager.isMusicMuted());

    EXPECT_TRUE(m_soundManager.muteAllSounds());
    EXPECT_TRUE(m_soundManager.isSoundsMuted());

    EXPECT_TRUE(m_soundManager.muteAllMusic());
    EXPECT_TRUE(m_soundManager.isMusicMuted());

    EXPECT_TRUE(m_soundManager.muteAll());
    EXPECT_TRUE(m_soundManager.isMuted());

    EXPECT_TRUE(m_soundManager.unmuteAll());
    EXPECT_FALSE(m_soundManager.isMuted());
    EXPECT_FALSE(m_soundManager.isSoundsMuted());
    EXPECT_FALSE(m_soundManager.isMusicMuted());
}

TEST_F(SoundManagerTests, ListenerControls)
{
    initializeSoundManager();

    Vec3 position(1.0f, 2.0f, 3.0f);
    Vec3 velocity(0.1f, 0.2f, 0.3f);
    Vec3 forward(0.0f, 0.0f, -1.0f);
    Vec3 up(0.0f, 1.0f, 0.0f);

    EXPECT_TRUE(m_soundManager.updateListener(position, velocity, forward, up));

    EXPECT_EQ(m_soundManager.getListenerPosition(), position);
    EXPECT_EQ(m_soundManager.getListenerVelocity(), velocity);
    EXPECT_EQ(m_soundManager.getListenerForward(), forward);
    EXPECT_EQ(m_soundManager.getListenerUp(), up);

    Vec3 newPosition(4.0f, 5.0f, 6.0f);
    EXPECT_TRUE(m_soundManager.setListenerPosition(newPosition));
    EXPECT_EQ(m_soundManager.getListenerPosition(), newPosition);
}

TEST_F(SoundManagerTests, FadeInSound)
{
    initializeSoundManager();

    auto handle = m_soundManager.fadeInSound("general/sfx/beep.wav", 0.1f, 1.0f);
    EXPECT_NE(handle, INVALID_SOUND_HANDLE);

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.isSoundPlaying(handle));

    waitForFade(0.1f);
    EXPECT_TRUE(m_soundManager.isSoundPlaying(handle));
}

TEST_F(SoundManagerTests, FadeInMusic)
{
    initializeSoundManager();

    auto handle = m_soundManager.fadeInMusic("general/sfx/click.wav", 0.1f, 0.8f);
    EXPECT_NE(handle, INVALID_MUSIC_HANDLE);

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.isMusicPlaying(handle));

    waitForFade(0.1f);
    EXPECT_TRUE(m_soundManager.isMusicPlaying(handle));
}

TEST_F(SoundManagerTests, FadeOutSound)
{
    initializeSoundManager();

    auto handle = m_soundManager.playSound("general/sfx/beep.wav");
    ASSERT_NE(handle, INVALID_SOUND_HANDLE);

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.isSoundPlaying(handle));

    EXPECT_TRUE(m_soundManager.fadeOutSound(handle, 0.1f));
    waitForFade(1.0f);

    EXPECT_EQ(m_soundManager.getActiveSoundsCount(), 0);
}

TEST_F(SoundManagerTests, FadeOutMusic)
{
    initializeSoundManager();

    auto handle = m_soundManager.playMusic("general/sfx/click.wav");
    ASSERT_NE(handle, INVALID_MUSIC_HANDLE);

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.isMusicPlaying(handle));

    EXPECT_TRUE(m_soundManager.fadeOutMusic(handle, 0.1f));
    waitForFade(1.0f);

    EXPECT_EQ(m_soundManager.getActiveMusicCount(), 0);
}

TEST_F(SoundManagerTests, FadeToVolume)
{
    initializeSoundManager();

    auto soundHandle = m_soundManager.playSound("general/sfx/beep.wav", 1.0f);
    ASSERT_NE(soundHandle, INVALID_SOUND_HANDLE);

    auto musicHandle = m_soundManager.playMusic("general/sfx/click.wav", 0.5f);
    ASSERT_NE(musicHandle, INVALID_MUSIC_HANDLE);

    m_soundManager.update();

    EXPECT_TRUE(m_soundManager.fadeToVolumeSound(soundHandle, 0.3f, 0.1f));
    EXPECT_TRUE(m_soundManager.fadeToVolumeMusic(musicHandle, 0.8f, 0.1f));

    waitForFade(0.1f);

    EXPECT_TRUE(m_soundManager.isSoundPlaying(soundHandle));
    EXPECT_TRUE(m_soundManager.isMusicPlaying(musicHandle));
}

TEST_F(SoundManagerTests, SceneManagement)
{
    initializeSoundManager();

    EXPECT_TRUE(m_soundManager.isSceneLoaded("general"));

    EXPECT_TRUE(m_soundManager.preloadScene("scene1"));
    EXPECT_TRUE(m_soundManager.isSceneLoaded("scene1"));

    auto handle = m_soundManager.playSound("scene1/sfx/explosion.wav");
    EXPECT_NE(handle, INVALID_SOUND_HANDLE);

    EXPECT_TRUE(m_soundManager.unloadScene("scene1"));
    EXPECT_FALSE(m_soundManager.isSceneLoaded("scene1"));
}

TEST_F(SoundManagerTests, InvalidSceneOperations)
{
    initializeSoundManager();

    EXPECT_FALSE(m_soundManager.preloadScene(""));
    EXPECT_FALSE(m_soundManager.preloadScene("nonexistent"));
    EXPECT_FALSE(m_soundManager.isSceneLoaded(""));
    EXPECT_FALSE(m_soundManager.isSceneLoaded("nonexistent"));
}

TEST_F(SoundManagerTests, MultipleSoundsAndMusic)
{
    initializeSoundManager();

    std::vector<SoundHandle> soundHandles;
    std::vector<MusicHandle> musicHandles;

    for (int i = 0; i < 3; ++i)
    {
        auto soundHandle = m_soundManager.playSound("general/sfx/beep.wav");
        EXPECT_NE(soundHandle, INVALID_SOUND_HANDLE);
        soundHandles.push_back(soundHandle);

        auto musicHandle = m_soundManager.playMusic("general/sfx/click.wav");
        EXPECT_NE(musicHandle, INVALID_MUSIC_HANDLE);
        musicHandles.push_back(musicHandle);
    }

    m_soundManager.update();
    EXPECT_EQ(m_soundManager.getActiveSoundsCount(), 3);
    EXPECT_EQ(m_soundManager.getActiveMusicCount(), 3);

    EXPECT_TRUE(m_soundManager.pauseAllSounds());
    EXPECT_TRUE(m_soundManager.pauseAllMusic());

    m_soundManager.update();
    for (auto handle : soundHandles)
    {
        EXPECT_TRUE(m_soundManager.isSoundPaused(handle));
    }
    for (auto handle : musicHandles)
    {
        EXPECT_TRUE(m_soundManager.isMusicPaused(handle));
    }

    EXPECT_TRUE(m_soundManager.stopAll());
    m_soundManager.update();
    EXPECT_EQ(m_soundManager.getActiveSoundsCount(), 0);
    EXPECT_EQ(m_soundManager.getActiveMusicCount(), 0);
}

TEST_F(SoundManagerTests, ErrorHandling)
{
    initializeSoundManager();

    m_soundManager.clearError();
    EXPECT_EQ(m_soundManager.getError(), "");

    EXPECT_FALSE(m_soundManager.pauseSound(INVALID_SOUND_HANDLE));
    EXPECT_NE(m_soundManager.getError(), "");

    m_soundManager.clearError();
    EXPECT_EQ(m_soundManager.getError(), "");

    EXPECT_FALSE(m_soundManager.resumeMusic(INVALID_MUSIC_HANDLE));
    EXPECT_NE(m_soundManager.getError(), "");
}

TEST_F(SoundManagerTests, UpdateFrequency)
{
    initializeSoundManager();

    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < 100; ++i)
    {
        m_soundManager.update();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    auto duration = std::chrono::steady_clock::now() - start;
    EXPECT_LT(duration, std::chrono::milliseconds(200));
}

TEST_F(SoundManagerTests, ConcurrentAccess)
{
    initializeSoundManager();

    std::vector<std::future<void>> futures;
    std::atomic<int> successCount{0};

    for (int i = 0; i < 4; ++i)
    {
        futures.emplace_back(std::async(std::launch::async, [this, &successCount]()
                                        {
            for (int j = 0; j < 10; ++j) {
                auto handle = m_soundManager.playSound("general/sfx/beep.wav");
                if (handle != INVALID_SOUND_HANDLE) {
                    successCount++;
                    m_soundManager.update();
                    m_soundManager.stopSound(handle);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            } }));
    }

    for (auto &future : futures)
    {
        future.wait();
    }

    EXPECT_GT(successCount.load(), 0);
}

TEST_F(SoundManagerTests, FadeValidation)
{
    initializeSoundManager();

    auto handle = m_soundManager.playSound("general/sfx/beep.wav");
    ASSERT_NE(handle, INVALID_SOUND_HANDLE);

    m_soundManager.update();

    EXPECT_FALSE(m_soundManager.fadeOutSound(handle, 0.0f));
    EXPECT_FALSE(m_soundManager.fadeOutSound(handle, -1.0f));
    EXPECT_FALSE(m_soundManager.fadeToVolumeSound(handle, -0.5f, 1.0f));
    EXPECT_FALSE(m_soundManager.fadeToVolumeSound(handle, 1.0f, 0.0f));

    EXPECT_NE(m_soundManager.getError(), "");
}

TEST_F(SoundManagerTests, InvalidHandleOperations)
{
    initializeSoundManager();

    EXPECT_FALSE(m_soundManager.isSoundPlaying(INVALID_SOUND_HANDLE));
    EXPECT_FALSE(m_soundManager.isMusicPlaying(INVALID_MUSIC_HANDLE));
    EXPECT_FALSE(m_soundManager.setSoundVolume(999, 0.5f));
    EXPECT_FALSE(m_soundManager.setMusicPitch(999, 1.2f));
    EXPECT_FALSE(m_soundManager.fadeOutSound(999, 1.0f));
    EXPECT_FALSE(m_soundManager.fadeOutMusic(999, 1.0f));

    EXPECT_NE(m_soundManager.getError(), "");
}