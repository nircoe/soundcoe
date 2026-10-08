#include <gtest/gtest.h>
#include <soundcoe/playback/sound_manager.hpp>
#include <soundcoe/core/types.hpp>
#include "utils/test_audio_files.hpp"
#include <thread>
#include <chrono>
#include <vector>
#include <future>
#include <atomic>

using namespace soundcoe;
using namespace soundcoe::internal;

//==============================================================================
//               SoundManagerTests - SoundManager comprehensive tests
//==============================================================================

class SoundManagerTests : public ::testing::Test
{
protected:
    sound_manager m_sound_manager;

    void SetUp() override
    {
        test_audio_files::create_test_files();
    }

    void TearDown() override
    {
        try
        {
            m_sound_manager.shutdown();
        }
        catch (...)
        {
        }
        test_audio_files::cleanup();
    }

    void initialize_sound_manager()
    {
        ASSERT_TRUE(m_sound_manager.initialize(test_audio_files::s_test_root_dir.string(), 8, 32));
    }

    void wait_for_fade(float duration)
    {
        auto start = std::chrono::steady_clock::now();
        auto targetDuration = std::chrono::duration<float>(duration + 0.1f); // Small buffer

        while (std::chrono::steady_clock::now() - start < targetDuration)
        {
            m_sound_manager.update();
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        }
    }
};

TEST_F(SoundManagerTests, InitializationAndShutdown)
{
    initialize_sound_manager();
    EXPECT_TRUE(m_sound_manager.is_initialized());

    m_sound_manager.shutdown();
    EXPECT_FALSE(m_sound_manager.is_initialized());

    EXPECT_TRUE(m_sound_manager.initialize(test_audio_files::s_test_root_dir.string()));
    EXPECT_TRUE(m_sound_manager.is_initialized());

    auto r = m_sound_manager.initialize(test_audio_files::s_test_root_dir.string());
    ASSERT_FALSE(r);
    EXPECT_EQ(r.error().code, error_code::already_initialized);
    EXPECT_TRUE(m_sound_manager.is_initialized());
}

TEST_F(SoundManagerTests, InvalidInitialization)
{
    sound_manager manager;

    auto empty = manager.initialize("");
    ASSERT_FALSE(empty);
    EXPECT_EQ(empty.error().code, error_code::invalid_argument);

    auto missing = manager.initialize("/nonexistent/path");
    ASSERT_FALSE(missing);
    EXPECT_EQ(missing.error().code, error_code::directory_not_found);

    EXPECT_FALSE(manager.is_initialized());
}

TEST_F(SoundManagerTests, PlaySoundBasic)
{
    initialize_sound_manager();

    auto handle = m_sound_manager.play_sound("beep.wav");
    ASSERT_TRUE(handle);
    EXPECT_TRUE(sound_manager::is_handle_valid(*handle));

    m_sound_manager.update();
    auto r = m_sound_manager.is_sound_playing(*handle);
    ASSERT_TRUE(r);
    EXPECT_TRUE(*r);
    EXPECT_EQ(m_sound_manager.get_active_sounds_count(), 1);
}

TEST_F(SoundManagerTests, PlayMusicBasic)
{
    initialize_sound_manager();

    auto handle = m_sound_manager.play_music("background.wav");
    ASSERT_TRUE(handle);
    EXPECT_TRUE(sound_manager::is_handle_valid(*handle));

    m_sound_manager.update();
    auto r = m_sound_manager.is_music_playing(*handle);
    ASSERT_TRUE(r);
    EXPECT_TRUE(*r);
    EXPECT_EQ(m_sound_manager.get_active_music_count(), 1);
}

TEST_F(SoundManagerTests, PlaySound3D)
{
    initialize_sound_manager();

    vec3 position(1.0f, 2.0f, 3.0f);
    vec3 velocity(0.1f, 0.2f, 0.3f);

    auto handle = m_sound_manager.play_sound3d("beep.wav", position, velocity);
    ASSERT_TRUE(handle);

    m_sound_manager.update();
    auto r = m_sound_manager.is_sound_playing(*handle);
    ASSERT_TRUE(r);
    EXPECT_TRUE(*r);
}

TEST_F(SoundManagerTests, InvalidFileHandling)
{
    initialize_sound_manager();

    auto handle = m_sound_manager.play_sound("nonexistent/file.wav");
    ASSERT_FALSE(handle);
    EXPECT_EQ(handle.error().code, error_code::file_not_found);

    auto music_handle_ = m_sound_manager.play_music("");
    ASSERT_FALSE(music_handle_);
    EXPECT_EQ(music_handle_.error().code, error_code::file_not_found);
}

