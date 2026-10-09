#include <gtest/gtest.h>
#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <soundcoe/core/types.hpp>
#include <AL/al.h>
#include <AL/alc.h>
#include <thread>
#include <chrono>
#include <future>
#include <expected>
#include <string>
#include <vector>

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265f
#endif

using namespace soundcoe;
using namespace soundcoe::internal;

//==============================================================================
//       AudioContextTests - audio_context singleton and initialization tests
//==============================================================================

class AudioContextTests : public ::testing::Test
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

TEST_F(AudioContextTests, AutoInitialization)
{
    EXPECT_TRUE(m_audio_context.is_initialized());

    EXPECT_NE(m_audio_context.get_device(), nullptr);
    EXPECT_NE(m_audio_context.get_context(), nullptr);
}

TEST_F(AudioContextTests, MultipleShutdownCalls)
{
    EXPECT_TRUE(m_audio_context.shutdown());
    EXPECT_TRUE(m_audio_context.shutdown());
    EXPECT_TRUE(m_audio_context.shutdown());

    EXPECT_FALSE(m_audio_context.is_initialized());
}

TEST_F(AudioContextTests, ThreadSafety)
{
    const int num_threads = 4;
    std::vector<std::future<bool>> futures;
    
    for (int i = 0; i < num_threads; ++i)
    {
        futures.push_back(std::async(std::launch::async, [&]() {
            bool all_succeeded = true;
            
            for (int j = 0; j < 10; ++j)
            {
                all_succeeded &= m_audio_context.is_initialized();
                all_succeeded &= (m_audio_context.get_device() != nullptr);
                all_succeeded &= (m_audio_context.get_context() != nullptr);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            
            return all_succeeded;
        }));
    }
    
    for (auto& future : futures)
    {
        EXPECT_TRUE(future.get());
    }
}

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

TEST_F(ErrorHandlerTests, ALErrorStringConversion)
{
    EXPECT_EQ(error_handler::get_openal_error_as_string(AL_NO_ERROR), "AL_NO_ERROR");
    EXPECT_EQ(error_handler::get_openal_error_as_string(AL_INVALID_NAME), "AL_INVALID_NAME");
    EXPECT_EQ(error_handler::get_openal_error_as_string(AL_INVALID_ENUM), "AL_INVALID_ENUM");
    EXPECT_EQ(error_handler::get_openal_error_as_string(AL_INVALID_VALUE), "AL_INVALID_VALUE");
    EXPECT_EQ(error_handler::get_openal_error_as_string(AL_INVALID_OPERATION), "AL_INVALID_OPERATION");
    EXPECT_EQ(error_handler::get_openal_error_as_string(AL_OUT_OF_MEMORY), "AL_OUT_OF_MEMORY");
    EXPECT_EQ(error_handler::get_openal_error_as_string(static_cast<ALenum>(9999)), "UNKNOWN ERROR");
}

TEST_F(ErrorHandlerTests, ALCErrorStringConversion)
{
    EXPECT_EQ(error_handler::get_alc_error_as_string(ALC_NO_ERROR), "ALC_NO_ERROR");
    EXPECT_EQ(error_handler::get_alc_error_as_string(ALC_INVALID_DEVICE), "ALC_INVALID_DEVICE");
    EXPECT_EQ(error_handler::get_alc_error_as_string(ALC_INVALID_CONTEXT), "ALC_INVALID_CONTEXT");
    EXPECT_EQ(error_handler::get_alc_error_as_string(ALC_INVALID_ENUM), "ALC_INVALID_ENUM");
    EXPECT_EQ(error_handler::get_alc_error_as_string(ALC_INVALID_VALUE), "ALC_INVALID_VALUE");
    EXPECT_EQ(error_handler::get_alc_error_as_string(ALC_OUT_OF_MEMORY), "ALC_OUT_OF_MEMORY");
    EXPECT_EQ(error_handler::get_alc_error_as_string(static_cast<ALCenum>(9999)), "UNKNOWN ERROR");
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

//==============================================================================
//                    ErrorTests - error and to_string tests
//==============================================================================

TEST(ErrorTests, ErrorConstruction)
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

TEST(ErrorTests, ToStringConversion)
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

//==============================================================================
//                    Vec3Tests - vec3 math operations tests
//==============================================================================

TEST(Vec3Tests, Construction)
{
    vec3 v1;
    EXPECT_EQ(v1.x, 0.0f);
    EXPECT_EQ(v1.y, 0.0f);
    EXPECT_EQ(v1.z, 0.0f);

    vec3 v2(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(v2.x, 1.0f);
    EXPECT_EQ(v2.y, 2.0f);
    EXPECT_EQ(v2.z, 3.0f);
}

TEST(Vec3Tests, StaticZero)
{
    vec3 zero = vec3::zero();
    EXPECT_EQ(zero.x, 0.0f);
    EXPECT_EQ(zero.y, 0.0f);
    EXPECT_EQ(zero.z, 0.0f);
}

TEST(Vec3Tests, Addition)
{
    vec3 v1(1.0f, 2.0f, 3.0f);
    vec3 v2(4.0f, 5.0f, 6.0f);
    vec3 result = v1 + v2;

    EXPECT_EQ(result.x, 5.0f);
    EXPECT_EQ(result.y, 7.0f);
    EXPECT_EQ(result.z, 9.0f);
}

TEST(Vec3Tests, Subtraction)
{
    vec3 v1(5.0f, 7.0f, 9.0f);
    vec3 v2(1.0f, 2.0f, 3.0f);
    vec3 result = v1 - v2;

    EXPECT_EQ(result.x, 4.0f);
    EXPECT_EQ(result.y, 5.0f);
    EXPECT_EQ(result.z, 6.0f);
}

TEST(Vec3Tests, ScalarMultiplication)
{
    vec3 v(2.0f, 3.0f, 4.0f);
    vec3 result = v * 2.0f;

    EXPECT_EQ(result.x, 4.0f);
    EXPECT_EQ(result.y, 6.0f);
    EXPECT_EQ(result.z, 8.0f);
}

TEST(Vec3Tests, Equality)
{
    vec3 v1(1.0f, 2.0f, 3.0f);
    vec3 v2(1.0f, 2.0f, 3.0f);
    vec3 v3(1.0f, 2.0f, 4.0f);

    EXPECT_TRUE(v1 == v2);
    EXPECT_FALSE(v1 == v3);
    EXPECT_TRUE(v1 != v3);
    EXPECT_FALSE(v1 != v2);
}

TEST(Vec3Tests, CompoundAssignment)
{
    vec3 v1(1.0f, 2.0f, 3.0f);
    vec3 v2(4.0f, 5.0f, 6.0f);

    v1 += v2;
    EXPECT_EQ(v1.x, 5.0f);
    EXPECT_EQ(v1.y, 7.0f);
    EXPECT_EQ(v1.z, 9.0f);

    v1 -= v2;
    EXPECT_EQ(v1.x, 1.0f);
    EXPECT_EQ(v1.y, 2.0f);
    EXPECT_EQ(v1.z, 3.0f);

    v1 *= 2.0f;
    EXPECT_EQ(v1.x, 2.0f);
    EXPECT_EQ(v1.y, 4.0f);
    EXPECT_EQ(v1.z, 6.0f);
}

TEST(Vec3Tests, Length)
{
    vec3 v(3.0f, 4.0f, 0.0f);
    EXPECT_FLOAT_EQ(v.length(), 5.0f);

    vec3 zero = vec3::zero();
    EXPECT_FLOAT_EQ(zero.length(), 0.0f);
}

TEST(Vec3Tests, Normalization)
{
    vec3 v(3.0f, 4.0f, 0.0f);
    vec3 normalized = v.normalized();

    EXPECT_FLOAT_EQ(normalized.length(), 1.0f);
    EXPECT_FLOAT_EQ(normalized.x, 0.6f);
    EXPECT_FLOAT_EQ(normalized.y, 0.8f);
    EXPECT_FLOAT_EQ(normalized.z, 0.0f);

    vec3 zero = vec3::zero();
    vec3 normalized_zero = zero.normalized();
    EXPECT_EQ(normalized_zero.x, 0.0f);
    EXPECT_EQ(normalized_zero.y, 0.0f);
    EXPECT_EQ(normalized_zero.z, 0.0f);
}

TEST(Vec3Tests, NormalizeInPlace)
{
    vec3 v(3.0f, 4.0f, 0.0f);
    v.normalize();

    EXPECT_FLOAT_EQ(v.length(), 1.0f);
    EXPECT_FLOAT_EQ(v.x, 0.6f);
    EXPECT_FLOAT_EQ(v.y, 0.8f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST(Vec3Tests, Distance)
{
    vec3 v1(0.0f, 0.0f, 0.0f);
    vec3 v2(3.0f, 4.0f, 0.0f);

    EXPECT_FLOAT_EQ(v1.distance(v2), 5.0f);
    EXPECT_FLOAT_EQ(v2.distance(v1), 5.0f);
}

TEST(Vec3Tests, DotProduct)
{
    vec3 v1(1.0f, 2.0f, 3.0f);
    vec3 v2(4.0f, 5.0f, 6.0f);

    float dot = v1.dot(v2);
    EXPECT_FLOAT_EQ(dot, 32.0f);

    vec3 perpendicular1(1.0f, 0.0f, 0.0f);
    vec3 perpendicular2(0.0f, 1.0f, 0.0f);
    EXPECT_FLOAT_EQ(perpendicular1.dot(perpendicular2), 0.0f);
}

TEST(Vec3Tests, CrossProduct)
{
    vec3 v1(1.0f, 0.0f, 0.0f);
    vec3 v2(0.0f, 1.0f, 0.0f);
    vec3 cross = v1.cross(v2);

    EXPECT_FLOAT_EQ(cross.x, 0.0f);
    EXPECT_FLOAT_EQ(cross.y, 0.0f);
    EXPECT_FLOAT_EQ(cross.z, 1.0f);

    vec3 parallel1(1.0f, 2.0f, 3.0f);
    vec3 parallel2(2.0f, 4.0f, 6.0f);
    vec3 cross_parallel = parallel1.cross(parallel2);
    EXPECT_FLOAT_EQ(cross_parallel.length(), 0.0f);
}

TEST(Vec3Tests, Lerp)
{
    vec3 start(0.0f, 0.0f, 0.0f);
    vec3 end(10.0f, 20.0f, 30.0f);

    vec3 quarter = start.lerp(end, 0.25f);
    EXPECT_FLOAT_EQ(quarter.x, 2.5f);
    EXPECT_FLOAT_EQ(quarter.y, 5.0f);
    EXPECT_FLOAT_EQ(quarter.z, 7.5f);

    vec3 half = start.lerp(end, 0.5f);
    EXPECT_FLOAT_EQ(half.x, 5.0f);
    EXPECT_FLOAT_EQ(half.y, 10.0f);
    EXPECT_FLOAT_EQ(half.z, 15.0f);

    vec3 at_start = start.lerp(end, 0.0f);
    EXPECT_TRUE(at_start == start);

    vec3 at_end = start.lerp(end, 1.0f);
    EXPECT_TRUE(at_end == end);
}

TEST(Vec3Tests, AngleTo)
{
    vec3 v1(1.0f, 0.0f, 0.0f);
    vec3 v2(0.0f, 1.0f, 0.0f);

    float angle = v1.angle(v2);
    EXPECT_FLOAT_EQ(angle, M_PI / 2.0f);

    vec3 same1(1.0f, 1.0f, 1.0f);
    vec3 same2(2.0f, 2.0f, 2.0f);
    float same_angle = same1.angle(same2);
    EXPECT_NEAR(same_angle, 0.0f, 0.001f);
}

TEST(Vec3Tests, StaticMethods)
{
    vec3 v1(1.0f, 2.0f, 3.0f);
    vec3 v2(4.0f, 5.0f, 6.0f);

    float static_dot = vec3::dot(v1, v2);
    float instance_dot = v1.dot(v2);
    EXPECT_FLOAT_EQ(static_dot, instance_dot);

    vec3 static_cross = vec3::cross(v1, v2);
    vec3 instance_cross = v1.cross(v2);
    EXPECT_TRUE(static_cross == instance_cross);

    float static_distance = vec3::distance(v1, v2);
    float instance_distance = v1.distance(v2);
    EXPECT_FLOAT_EQ(static_distance, instance_distance);

    vec3 static_lerp = vec3::lerp(v1, v2, 0.5f);
    vec3 instance_lerp = v1.lerp(v2, 0.5f);
    EXPECT_TRUE(static_lerp == instance_lerp);
}
