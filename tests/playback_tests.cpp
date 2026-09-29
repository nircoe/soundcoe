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
    sound_manager m_soundManager;

    void SetUp() override
    {
        test_audio_files::create_test_files();
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
        test_audio_files::cleanup();
    }

    void initializeSoundManager()
    {
        ASSERT_TRUE(m_soundManager.initialize(test_audio_files::s_test_root_dir.string(), 8, 32));
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
    EXPECT_TRUE(m_soundManager.is_initialized());

    m_soundManager.shutdown();
    EXPECT_FALSE(m_soundManager.is_initialized());

    EXPECT_TRUE(m_soundManager.initialize(test_audio_files::s_test_root_dir.string()));
    EXPECT_TRUE(m_soundManager.is_initialized());
}

TEST_F(SoundManagerTests, InvalidInitialization)
{
    sound_manager manager;
    EXPECT_FALSE(manager.initialize(""));
    EXPECT_FALSE(manager.initialize("/nonexistent/path"));
    EXPECT_FALSE(manager.is_initialized());
}

TEST_F(SoundManagerTests, PlaySoundBasic)
{
    initializeSoundManager();

    auto handle = m_soundManager.play_sound("beep.wav");
    EXPECT_NE(handle, INVALID_SOUND_HANDLE);
    EXPECT_TRUE(sound_manager::is_handle_valid(handle));

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.is_sound_playing(handle));
    EXPECT_EQ(m_soundManager.get_active_sounds_count(), 1);
}

TEST_F(SoundManagerTests, PlayMusicBasic)
{
    initializeSoundManager();

    auto handle = m_soundManager.play_music("background.wav");
    EXPECT_NE(handle, INVALID_MUSIC_HANDLE);
    EXPECT_TRUE(sound_manager::is_handle_valid(handle));

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.is_music_playing(handle));
    EXPECT_EQ(m_soundManager.get_active_music_count(), 1);
}

TEST_F(SoundManagerTests, PlaySound3D)
{
    initializeSoundManager();

    vec3 position(1.0f, 2.0f, 3.0f);
    vec3 velocity(0.1f, 0.2f, 0.3f);

    auto handle = m_soundManager.play_sound3d("beep.wav", position, velocity);
    EXPECT_NE(handle, INVALID_SOUND_HANDLE);

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.is_sound_playing(handle));
}

TEST_F(SoundManagerTests, InvalidFileHandling)
{
    initializeSoundManager();

    auto handle = m_soundManager.play_sound("nonexistent/file.wav");
    EXPECT_EQ(handle, INVALID_SOUND_HANDLE);
    EXPECT_FALSE(sound_manager::is_handle_valid(handle));

    auto music_handle_ = m_soundManager.play_music("");
    EXPECT_EQ(music_handle_, INVALID_MUSIC_HANDLE);
}

TEST_F(SoundManagerTests, PlaybackControls)
{
    initializeSoundManager();

    auto handle = m_soundManager.play_sound("beep.wav");
    ASSERT_NE(handle, INVALID_SOUND_HANDLE);

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.is_sound_playing(handle));

    EXPECT_TRUE(m_soundManager.pause_sound(handle));
    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.is_sound_paused(handle));

    EXPECT_TRUE(m_soundManager.resume_sound(handle));
    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.is_sound_playing(handle));

    EXPECT_TRUE(m_soundManager.stop_sound(handle));
    m_soundManager.update();
    EXPECT_EQ(m_soundManager.get_active_sounds_count(), 0);
}

TEST_F(SoundManagerTests, VolumeAndPitchControls)
{
    initializeSoundManager();

    auto handle = m_soundManager.play_sound("beep.wav", 0.5f, 1.2f);
    ASSERT_NE(handle, INVALID_SOUND_HANDLE);

    m_soundManager.update();

    EXPECT_TRUE(m_soundManager.set_sound_volume(handle, 0.8f));
    EXPECT_TRUE(m_soundManager.set_sound_pitch(handle, 0.9f));

    vec3 newPos(5.0f, 6.0f, 7.0f);
    EXPECT_TRUE(m_soundManager.set_sound_position(handle, newPos));

    vec3 newVel(0.5f, 0.6f, 0.7f);
    EXPECT_TRUE(m_soundManager.set_sound_velocity(handle, newVel));
}

TEST_F(SoundManagerTests, MasterVolumeControls)
{
    initializeSoundManager();

    EXPECT_TRUE(m_soundManager.set_master_volume(0.8f));
    EXPECT_EQ(m_soundManager.get_master_volume(), 0.8f);

    EXPECT_TRUE(m_soundManager.set_master_sounds_volume(0.6f));
    EXPECT_EQ(m_soundManager.get_master_sounds_volume(), 0.6f);

    EXPECT_TRUE(m_soundManager.set_master_music_volume(0.4f));
    EXPECT_EQ(m_soundManager.get_master_music_volume(), 0.4f);
}