TEST_F(SoundManagerTests, PlayErrors)
{
    // Test 1: play_sound before initialize
    {
        sound_manager manager;
        auto r = manager.play_sound("beep.wav");
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::not_initialized);
    }

    initialize_sound_manager();

    // Test 2: missing file
    {
        auto r = m_sound_manager.play_sound("missing.wav");
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::file_not_found);
    }

    // Test 3: failed play leaves nothing active
    EXPECT_EQ(m_sound_manager.get_active_sounds_count(), 0);
}

TEST_F(SoundManagerTests, PlaybackControls)
{
    initialize_sound_manager();

    auto handle = m_sound_manager.play_sound("beep.wav");
    ASSERT_TRUE(handle);

    m_sound_manager.update();
    auto playing = m_sound_manager.is_sound_playing(*handle);
    ASSERT_TRUE(playing);
    EXPECT_TRUE(*playing);

    EXPECT_TRUE(m_sound_manager.pause_sound(*handle));
    m_sound_manager.update();
    auto paused = m_sound_manager.is_sound_paused(*handle);
    ASSERT_TRUE(paused);
    EXPECT_TRUE(*paused);

    EXPECT_TRUE(m_sound_manager.resume_sound(*handle));
    m_sound_manager.update();
    auto resumed = m_sound_manager.is_sound_playing(*handle);
    ASSERT_TRUE(resumed);
    EXPECT_TRUE(*resumed);

    EXPECT_TRUE(m_sound_manager.stop_sound(*handle));
    m_sound_manager.update();
    EXPECT_EQ(m_sound_manager.get_active_sounds_count(), 0);
}

TEST_F(SoundManagerTests, VolumeAndPitchControls)
{
    initialize_sound_manager();

    auto handle = m_sound_manager.play_sound("beep.wav", 0.5f, 1.2f);
    ASSERT_TRUE(handle);

    m_sound_manager.update();

    EXPECT_TRUE(m_sound_manager.set_sound_volume(*handle, 0.8f));
    EXPECT_TRUE(m_sound_manager.set_sound_pitch(*handle, 0.9f));

    vec3 new_pos(5.0f, 6.0f, 7.0f);
    EXPECT_TRUE(m_sound_manager.set_sound_position(*handle, new_pos));

    vec3 new_vel(0.5f, 0.6f, 0.7f);
    EXPECT_TRUE(m_sound_manager.set_sound_velocity(*handle, new_vel));
}

TEST_F(SoundManagerTests, MasterVolumeControls)
{
    initialize_sound_manager();

    EXPECT_TRUE(m_sound_manager.set_master_volume(0.8f));
    EXPECT_EQ(m_sound_manager.get_master_volume(), 0.8f);

    EXPECT_TRUE(m_sound_manager.set_master_sounds_volume(0.6f));
    EXPECT_EQ(m_sound_manager.get_master_sounds_volume(), 0.6f);

    EXPECT_TRUE(m_sound_manager.set_master_music_volume(0.4f));
    EXPECT_EQ(m_sound_manager.get_master_music_volume(), 0.4f);
}

TEST_F(SoundManagerTests, MasterPitchControls)
{
    initialize_sound_manager();

    EXPECT_TRUE(m_sound_manager.set_master_pitch(1.2f));
    EXPECT_EQ(m_sound_manager.get_master_pitch(), 1.2f);

    EXPECT_TRUE(m_sound_manager.set_master_sounds_pitch(0.9f));
    EXPECT_EQ(m_sound_manager.get_master_sounds_pitch(), 0.9f);

    EXPECT_TRUE(m_sound_manager.set_master_music_pitch(1.1f));
    EXPECT_EQ(m_sound_manager.get_master_music_pitch(), 1.1f);
}

TEST_F(SoundManagerTests, MuteControls)
{
    initialize_sound_manager();

    EXPECT_FALSE(m_sound_manager.is_muted());
    EXPECT_FALSE(m_sound_manager.is_sounds_muted());
    EXPECT_FALSE(m_sound_manager.is_music_muted());

    EXPECT_TRUE(m_sound_manager.mute_all_sounds());
    EXPECT_TRUE(m_sound_manager.is_sounds_muted());

    EXPECT_TRUE(m_sound_manager.mute_all_music());
    EXPECT_TRUE(m_sound_manager.is_music_muted());

    EXPECT_TRUE(m_sound_manager.mute_all());
    EXPECT_TRUE(m_sound_manager.is_muted());

    EXPECT_TRUE(m_sound_manager.unmute_all());
    EXPECT_FALSE(m_sound_manager.is_muted());
    EXPECT_FALSE(m_sound_manager.is_sounds_muted());
    EXPECT_FALSE(m_sound_manager.is_music_muted());
}

