#include <gtest/gtest.h>
#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <soundcoe/core/types.hpp>
#include <thread>
#include <chrono>

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265f
#endif

using namespace soundcoe;

//==============================================================================
//       AudioContextTests - AudioContext singleton and initialization tests
//==============================================================================

class AudioContextTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        try
        {
            AudioContext::getInstance().shutdown();
        }
        catch (...)
        {
        }
    }

    void TearDown() override
    {
        try
        {
            AudioContext::getInstance().shutdown();
        }
        catch (...)
        {
        }
    }
};

TEST_F(AudioContextTests, SingletonBehavior)
{
    AudioContext &context1 = AudioContext::getInstance();
    AudioContext &context2 = AudioContext::getInstance();

    EXPECT_EQ(&context1, &context2);
}

TEST_F(AudioContextTests, AutoInitialization)
{
    AudioContext &context = AudioContext::getInstance();

    EXPECT_TRUE(context.isInitialized());

    EXPECT_NE(context.getDevice(), nullptr);
    EXPECT_NE(context.getContext(), nullptr);
}

TEST_F(AudioContextTests, InitializedState)
{
    AudioContext &context = AudioContext::getInstance();

    EXPECT_TRUE(context.isInitialized());

    EXPECT_EQ(&context, &AudioContext::getInstance());
}

TEST_F(AudioContextTests, MultipleShutdownCalls)
{
    AudioContext &context = AudioContext::getInstance();

    EXPECT_NO_THROW(context.shutdown());
    EXPECT_NO_THROW(context.shutdown());
    EXPECT_NO_THROW(context.shutdown());

    EXPECT_FALSE(context.isInitialized());
}

TEST_F(AudioContextTests, ThreadSafety)
{
    std::vector<AudioContext *> contexts;
    std::vector<std::thread> threads;
    const int numThreads = 4;

    contexts.resize(numThreads);

    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([&contexts, i]()
                             {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            contexts[i] = &AudioContext::getInstance(); });
    }

    for (auto &thread : threads)
        thread.join();

    for (int i = 1; i < numThreads; ++i)
        EXPECT_EQ(contexts[0], contexts[i]);
}

//==============================================================================
//          ErrorHandlerTests - ErrorHandler functionality tests
//==============================================================================

TEST(ErrorHandlerTests, ALErrorStringConversion)
{
    EXPECT_EQ(ErrorHandler::getOpenALErrorAsString(AL_NO_ERROR), "AL_NO_ERROR");
    EXPECT_EQ(ErrorHandler::getOpenALErrorAsString(AL_INVALID_NAME), "AL_INVALID_NAME");
    EXPECT_EQ(ErrorHandler::getOpenALErrorAsString(AL_INVALID_ENUM), "AL_INVALID_ENUM");
    EXPECT_EQ(ErrorHandler::getOpenALErrorAsString(AL_INVALID_VALUE), "AL_INVALID_VALUE");
    EXPECT_EQ(ErrorHandler::getOpenALErrorAsString(AL_INVALID_OPERATION), "AL_INVALID_OPERATION");
    EXPECT_EQ(ErrorHandler::getOpenALErrorAsString(AL_OUT_OF_MEMORY), "AL_OUT_OF_MEMORY");
    EXPECT_EQ(ErrorHandler::getOpenALErrorAsString(static_cast<ALenum>(9999)), "UNKNOWN ERROR");
}

TEST(ErrorHandlerTests, ALCErrorStringConversion)
{
    EXPECT_EQ(ErrorHandler::getALCErrorAsString(ALC_NO_ERROR), "ALC_NO_ERROR");
    EXPECT_EQ(ErrorHandler::getALCErrorAsString(ALC_INVALID_DEVICE), "ALC_INVALID_DEVICE");
    EXPECT_EQ(ErrorHandler::getALCErrorAsString(ALC_INVALID_CONTEXT), "ALC_INVALID_CONTEXT");
    EXPECT_EQ(ErrorHandler::getALCErrorAsString(ALC_INVALID_ENUM), "ALC_INVALID_ENUM");
    EXPECT_EQ(ErrorHandler::getALCErrorAsString(ALC_INVALID_VALUE), "ALC_INVALID_VALUE");
    EXPECT_EQ(ErrorHandler::getALCErrorAsString(ALC_OUT_OF_MEMORY), "ALC_OUT_OF_MEMORY");
    EXPECT_EQ(ErrorHandler::getALCErrorAsString(static_cast<ALCenum>(9999)), "UNKNOWN ERROR");
}

