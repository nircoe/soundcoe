#include <soundcoe/core/error.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <soundcoe/core/types.hpp>
#include <gtest/gtest.h>
#include <expected>

using namespace soundcoe;
using namespace soundcoe::internal;

//==============================================================================
//                    ErrorTests - error and to_string tests
//==============================================================================

class ErrorTests : public ::testing::Test
{
};

TEST_F(ErrorTests, ErrorConstruction)
{
    // Test 1: error holds the code and message it was built with
    {
        error err{error_code::invalid_argument, "x"};
        EXPECT_EQ(err.code, error_code::invalid_argument);
        EXPECT_EQ(err.message, "x");
    }

    // Test 2: make_error returns the same code and message it was given
    {
        error err = error_handler::make_error(error_code::file_not_found, "missing file");
        EXPECT_EQ(err.code, error_code::file_not_found);
        EXPECT_EQ(err.message, "missing file");
    }

    // Test 3: std::expected carries the error through std::unexpected
    {
        std::expected<void, error> r =
            std::unexpected(error_handler::make_error(error_code::invalid_state, "bad state"));
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::invalid_state);
        EXPECT_EQ(r.error().message, "bad state");

        std::expected<void, error> ok;
        EXPECT_TRUE(ok.has_value());
    }
}

TEST_F(ErrorTests, ToStringConversion)
{
    // Test 1: every audio_format has a name
    {
        EXPECT_EQ(to_string(audio_format::wav), "WAV");
        EXPECT_EQ(to_string(audio_format::ogg), "OGG");
        EXPECT_EQ(to_string(audio_format::mp3), "MP3");
        EXPECT_EQ(to_string(audio_format::unsupported), "Unsupported");
    }

    // Test 2: every audio_decoder_operation has a name
    {
        EXPECT_EQ(to_string(audio_decoder_operation::open_file), "Open File");
        EXPECT_EQ(to_string(audio_decoder_operation::decode_audio), "Decode Audio");
    }
}