TEST_F(SoundManagerTests, ListenerControls)
{
    initialize_sound_manager();

    vec3 position(1.0f, 2.0f, 3.0f);
    vec3 velocity(0.1f, 0.2f, 0.3f);
    vec3 forward(0.0f, 0.0f, -1.0f);
    vec3 up(0.0f, 1.0f, 0.0f);

    EXPECT_TRUE(m_sound_manager.update_listener(position, velocity, forward, up));

    EXPECT_EQ(m_sound_manager.get_listener_position(), position);
    EXPECT_EQ(m_sound_manager.get_listener_velocity(), velocity);
    EXPECT_EQ(m_sound_manager.get_listener_forward(), forward);
    EXPECT_EQ(m_sound_manager.get_listener_up(), up);

    vec3 new_position(4.0f, 5.0f, 6.0f);
    EXPECT_TRUE(m_sound_manager.set_listener_position(new_position));
    EXPECT_EQ(m_sound_manager.get_listener_position(), new_position);
}

TEST_F(SoundManagerTests, FadeInSound)
{
    initialize_sound_manager();

    auto handle = m_sound_manager.fade_in_sound("beep.wav", 0.1f, 1.0f);
    ASSERT_TRUE(handle);

    m_sound_manager.update();
    auto playing = m_sound_manager.is_sound_playing(*handle);
    ASSERT_TRUE(playing);
    EXPECT_TRUE(*playing);

    wait_for_fade(0.1f);
    auto still_playing = m_sound_manager.is_sound_playing(*handle);
    ASSERT_TRUE(still_playing);
    EXPECT_TRUE(*still_playing);
}

TEST_F(SoundManagerTests, FadeInInvalidDuration)
{
    initialize_sound_manager();

    auto sound = m_sound_manager.fade_in_sound("beep.wav", 0.0f);
    ASSERT_FALSE(sound);
    EXPECT_EQ(sound.error().code, error_code::invalid_argument);
    EXPECT_EQ(m_sound_manager.get_active_sounds_count(), 0);

    auto music = m_sound_manager.fade_in_music("background.wav", -1.0f);
    ASSERT_FALSE(music);
    EXPECT_EQ(music.error().code, error_code::invalid_argument);
    EXPECT_EQ(m_sound_manager.get_active_music_count(), 0);
}

TEST_F(SoundManagerTests, FadeInMusic)
{
    initialize_sound_manager();

    auto handle = m_sound_manager.fade_in_music("background.wav", 0.1f, 0.8f);
    ASSERT_TRUE(handle);

    m_sound_manager.update();
    auto playing = m_sound_manager.is_music_playing(*handle);
    ASSERT_TRUE(playing);
    EXPECT_TRUE(*playing);

    wait_for_fade(0.1f);
    auto still_playing = m_sound_manager.is_music_playing(*handle);
    ASSERT_TRUE(still_playing);
    EXPECT_TRUE(*still_playing);
}

TEST_F(SoundManagerTests, FadeOutSound)
{
    initialize_sound_manager();

    auto handle = m_sound_manager.play_sound("beep.wav");
    ASSERT_TRUE(handle);

    m_sound_manager.update();
    auto r = m_sound_manager.is_sound_playing(*handle);
    ASSERT_TRUE(r);
    EXPECT_TRUE(*r);

    EXPECT_TRUE(m_sound_manager.fade_out_sound(*handle, 0.1f));
    wait_for_fade(1.0f);

    EXPECT_EQ(m_sound_manager.get_active_sounds_count(), 0);
}

TEST_F(SoundManagerTests, FadeOutMusic)
{
    initialize_sound_manager();

    auto handle = m_sound_manager.play_music("background.wav");
    ASSERT_TRUE(handle);

    m_sound_manager.update();
    auto r = m_sound_manager.is_music_playing(*handle);
    ASSERT_TRUE(r);
    EXPECT_TRUE(*r);

    EXPECT_TRUE(m_sound_manager.fade_out_music(*handle, 0.1f));
    wait_for_fade(1.0f);

    EXPECT_EQ(m_sound_manager.get_active_music_count(), 0);
}

