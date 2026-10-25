#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <gtest/gtest.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <string>

using namespace soundcoe;
using namespace soundcoe::internal;

//==============================================================================
//          ErrorHandlerTests - error_handler functionality tests
//==============================================================================

class ErrorHandlerTests : public ::testing::Test
{
protected:
    audio_context m_audio_context;

    void SetUp() override
    {
        ASSERT_TRUE(m_audio_context.initialize());
    }

    void TearDown() override
    {
        static_cast<void>(m_audio_context.shutdown());
    }
};

TEST_F(ErrorHandlerTests, ErrorStrings)
{
    // Test 1: AL error codes map to their names
    {
        EXPECT_EQ(error_handler::get_openal_error_as_string(AL_NO_ERROR), "AL_NO_ERROR");
        EXPECT_EQ(error_handler::get_openal_error_as_string(AL_INVALID_NAME), "AL_INVALID_NAME");
        EXPECT_EQ(error_handler::get_openal_error_as_string(AL_INVALID_ENUM), "AL_INVALID_ENUM");
        EXPECT_EQ(error_handler::get_openal_error_as_string(AL_INVALID_VALUE), "AL_INVALID_VALUE");
        EXPECT_EQ(error_handler::get_openal_error_as_string(AL_INVALID_OPERATION), "AL_INVALID_OPERATION");
        EXPECT_EQ(error_handler::get_openal_error_as_string(AL_OUT_OF_MEMORY), "AL_OUT_OF_MEMORY");
        EXPECT_EQ(error_handler::get_openal_error_as_string(static_cast<ALenum>(9999)), "UNKNOWN ERROR");
    }

    // Test 2: ALC error codes map to their names
    {
        EXPECT_EQ(error_handler::get_alc_error_as_string(ALC_NO_ERROR), "ALC_NO_ERROR");
        EXPECT_EQ(error_handler::get_alc_error_as_string(ALC_INVALID_DEVICE), "ALC_INVALID_DEVICE");
        EXPECT_EQ(error_handler::get_alc_error_as_string(ALC_INVALID_CONTEXT), "ALC_INVALID_CONTEXT");
        EXPECT_EQ(error_handler::get_alc_error_as_string(ALC_INVALID_ENUM), "ALC_INVALID_ENUM");
        EXPECT_EQ(error_handler::get_alc_error_as_string(ALC_INVALID_VALUE), "ALC_INVALID_VALUE");
        EXPECT_EQ(error_handler::get_alc_error_as_string(ALC_OUT_OF_MEMORY), "ALC_OUT_OF_MEMORY");
        EXPECT_EQ(error_handler::get_alc_error_as_string(static_cast<ALCenum>(9999)), "UNKNOWN ERROR");
    }
}

TEST_F(ErrorHandlerTests, CheckOpenALError)
{
    // Test 1: no pending error is a success
    {
        error_handler::clear_openal_error();
        EXPECT_TRUE(error_handler::check_openal_error("Test Operation").has_value());
    }

    // Test 2: a bad call gives an openal_error with the AL error name in the message
    {
        ALint value = 0;
        alGetSourcei(0xDEADBEEF, AL_BUFFER, &value);

        auto r = error_handler::check_openal_error("Test Operation");
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::openal_error);
        EXPECT_NE(r.error().message.find("AL_INVALID_NAME"), std::string::npos);
        EXPECT_NE(r.error().message.find("Test Operation"), std::string::npos);
    }

    // Test 3: the failed check cleared the AL error, so the next check succeeds
    {
        EXPECT_TRUE(error_handler::check_openal_error("Test Operation").has_value());
    }
}

TEST_F(ErrorHandlerTests, CheckALCError)
{
    // Test 1: no pending error is a success
    {
        error_handler::clear_alc_error(m_audio_context.get_device());
        EXPECT_TRUE(error_handler::check_alc_error(m_audio_context.get_device(), "Test Operation").has_value());
    }

    // Test 2: a bad call gives an alc_error with the ALC error name in the message
    {
        alcGetIntegerv(m_audio_context.get_device(), 999999, 1, nullptr);

        auto r = error_handler::check_alc_error(m_audio_context.get_device(), "Test Operation");
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::alc_error);
        EXPECT_NE(r.error().message.find("ALC_INVALID_VALUE"), std::string::npos);
        EXPECT_NE(r.error().message.find("Test Operation"), std::string::npos);
    }
}

TEST_F(ErrorHandlerTests, ClearErrorFunctions)
{
    alSourcei(999999, AL_BUFFER, 0);
    ALenum al_error = error_handler::clear_openal_error();
    EXPECT_EQ(al_error, AL_INVALID_NAME);

    ALenum second_al_call = error_handler::clear_openal_error();
    EXPECT_EQ(second_al_call, AL_NO_ERROR);

    alcGetIntegerv(m_audio_context.get_device(), 999999, 1, nullptr);
    ALCenum alc_error = error_handler::clear_alc_error(m_audio_context.get_device());
    EXPECT_EQ(alc_error, ALC_INVALID_VALUE);

    ALCenum second_alc_call = error_handler::clear_alc_error(m_audio_context.get_device());
    EXPECT_EQ(second_alc_call, ALC_NO_ERROR);
}
