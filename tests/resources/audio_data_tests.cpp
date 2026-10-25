#include <soundcoe/core/error.hpp>
#include <soundcoe/resources/audio_data.hpp>
#include <utils/test_audio_files.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace soundcoe;
using namespace soundcoe::internal;

//==============================================================================
//             AudioDataTests - audio_data decoder failure tests
//==============================================================================

class AudioDataTests : public ::testing::Test
{
protected:
    static void SetUpTestSuite() { test_audio_files::create_test_files(); }
    static void TearDownTestSuite() { test_audio_files::cleanup(); }
};

TEST_F(AudioDataTests, DecodeFailures)
{
    std::string filename = (test_audio_files::s_test_sub_dir1 / "readme.txt").string();

    // Test 1: wav decoder fails on a text file
    {
        auto r = audio_data::load_from_wav(filename);
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::audio_decode_failure);
        EXPECT_NE(r.error().message.find(filename), std::string::npos);
    }

    // Test 2: ogg decoder fails on a text file
    {
        auto r = audio_data::load_from_ogg(filename);
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::audio_decode_failure);
        EXPECT_NE(r.error().message.find(filename), std::string::npos);
    }

    // Test 3: mp3 decoder fails on a text file
    {
        auto r = audio_data::load_from_mp3(filename);
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::audio_decode_failure);
        EXPECT_NE(r.error().message.find(filename), std::string::npos);
    }

    // Test 4: all decoders fail on a file that doesn't exist
    {
        std::string missing = "nonexistent_file";

        auto wav = audio_data::load_from_wav(missing);
        ASSERT_FALSE(wav);
        EXPECT_EQ(wav.error().code, error_code::audio_decode_failure);

        auto ogg = audio_data::load_from_ogg(missing);
        ASSERT_FALSE(ogg);
        EXPECT_EQ(ogg.error().code, error_code::audio_decode_failure);

        auto mp3 = audio_data::load_from_mp3(missing);
        ASSERT_FALSE(mp3);
        EXPECT_EQ(mp3.error().code, error_code::audio_decode_failure);
    }
}
