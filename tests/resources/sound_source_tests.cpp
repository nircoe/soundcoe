#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error.hpp>
#include <soundcoe/core/types.hpp>
#include <soundcoe/resources/sound_buffer.hpp>
#include <soundcoe/resources/sound_source.hpp>
#include <utils/test_audio_files.hpp>
#include <gtest/gtest.h>
#include <AL/al.h>
#include <expected>
#include <string>
#include <utility>
#include <vector>

using namespace soundcoe;
using namespace soundcoe::internal;

//==============================================================================
//        SoundSourceTests - sound_source playback and properties tests
//==============================================================================

class SoundSourceTests : public ::testing::Test
{
private:
    audio_context m_audio_context;

protected:
    void SetUp() override
    {
        test_audio_files::create_test_files();
        ASSERT_TRUE(m_audio_context.initialize());
    }

    void TearDown() override
    {
        static_cast<void>(m_audio_context.shutdown());
    }

    static void SetUpTestSuite() { test_audio_files::create_test_files(); }
    static void TearDownTestSuite() { test_audio_files::cleanup(); }
};

TEST_F(SoundSourceTests, Properties)
{
    // Test 1: default source is not created
    {
        sound_source source;
        EXPECT_FALSE(source.is_created());
        EXPECT_EQ(source.get_source_id(), 0);
        EXPECT_EQ(source.get_volume(), 1.0f);
        EXPECT_EQ(source.get_pitch(), 1.0f);
        EXPECT_FALSE(source.is_looping());
    }

    // Test 2: setters and getters
    {
        sound_source source;
        ASSERT_TRUE(source.create());

        EXPECT_TRUE(source.set_volume(0.5f));
        EXPECT_FLOAT_EQ(source.get_volume(), 0.5f);

        EXPECT_TRUE(source.set_pitch(1.5f));
        EXPECT_FLOAT_EQ(source.get_pitch(), 1.5f);

        vec3 pos(1.0f, 2.0f, 3.0f);
        EXPECT_TRUE(source.set_position(pos));
        EXPECT_EQ(source.get_position().x, pos.x);
        EXPECT_EQ(source.get_position().y, pos.y);
        EXPECT_EQ(source.get_position().z, pos.z);

        EXPECT_TRUE(source.set_looping(true));
        EXPECT_TRUE(source.is_looping());
    }

    // Test 3: moving leaves the source empty
    {
        sound_source source1;
        ALuint original_id = source1.get_source_id();

        sound_source source2 = std::move(source1);
        EXPECT_EQ(source2.get_source_id(), original_id);
        EXPECT_FALSE(source1.is_created());
        EXPECT_EQ(source1.get_source_id(), 0);
    }
}

TEST_F(SoundSourceTests, Playback)
{
    // Test 1: attach, play, pause, stop and detach
    {
        std::string filename = (test_audio_files::s_test_sub_dir1 / "test1.wav").string();
        sound_buffer buffer;
        ASSERT_TRUE(buffer.load_from_file(filename));
        sound_source source;

        EXPECT_TRUE(source.attach_buffer(buffer));
        EXPECT_EQ(source.get_buffer_id(), buffer.get_buffer_id());

        EXPECT_TRUE(source.play());
        EXPECT_TRUE(source.pause());
        EXPECT_TRUE(source.stop());
        EXPECT_TRUE(source.is_stopped());

        EXPECT_TRUE(source.detach_buffer());
        EXPECT_EQ(source.get_buffer_id(), 0);
    }

    // Test 2: state after play and stop
    {
        std::string filename = (test_audio_files::s_test_sub_dir1 / "test1.wav").string();
        sound_buffer buffer;
        ASSERT_TRUE(buffer.load_from_file(filename));
        sound_source source;

        ASSERT_TRUE(source.attach_buffer(buffer));
        EXPECT_EQ(source.get_state(), sound_state::initial);
        EXPECT_FALSE(source.is_stopped());

        ASSERT_TRUE(source.play());
        sound_state state = source.get_state();
        EXPECT_TRUE(state == sound_state::playing || state == sound_state::stopped);

        ASSERT_TRUE(source.stop());
        EXPECT_EQ(source.get_state(), sound_state::stopped);
    }
}

TEST_F(SoundSourceTests, NotCreatedOperations)
{
    // Test 1: every operation on a source that was never created gives invalid_state
    {
        sound_source source;

        std::vector<std::expected<void, error>> results;
        results.push_back(source.play());
        results.push_back(source.pause());
        results.push_back(source.stop());
        results.push_back(source.set_volume(0.5f));
        results.push_back(source.set_pitch(1.5f));
        results.push_back(source.set_position(vec3(1.0f, 2.0f, 3.0f)));
        results.push_back(source.set_velocity(vec3(1.0f, 2.0f, 3.0f)));
        results.push_back(source.set_looping(true));

        for (const auto &r : results)
        {
            ASSERT_FALSE(r);
            EXPECT_EQ(r.error().code, error_code::invalid_state);
        }
        EXPECT_FALSE(source.is_created());
    }

    // Test 2: the message names the operation
    {
        sound_source source;
        auto r = source.play();
        ASSERT_FALSE(r);
        EXPECT_NE(r.error().message.find("sound_source::play"), std::string::npos);
    }

    // Test 3: destroy and detach_buffer on a source that was never created are a success
    {
        sound_source source;
        EXPECT_TRUE(source.detach_buffer());
        EXPECT_TRUE(source.destroy());
    }
}

TEST_F(SoundSourceTests, CreateAndDestroy)
{
    sound_source source;

    // Test 1: create works and a second create stays a success
    {
        ASSERT_TRUE(source.create());
        EXPECT_TRUE(source.is_created());
        EXPECT_NE(source.get_source_id(), 0);
        EXPECT_TRUE(source.create());
    }

    // Test 2: destroy resets the source and operations fail again afterwards
    {
        ASSERT_TRUE(source.destroy());
        EXPECT_FALSE(source.is_created());
        EXPECT_EQ(source.get_source_id(), 0);

        auto r = source.play();
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::invalid_state);
    }
}
