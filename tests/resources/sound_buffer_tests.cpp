#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error.hpp>
#include <soundcoe/resources/sound_buffer.hpp>
#include <utils/test_audio_files.hpp>
#include <gtest/gtest.h>
#include <AL/al.h>
#include <string>
#include <utility>
#include <vector>

using namespace soundcoe;
using namespace soundcoe::internal;

//==============================================================================
//         SoundBufferTests - sound_buffer loading and management tests
//==============================================================================

class SoundBufferTests : public ::testing::Test
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

TEST_F(SoundBufferTests, Loading)
{
    // Test 1: default buffer is empty
    {
        sound_buffer buffer;
        EXPECT_FALSE(buffer.is_loaded());
        EXPECT_EQ(buffer.get_buffer_id(), 0);
        EXPECT_EQ(buffer.get_filename(), "");
    }

    // Test 2: load from a file
    {
        std::string filename = (test_audio_files::s_test_sub_dir1 / "test1.wav").string();
        sound_buffer buffer;
        ASSERT_TRUE(buffer.load_from_file(filename));

        EXPECT_TRUE(buffer.is_loaded());
        EXPECT_NE(buffer.get_buffer_id(), 0);
        EXPECT_GT(buffer.get_duration(), 0.0f);
        EXPECT_EQ(buffer.get_filename(), filename);
    }

    // Test 3: unload resets the buffer
    {
        sound_buffer buffer;
        std::string filename = (test_audio_files::s_test_sub_dir1 / "test1.wav").string();

        ASSERT_TRUE(buffer.load_from_file(filename));
        EXPECT_TRUE(buffer.is_loaded());
        EXPECT_NE(buffer.get_buffer_id(), 0);

        buffer.unload();
        EXPECT_FALSE(buffer.is_loaded());
        EXPECT_EQ(buffer.get_buffer_id(), 0);
    }

    // Test 4: missing and unsupported files
    {
        sound_buffer buffer;
        auto missing = buffer.load_from_file("nonexistent.wav");
        ASSERT_FALSE(missing);
        EXPECT_EQ(missing.error().code, error_code::file_not_found);
        EXPECT_FALSE(buffer.is_loaded());

        std::string txt_file = (test_audio_files::s_test_sub_dir1 / "readme.txt").string();
        auto unsupported = buffer.load_from_file(txt_file);
        ASSERT_FALSE(unsupported);
        EXPECT_EQ(unsupported.error().code, error_code::unsupported_format);
        EXPECT_FALSE(buffer.is_loaded());
    }

    // Test 5: moving leaves the source empty
    {
        std::string filename = (test_audio_files::s_test_sub_dir1 / "test1.wav").string();
        sound_buffer buffer1;
        ASSERT_TRUE(buffer1.load_from_file(filename));
        ALuint original_id = buffer1.get_buffer_id();

        sound_buffer buffer2 = std::move(buffer1);
        EXPECT_EQ(buffer2.get_buffer_id(), original_id);
        EXPECT_FALSE(buffer1.is_loaded());
        EXPECT_EQ(buffer1.get_buffer_id(), 0);
    }
}

TEST_F(SoundBufferTests, LoadFromMemory)
{
    // Test 1: invalid ALenum gives invalid_argument and nothing is loaded
    {
        sound_buffer buffer;
        std::vector<char> data(64, 0);

        auto r = buffer.load_from_memory(data.data(), AL_NONE, static_cast<ALsizei>(data.size()), 8000);
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::invalid_argument);
        EXPECT_FALSE(buffer.is_loaded());
    }

    // Test 2: valid mono16 data loads
    {
        sound_buffer buffer;
        std::vector<char> data(64, 0);

        ASSERT_TRUE(buffer.load_from_memory(data.data(), AL_FORMAT_MONO16, static_cast<ALsizei>(data.size()), 8000));
        EXPECT_TRUE(buffer.is_loaded());
        EXPECT_NE(buffer.get_buffer_id(), 0);
    }
}