TEST_F(SoundManagerTests, FadeToVolume)
{
    initialize_sound_manager();

    auto sound_handle_ = m_sound_manager.play_sound("beep.wav", 1.0f);
    ASSERT_TRUE(sound_handle_);

    auto music_handle_ = m_sound_manager.play_music("background.wav", 0.5f);
    ASSERT_TRUE(music_handle_);

    m_sound_manager.update();

    EXPECT_TRUE(m_sound_manager.fade_to_volume_sound(*sound_handle_, 0.3f, 0.1f));
    EXPECT_TRUE(m_sound_manager.fade_to_volume_music(*music_handle_, 0.8f, 0.1f));

    wait_for_fade(0.1f);

    auto sound_playing = m_sound_manager.is_sound_playing(*sound_handle_);
    ASSERT_TRUE(sound_playing);
    EXPECT_TRUE(*sound_playing);
    auto music_playing = m_sound_manager.is_music_playing(*music_handle_);
    ASSERT_TRUE(music_playing);
    EXPECT_TRUE(*music_playing);
}

TEST_F(SoundManagerTests, SceneManagement)
{
    initialize_sound_manager();

    EXPECT_TRUE(m_sound_manager.is_scene_loaded("general"));

    EXPECT_TRUE(m_sound_manager.preload_scene("scene1"));
    EXPECT_TRUE(m_sound_manager.is_scene_loaded("scene1"));

    auto handle = m_sound_manager.play_sound("explosion.wav");
    EXPECT_TRUE(handle);

    EXPECT_TRUE(m_sound_manager.unload_scene("scene1"));
    EXPECT_FALSE(m_sound_manager.is_scene_loaded("scene1"));
}

TEST_F(SoundManagerTests, InvalidSceneOperations)
{
    initialize_sound_manager();

    auto empty = m_sound_manager.preload_scene("");
    ASSERT_FALSE(empty);
    EXPECT_EQ(empty.error().code, error_code::invalid_argument);

    auto missing = m_sound_manager.preload_scene("nonexistent");
    ASSERT_FALSE(missing);
    EXPECT_EQ(missing.error().code, error_code::directory_not_found);

    EXPECT_FALSE(m_sound_manager.is_scene_loaded(""));
    EXPECT_FALSE(m_sound_manager.is_scene_loaded("nonexistent"));
}

TEST_F(SoundManagerTests, MultipleSoundsAndMusic)
{
    initialize_sound_manager();

    std::vector<sound_handle> sound_handles;
    std::vector<music_handle> music_handles;

    for (int i = 0; i < 3; ++i)
    {
        auto sound_handle_ = m_sound_manager.play_sound("beep.wav");
        ASSERT_TRUE(sound_handle_);
        sound_handles.push_back(*sound_handle_);

        auto music_handle_ = m_sound_manager.play_music("background.wav");
        ASSERT_TRUE(music_handle_);
        music_handles.push_back(*music_handle_);
    }

    m_sound_manager.update();
    EXPECT_EQ(m_sound_manager.get_active_sounds_count(), 3);
    EXPECT_EQ(m_sound_manager.get_active_music_count(), 3);

    EXPECT_TRUE(m_sound_manager.pause_all_sounds());
    EXPECT_TRUE(m_sound_manager.pause_all_music());

    m_sound_manager.update();
    for (auto handle : sound_handles)
    {
        auto r = m_sound_manager.is_sound_paused(handle);
        ASSERT_TRUE(r);
        EXPECT_TRUE(*r);
    }
    for (auto handle : music_handles)
    {
        auto r = m_sound_manager.is_music_paused(handle);
        ASSERT_TRUE(r);
        EXPECT_TRUE(*r);
    }

    EXPECT_TRUE(m_sound_manager.stop_all());
    m_sound_manager.update();
    EXPECT_EQ(m_sound_manager.get_active_sounds_count(), 0);
    EXPECT_EQ(m_sound_manager.get_active_music_count(), 0);
}

TEST_F(SoundManagerTests, ErrorHandling)
{
    initialize_sound_manager();

    auto pause = m_sound_manager.pause_sound(INVALID_SOUND_HANDLE);
    ASSERT_FALSE(pause);
    EXPECT_EQ(pause.error().code, error_code::invalid_handle);

    auto resume = m_sound_manager.resume_music(INVALID_MUSIC_HANDLE);
    ASSERT_FALSE(resume);
    EXPECT_EQ(resume.error().code, error_code::invalid_handle);
}