TEST_F(SoundManagerTests, MasterPitchControls)
{
    initializeSoundManager();

    EXPECT_TRUE(m_soundManager.set_master_pitch(1.2f));
    EXPECT_EQ(m_soundManager.get_master_pitch(), 1.2f);

    EXPECT_TRUE(m_soundManager.set_master_sounds_pitch(0.9f));
    EXPECT_EQ(m_soundManager.get_master_sounds_pitch(), 0.9f);

    EXPECT_TRUE(m_soundManager.set_master_music_pitch(1.1f));
    EXPECT_EQ(m_soundManager.get_master_music_pitch(), 1.1f);
}

TEST_F(SoundManagerTests, MuteControls)
{
    initializeSoundManager();

    EXPECT_FALSE(m_soundManager.is_muted());
    EXPECT_FALSE(m_soundManager.is_sounds_muted());
    EXPECT_FALSE(m_soundManager.is_music_muted());

    EXPECT_TRUE(m_soundManager.mute_all_sounds());
    EXPECT_TRUE(m_soundManager.is_sounds_muted());

    EXPECT_TRUE(m_soundManager.mute_all_music());
    EXPECT_TRUE(m_soundManager.is_music_muted());

    EXPECT_TRUE(m_soundManager.mute_all());
    EXPECT_TRUE(m_soundManager.is_muted());

    EXPECT_TRUE(m_soundManager.unmute_all());
    EXPECT_FALSE(m_soundManager.is_muted());
    EXPECT_FALSE(m_soundManager.is_sounds_muted());
    EXPECT_FALSE(m_soundManager.is_music_muted());
}

TEST_F(SoundManagerTests, ListenerControls)
{
    initializeSoundManager();

    vec3 position(1.0f, 2.0f, 3.0f);
    vec3 velocity(0.1f, 0.2f, 0.3f);
    vec3 forward(0.0f, 0.0f, -1.0f);
    vec3 up(0.0f, 1.0f, 0.0f);

    EXPECT_TRUE(m_soundManager.update_listener(position, velocity, forward, up));

    EXPECT_EQ(m_soundManager.get_listener_position(), position);
    EXPECT_EQ(m_soundManager.get_listener_velocity(), velocity);
    EXPECT_EQ(m_soundManager.get_listener_forward(), forward);
    EXPECT_EQ(m_soundManager.get_listener_up(), up);

    vec3 newPosition(4.0f, 5.0f, 6.0f);
    EXPECT_TRUE(m_soundManager.set_listener_position(newPosition));
    EXPECT_EQ(m_soundManager.get_listener_position(), newPosition);
}

TEST_F(SoundManagerTests, FadeInSound)
{
    initializeSoundManager();

    auto handle = m_soundManager.fade_in_sound("beep.wav", 0.1f, 1.0f);
    EXPECT_NE(handle, INVALID_SOUND_HANDLE);

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.is_sound_playing(handle));

    waitForFade(0.1f);
    EXPECT_TRUE(m_soundManager.is_sound_playing(handle));
}

TEST_F(SoundManagerTests, FadeInMusic)
{
    initializeSoundManager();

    auto handle = m_soundManager.fade_in_music("background.wav", 0.1f, 0.8f);
    EXPECT_NE(handle, INVALID_MUSIC_HANDLE);

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.is_music_playing(handle));

    waitForFade(0.1f);
    EXPECT_TRUE(m_soundManager.is_music_playing(handle));
}

TEST_F(SoundManagerTests, FadeOutSound)
{
    initializeSoundManager();

    auto handle = m_soundManager.play_sound("beep.wav");
    ASSERT_NE(handle, INVALID_SOUND_HANDLE);

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.is_sound_playing(handle));

    EXPECT_TRUE(m_soundManager.fade_out_sound(handle, 0.1f));
    waitForFade(1.0f);

    EXPECT_EQ(m_soundManager.get_active_sounds_count(), 0);
}

TEST_F(SoundManagerTests, FadeOutMusic)
{
    initializeSoundManager();

    auto handle = m_soundManager.play_music("background.wav");
    ASSERT_NE(handle, INVALID_MUSIC_HANDLE);

    m_soundManager.update();
    EXPECT_TRUE(m_soundManager.is_music_playing(handle));

    EXPECT_TRUE(m_soundManager.fade_out_music(handle, 0.1f));
    waitForFade(1.0f);

    EXPECT_EQ(m_soundManager.get_active_music_count(), 0);
}

TEST_F(SoundManagerTests, FadeToVolume)
{
    initializeSoundManager();

    auto sound_handle_ = m_soundManager.play_sound("beep.wav", 1.0f);
    ASSERT_NE(sound_handle_, INVALID_SOUND_HANDLE);

    auto music_handle_ = m_soundManager.play_music("background.wav", 0.5f);
    ASSERT_NE(music_handle_, INVALID_MUSIC_HANDLE);

    m_soundManager.update();

    EXPECT_TRUE(m_soundManager.fade_to_volume_sound(sound_handle_, 0.3f, 0.1f));
    EXPECT_TRUE(m_soundManager.fade_to_volume_music(music_handle_, 0.8f, 0.1f));

    waitForFade(0.1f);

    EXPECT_TRUE(m_soundManager.is_sound_playing(sound_handle_));
    EXPECT_TRUE(m_soundManager.is_music_playing(music_handle_));
}