TEST(ErrorHandlerTests, CheckErrorFunctions)
{
    EXPECT_NO_THROW(ErrorHandler::checkOpenALError("Test Operation"));
    EXPECT_NO_THROW(ErrorHandler::checkALCError(nullptr, "Test Operation"));
}

TEST(ErrorHandlerTests, ClearErrorFunctions)
{
    EXPECT_NO_THROW(ErrorHandler::clearOpenALError());
    EXPECT_NO_THROW(ErrorHandler::clearALCError(nullptr));
}

//==============================================================================
//                    Vec3Tests - Vec3 math operations tests
//==============================================================================

TEST(Vec3Tests, Construction)
{
    Vec3 v1;
    EXPECT_EQ(v1.x, 0.0f);
    EXPECT_EQ(v1.y, 0.0f);
    EXPECT_EQ(v1.z, 0.0f);

    Vec3 v2(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(v2.x, 1.0f);
    EXPECT_EQ(v2.y, 2.0f);
    EXPECT_EQ(v2.z, 3.0f);
}

TEST(Vec3Tests, StaticZero)
{
    Vec3 zero = Vec3::zero();
    EXPECT_EQ(zero.x, 0.0f);
    EXPECT_EQ(zero.y, 0.0f);
    EXPECT_EQ(zero.z, 0.0f);
}

TEST(Vec3Tests, Addition)
{
    Vec3 v1(1.0f, 2.0f, 3.0f);
    Vec3 v2(4.0f, 5.0f, 6.0f);
    Vec3 result = v1 + v2;

    EXPECT_EQ(result.x, 5.0f);
    EXPECT_EQ(result.y, 7.0f);
    EXPECT_EQ(result.z, 9.0f);
}

TEST(Vec3Tests, Subtraction)
{
    Vec3 v1(5.0f, 7.0f, 9.0f);
    Vec3 v2(1.0f, 2.0f, 3.0f);
    Vec3 result = v1 - v2;

    EXPECT_EQ(result.x, 4.0f);
    EXPECT_EQ(result.y, 5.0f);
    EXPECT_EQ(result.z, 6.0f);
}

TEST(Vec3Tests, ScalarMultiplication)
{
    Vec3 v(2.0f, 3.0f, 4.0f);
    Vec3 result = v * 2.0f;

    EXPECT_EQ(result.x, 4.0f);
    EXPECT_EQ(result.y, 6.0f);
    EXPECT_EQ(result.z, 8.0f);
}

TEST(Vec3Tests, Equality)
{
    Vec3 v1(1.0f, 2.0f, 3.0f);
    Vec3 v2(1.0f, 2.0f, 3.0f);
    Vec3 v3(1.0f, 2.0f, 4.0f);

    EXPECT_TRUE(v1 == v2);
    EXPECT_FALSE(v1 == v3);
    EXPECT_TRUE(v1 != v3);
    EXPECT_FALSE(v1 != v2);
}

TEST(Vec3Tests, CompoundAssignment)
{
    Vec3 v1(1.0f, 2.0f, 3.0f);
    Vec3 v2(4.0f, 5.0f, 6.0f);

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
    Vec3 v(3.0f, 4.0f, 0.0f);
    EXPECT_FLOAT_EQ(v.length(), 5.0f);

    Vec3 zero = Vec3::zero();
    EXPECT_FLOAT_EQ(zero.length(), 0.0f);
}

TEST(Vec3Tests, Normalization)
{
    Vec3 v(3.0f, 4.0f, 0.0f);
    Vec3 normalized = v.normalized();

    EXPECT_FLOAT_EQ(normalized.length(), 1.0f);
    EXPECT_FLOAT_EQ(normalized.x, 0.6f);
    EXPECT_FLOAT_EQ(normalized.y, 0.8f);
    EXPECT_FLOAT_EQ(normalized.z, 0.0f);

    Vec3 zero = Vec3::zero();
    Vec3 normalizedZero = zero.normalized();
    EXPECT_EQ(normalizedZero.x, 0.0f);
    EXPECT_EQ(normalizedZero.y, 0.0f);
    EXPECT_EQ(normalizedZero.z, 0.0f);
}

TEST(Vec3Tests, NormalizeInPlace)
{
    Vec3 v(3.0f, 4.0f, 0.0f);
    v.normalize();

    EXPECT_FLOAT_EQ(v.length(), 1.0f);
    EXPECT_FLOAT_EQ(v.x, 0.6f);
    EXPECT_FLOAT_EQ(v.y, 0.8f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST(Vec3Tests, Distance)
{
    Vec3 v1(0.0f, 0.0f, 0.0f);
    Vec3 v2(3.0f, 4.0f, 0.0f);

    EXPECT_FLOAT_EQ(v1.distanceTo(v2), 5.0f);
    EXPECT_FLOAT_EQ(v2.distanceTo(v1), 5.0f);
}

TEST(Vec3Tests, DotProduct)
{
    Vec3 v1(1.0f, 2.0f, 3.0f);
    Vec3 v2(4.0f, 5.0f, 6.0f);

    float dot = v1.dotProduct(v2);
    EXPECT_FLOAT_EQ(dot, 32.0f);

    Vec3 perpendicular1(1.0f, 0.0f, 0.0f);
    Vec3 perpendicular2(0.0f, 1.0f, 0.0f);
    EXPECT_FLOAT_EQ(perpendicular1.dotProduct(perpendicular2), 0.0f);
}

TEST(Vec3Tests, CrossProduct)
{
    Vec3 v1(1.0f, 0.0f, 0.0f);
    Vec3 v2(0.0f, 1.0f, 0.0f);
    Vec3 cross = v1.crossProduct(v2);

    EXPECT_FLOAT_EQ(cross.x, 0.0f);
    EXPECT_FLOAT_EQ(cross.y, 0.0f);
    EXPECT_FLOAT_EQ(cross.z, 1.0f);

    Vec3 parallel1(1.0f, 2.0f, 3.0f);
    Vec3 parallel2(2.0f, 4.0f, 6.0f);
    Vec3 crossParallel = parallel1.crossProduct(parallel2);
    EXPECT_FLOAT_EQ(crossParallel.length(), 0.0f);
}

TEST(Vec3Tests, Lerp)
{
    Vec3 start(0.0f, 0.0f, 0.0f);
    Vec3 end(10.0f, 20.0f, 30.0f);

    Vec3 quarter = start.lerp(end, 0.25f);
    EXPECT_FLOAT_EQ(quarter.x, 2.5f);
    EXPECT_FLOAT_EQ(quarter.y, 5.0f);
    EXPECT_FLOAT_EQ(quarter.z, 7.5f);

    Vec3 half = start.lerp(end, 0.5f);
    EXPECT_FLOAT_EQ(half.x, 5.0f);
    EXPECT_FLOAT_EQ(half.y, 10.0f);
    EXPECT_FLOAT_EQ(half.z, 15.0f);

    Vec3 atStart = start.lerp(end, 0.0f);
    EXPECT_TRUE(atStart == start);

    Vec3 atEnd = start.lerp(end, 1.0f);
    EXPECT_TRUE(atEnd == end);
}

TEST(Vec3Tests, AngleTo)
{
    Vec3 v1(1.0f, 0.0f, 0.0f);
    Vec3 v2(0.0f, 1.0f, 0.0f);

    float angle = v1.angleTo(v2);
    EXPECT_FLOAT_EQ(angle, M_PI / 2.0f);

    Vec3 same1(1.0f, 1.0f, 1.0f);
    Vec3 same2(2.0f, 2.0f, 2.0f);
    float sameAngle = same1.angleTo(same2);
    EXPECT_NEAR(sameAngle, 0.0f, 0.001f);
}

TEST(Vec3Tests, StaticMethods)
{
    Vec3 v1(1.0f, 2.0f, 3.0f);
    Vec3 v2(4.0f, 5.0f, 6.0f);

    float staticDot = Vec3::dotProduct(v1, v2);
    float instanceDot = v1.dotProduct(v2);
    EXPECT_FLOAT_EQ(staticDot, instanceDot);

    Vec3 staticCross = Vec3::crossProduct(v1, v2);
    Vec3 instanceCross = v1.crossProduct(v2);
    EXPECT_TRUE(staticCross == instanceCross);

    float staticDistance = Vec3::distanceTo(v1, v2);
    float instanceDistance = v1.distanceTo(v2);
    EXPECT_FLOAT_EQ(staticDistance, instanceDistance);

    Vec3 staticLerp = Vec3::lerp(v1, v2, 0.5f);
    Vec3 instanceLerp = v1.lerp(v2, 0.5f);
    EXPECT_TRUE(staticLerp == instanceLerp);
}