TEST_F(SoundManagerTests, PlaybackControlErrors)
{
    initialize_sound_manager();

    // Test 1: stop_sound with an invalid handle
    {
        auto r = m_sound_manager.stop_sound(INVALID_SOUND_HANDLE);
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::invalid_handle);
    }

    // Test 2: resume_sound on a sound that is playing
    {
        auto handle = m_sound_manager.play_sound("beep.wav");
        ASSERT_TRUE(handle);

        auto r = m_sound_manager.resume_sound(*handle);
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::invalid_state);
    }

    // Test 3: is_sound_playing with an invalid handle
    {
        auto r = m_sound_manager.is_sound_playing(INVALID_SOUND_HANDLE);
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::invalid_handle);
    }

    // Test 4: is_sound_playing on a sound that was stopped and released
    {
        auto handle = m_sound_manager.play_sound("beep.wav");
        ASSERT_TRUE(handle);
        ASSERT_TRUE(m_sound_manager.stop_sound(*handle));

        auto r = m_sound_manager.is_sound_playing(*handle);
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::invalid_handle);
    }
}

TEST_F(SoundManagerTests, ConcurrentAccess)
{
    initialize_sound_manager();

    std::vector<std::future<void>> futures;
    std::atomic<int> successCount{0};

    for (int i = 0; i < 4; ++i)
    {
        futures.emplace_back(std::async(std::launch::async, [this, &successCount]()
                                        {
            for (int j = 0; j < 10; ++j) {
                auto handle = m_sound_manager.play_sound("beep.wav");
                if (handle) {
                    successCount++;
                    m_sound_manager.update();
                    static_cast<void>(m_sound_manager.stop_sound(*handle));
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
    initialize_sound_manager();

    auto handle = m_sound_manager.play_sound("beep.wav");
    ASSERT_TRUE(handle);

    m_sound_manager.update();

    auto zero_duration = m_sound_manager.fade_out_sound(*handle, 0.0f);
    ASSERT_FALSE(zero_duration);
    EXPECT_EQ(zero_duration.error().code, error_code::invalid_argument);

    auto negative_duration = m_sound_manager.fade_out_sound(*handle, -1.0f);
    ASSERT_FALSE(negative_duration);
    EXPECT_EQ(negative_duration.error().code, error_code::invalid_argument);

    auto negative_target = m_sound_manager.fade_to_volume_sound(*handle, -0.5f, 1.0f);
    ASSERT_FALSE(negative_target);
    EXPECT_EQ(negative_target.error().code, error_code::invalid_argument);

    auto zero_fade_duration = m_sound_manager.fade_to_volume_sound(*handle, 1.0f, 0.0f);
    ASSERT_FALSE(zero_fade_duration);
    EXPECT_EQ(zero_fade_duration.error().code, error_code::invalid_argument);
}

TEST_F(SoundManagerTests, InvalidHandleOperations)
{
    initialize_sound_manager();

    auto sound_playing = m_sound_manager.is_sound_playing(INVALID_SOUND_HANDLE);
    ASSERT_FALSE(sound_playing);
    EXPECT_EQ(sound_playing.error().code, error_code::invalid_handle);

    auto music_playing = m_sound_manager.is_music_playing(INVALID_MUSIC_HANDLE);
    ASSERT_FALSE(music_playing);
    EXPECT_EQ(music_playing.error().code, error_code::invalid_handle);

    EXPECT_FALSE(m_sound_manager.set_sound_volume(999, 0.5f));
    EXPECT_FALSE(m_sound_manager.set_music_pitch(999, 1.2f));

    auto sound = m_sound_manager.fade_out_sound(999, 1.0f);
    ASSERT_FALSE(sound);
    EXPECT_EQ(sound.error().code, error_code::invalid_handle);

    auto music = m_sound_manager.fade_out_music(999, 1.0f);
    ASSERT_FALSE(music);
    EXPECT_EQ(music.error().code, error_code::invalid_handle);

    auto invalid = m_sound_manager.fade_out_sound(INVALID_SOUND_HANDLE, 1.0f);
    ASSERT_FALSE(invalid);
    EXPECT_EQ(invalid.error().code, error_code::invalid_handle);

    EXPECT_NE(m_sound_manager.get_error(), "");
}