TEST_F(SoundManagerTests, SceneManagement)
{
    initializeSoundManager();

    EXPECT_TRUE(m_soundManager.is_scene_loaded("general"));

    EXPECT_TRUE(m_soundManager.preload_scene("scene1"));
    EXPECT_TRUE(m_soundManager.is_scene_loaded("scene1"));

    auto handle = m_soundManager.play_sound("explosion.wav");
    EXPECT_NE(handle, INVALID_SOUND_HANDLE);

    EXPECT_TRUE(m_soundManager.unload_scene("scene1"));
    EXPECT_FALSE(m_soundManager.is_scene_loaded("scene1"));
}

TEST_F(SoundManagerTests, InvalidSceneOperations)
{
    initializeSoundManager();

    EXPECT_FALSE(m_soundManager.preload_scene(""));
    EXPECT_FALSE(m_soundManager.preload_scene("nonexistent"));
    EXPECT_FALSE(m_soundManager.is_scene_loaded(""));
    EXPECT_FALSE(m_soundManager.is_scene_loaded("nonexistent"));
}

TEST_F(SoundManagerTests, MultipleSoundsAndMusic)
{
    initializeSoundManager();

    std::vector<sound_handle> sound_handles;
    std::vector<music_handle> music_handles;

    for (int i = 0; i < 3; ++i)
    {
        auto sound_handle_ = m_soundManager.play_sound("beep.wav");
        EXPECT_NE(sound_handle_, INVALID_SOUND_HANDLE);
        sound_handles.push_back(sound_handle_);

        auto music_handle_ = m_soundManager.play_music("background.wav");
        EXPECT_NE(music_handle_, INVALID_MUSIC_HANDLE);
        music_handles.push_back(music_handle_);
    }

    m_soundManager.update();
    EXPECT_EQ(m_soundManager.get_active_sounds_count(), 3);
    EXPECT_EQ(m_soundManager.get_active_music_count(), 3);

    EXPECT_TRUE(m_soundManager.pause_all_sounds());
    EXPECT_TRUE(m_soundManager.pause_all_music());

    m_soundManager.update();
    for (auto handle : sound_handles)
    {
        EXPECT_TRUE(m_soundManager.is_sound_paused(handle));
    }
    for (auto handle : music_handles)
    {
        EXPECT_TRUE(m_soundManager.is_music_paused(handle));
    }

    EXPECT_TRUE(m_soundManager.stop_all());
    m_soundManager.update();
    EXPECT_EQ(m_soundManager.get_active_sounds_count(), 0);
    EXPECT_EQ(m_soundManager.get_active_music_count(), 0);
}

TEST_F(SoundManagerTests, ErrorHandling)
{
    initializeSoundManager();

    m_soundManager.clear_error();
    EXPECT_EQ(m_soundManager.get_error(), "");

    EXPECT_FALSE(m_soundManager.pause_sound(INVALID_SOUND_HANDLE));
    EXPECT_NE(m_soundManager.get_error(), "");

    m_soundManager.clear_error();
    EXPECT_EQ(m_soundManager.get_error(), "");

    EXPECT_FALSE(m_soundManager.resume_music(INVALID_MUSIC_HANDLE));
    EXPECT_NE(m_soundManager.get_error(), "");
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
                auto handle = m_soundManager.play_sound("beep.wav");
                if (handle != INVALID_SOUND_HANDLE) {
                    successCount++;
                    m_soundManager.update();
                    m_soundManager.stop_sound(handle);
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

    auto handle = m_soundManager.play_sound("beep.wav");
    ASSERT_NE(handle, INVALID_SOUND_HANDLE);

    m_soundManager.update();

    EXPECT_FALSE(m_soundManager.fade_out_sound(handle, 0.0f));
    EXPECT_FALSE(m_soundManager.fade_out_sound(handle, -1.0f));
    EXPECT_FALSE(m_soundManager.fade_to_volume_sound(handle, -0.5f, 1.0f));
    EXPECT_FALSE(m_soundManager.fade_to_volume_sound(handle, 1.0f, 0.0f));

    EXPECT_NE(m_soundManager.get_error(), "");
}

TEST_F(SoundManagerTests, InvalidHandleOperations)
{
    initializeSoundManager();

    EXPECT_FALSE(m_soundManager.is_sound_playing(INVALID_SOUND_HANDLE));
    EXPECT_FALSE(m_soundManager.is_music_playing(INVALID_MUSIC_HANDLE));
    EXPECT_FALSE(m_soundManager.set_sound_volume(999, 0.5f));
    EXPECT_FALSE(m_soundManager.set_music_pitch(999, 1.2f));
    EXPECT_FALSE(m_soundManager.fade_out_sound(999, 1.0f));
    EXPECT_FALSE(m_soundManager.fade_out_music(999, 1.0f));

    EXPECT_NE(m_soundManager.get_error(), "");
}
