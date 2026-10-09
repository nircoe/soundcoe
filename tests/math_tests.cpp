#include <gtest/gtest.h>
#include <soundcoe/utils/math.hpp>
#include <soundcoe/core/types.hpp>
#include <limits>

#define _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI 3.14159265f
#endif

using namespace soundcoe;
using namespace soundcoe::math;

//==============================================================================
//                    MathTests - Math utility functions tests
//==============================================================================

class MathTests : public ::testing::Test
{
protected:
    const float EPSILON = 0.0001f;

    void expect_near(float actual, float expected, float tolerance = 0.0001f)
    {
        EXPECT_NEAR(actual, expected, tolerance);
    }
};

//==============================================================================
//                        Vector Math Functions
//==============================================================================

TEST_F(MathTests, VectorDistance)
{
    vec3 a(0.0f, 0.0f, 0.0f);
    vec3 b(3.0f, 4.0f, 0.0f);

    expect_near(distance(a, b), 5.0f);
    expect_near(distance(b, a), 5.0f);
    expect_near(distance(a, a), 0.0f);
}

TEST_F(MathTests, VectorDistanceSquared)
{
    vec3 a(0.0f, 0.0f, 0.0f);
    vec3 b(3.0f, 4.0f, 0.0f);

    expect_near(distance_squared(a, b), 25.0f);
    expect_near(distance_squared(b, a), 25.0f);
    expect_near(distance_squared(a, a), 0.0f);
}

TEST_F(MathTests, VectorNormalized)
{
    vec3 v(3.0f, 4.0f, 0.0f);
    vec3 norm = normalized(v);

    expect_near(length(norm), 1.0f);
    expect_near(norm.x, 0.6f);
    expect_near(norm.y, 0.8f);
    expect_near(norm.z, 0.0f);

    vec3 zero = vec3::zero();
    vec3 normalized_zero = normalized(zero);
    EXPECT_EQ(normalized_zero.x, 0.0f);
    EXPECT_EQ(normalized_zero.y, 0.0f);
    EXPECT_EQ(normalized_zero.z, 0.0f);
}

TEST_F(MathTests, VectorLength)
{
    vec3 v(3.0f, 4.0f, 0.0f);
    expect_near(length(v), 5.0f);

    vec3 zero = vec3::zero();
    expect_near(length(zero), 0.0f);
}

TEST_F(MathTests, VectorLengthSquared)
{
    vec3 v(3.0f, 4.0f, 0.0f);
    expect_near(length_squared(v), 25.0f);

    vec3 zero = vec3::zero();
    expect_near(length_squared(zero), 0.0f);
}

TEST_F(MathTests, VectorDot)
{
    vec3 a(1.0f, 2.0f, 3.0f);
    vec3 b(4.0f, 5.0f, 6.0f);

    expect_near(dot(a, b), 32.0f);

    vec3 x(1.0f, 0.0f, 0.0f);
    vec3 y(0.0f, 1.0f, 0.0f);
    expect_near(dot(x, y), 0.0f);
}

TEST_F(MathTests, VectorCross)
{
    vec3 x(1.0f, 0.0f, 0.0f);
    vec3 y(0.0f, 1.0f, 0.0f);
    vec3 result = cross(x, y);

    expect_near(result.x, 0.0f);
    expect_near(result.y, 0.0f);
    expect_near(result.z, 1.0f);

    vec3 parallel1(1.0f, 2.0f, 3.0f);
    vec3 parallel2(2.0f, 4.0f, 6.0f);
    vec3 cross_parallel = cross(parallel1, parallel2);
    expect_near(length(cross_parallel), 0.0f);
}

//==============================================================================
//                        Decibel Conversions
//==============================================================================

TEST_F(MathTests, DbToLinear)
{
    expect_near(db_to_linear(-6.0f), 0.5f, 0.01f);
    expect_near(db_to_linear(0.0f), 1.0f);
    expect_near(db_to_linear(-20.0f), 0.1f, 0.01f);
    expect_near(db_to_linear(6.0f), 2.0f, 0.01f);
}

TEST_F(MathTests, LinearToDb)
{
    expect_near(linear_to_db(0.5f), -6.0f, 0.1f);
    expect_near(linear_to_db(1.0f), 0.0f);
    expect_near(linear_to_db(0.1f), -20.0f, 0.1f);
    expect_near(linear_to_db(2.0f), 6.0f, 0.1f);

    EXPECT_EQ(linear_to_db(0.0f), -std::numeric_limits<float>::infinity());
    EXPECT_EQ(linear_to_db(-1.0f), -std::numeric_limits<float>::infinity());
}

TEST_F(MathTests, DbToGain)
{
    expect_near(db_to_gain(0.0f), 1.0f);
    expect_near(db_to_gain(-3.0f), 0.5f, 0.01f);
    expect_near(db_to_gain(3.0f), 2.0f, 0.01f);
}

TEST_F(MathTests, GainToDb)
{
    expect_near(gain_to_db(1.0f), 0.0f);
    expect_near(gain_to_db(0.5f), -3.0f, 0.1f);
    expect_near(gain_to_db(2.0f), 3.0f, 0.1f);

    EXPECT_EQ(gain_to_db(0.0f), -std::numeric_limits<float>::infinity());
    EXPECT_EQ(gain_to_db(-1.0f), -std::numeric_limits<float>::infinity());
}

//==============================================================================
//                        Sample/Time Conversions
//==============================================================================

TEST_F(MathTests, SamplesToTime)
{
    expect_near(samples_to_time(44100, 44100), 1.0f);
    expect_near(samples_to_time(22050, 44100), 0.5f);
    expect_near(samples_to_time(1000, 0), 0.0f);
}

TEST_F(MathTests, TimeToSamples)
{
    EXPECT_EQ(time_to_samples(1.0f, 44100), 44100);
    EXPECT_EQ(time_to_samples(0.5f, 44100), 22050);
    EXPECT_EQ(time_to_samples(-1.0f, 44100), 0);
}

//==============================================================================
//                        General Math Functions
//==============================================================================

TEST_F(MathTests, Lerp)
{
    expect_near(lerp(0.0f, 10.0f, 0.0f), 0.0f);
    expect_near(lerp(0.0f, 10.0f, 1.0f), 10.0f);
    expect_near(lerp(0.0f, 10.0f, 0.5f), 5.0f);
    expect_near(lerp(0.0f, 10.0f, 0.25f), 2.5f);
}

TEST_F(MathTests, Clamp)
{
    expect_near(clamp(0.5f, 0.0f, 1.0f), 0.5f);
    expect_near(clamp(-0.5f, 0.0f, 1.0f), 0.0f);
    expect_near(clamp(1.5f, 0.0f, 1.0f), 1.0f);

    expect_near(clamp(15.0f, 10.0f, 20.0f), 15.0f);
    expect_near(clamp(5.0f, 10.0f, 20.0f), 10.0f);
    expect_near(clamp(25.0f, 10.0f, 20.0f), 20.0f);
}

TEST_F(MathTests, Smoothstep)
{
    expect_near(smoothstep(0.0f, 1.0f, 0.0f), 0.0f);
    expect_near(smoothstep(0.0f, 1.0f, 1.0f), 1.0f);
    expect_near(smoothstep(0.0f, 1.0f, 0.5f), 0.5f);

    float linear_25 = 0.25f;
    float smooth_25 = smoothstep(0.0f, 1.0f, 0.25f);
    EXPECT_LT(smooth_25, linear_25);
}

TEST_F(MathTests, ExponentialFade)
{
    expect_near(exponential_fade(0.0f), 0.0f);
    expect_near(exponential_fade(1.0f), 1.0f);

    expect_near(exponential_fade(0.5f, 1.0f), 0.5f);
    EXPECT_LT(exponential_fade(0.5f, 2.0f), 0.5f);
}

//==============================================================================
//                        Audio-Specific Functions
//==============================================================================

TEST_F(MathTests, VolumeByDistance)
{
    expect_near(calculate_volume_by_distance(0.0f, 100.0f), 1.0f);
    expect_near(calculate_volume_by_distance(100.0f, 100.0f), 0.0f);
    expect_near(calculate_volume_by_distance(50.0f, 100.0f, 1.0f), 0.5f);
    expect_near(calculate_volume_by_distance(150.0f, 100.0f), 0.0f);
    expect_near(calculate_volume_by_distance(50.0f, 0.0f), 0.0f);
    expect_near(calculate_volume_by_distance(50.0f, -10.0f), 0.0f);
    expect_near(calculate_volume_by_distance(-50.0f, 100.0f, 1.0f), 0.5f);
}

TEST_F(MathTests, PanCalculation)
{
    vec3 listener(0.0f, 0.0f, 0.0f);
    vec3 forward(0.0f, 0.0f, -1.0f);

    vec3 right_source(1.0f, 0.0f, 0.0f);
    float right_pan = calculate_pan(listener, right_source, forward);
    EXPECT_GT(right_pan, 0.0f);
    EXPECT_LE(right_pan, 1.0f);

    vec3 left_source(-1.0f, 0.0f, 0.0f);
    float left_pan = calculate_pan(listener, left_source, forward);
    EXPECT_LT(left_pan, 0.0f);
    EXPECT_GE(left_pan, -1.0f);

    vec3 front_source(0.0f, 0.0f, -1.0f);
    float front_pan = calculate_pan(listener, front_source, forward);
    expect_near(front_pan, 0.0f, 0.1f);
}

//==============================================================================
//                        Pitch/Musical Math
//==============================================================================

TEST_F(MathTests, SemitonesToRatio)
{
    expect_near(semitones_to_ratio(12.0f), 2.0f, 0.01f);
    expect_near(semitones_to_ratio(0.0f), 1.0f);
    expect_near(semitones_to_ratio(-12.0f), 0.5f, 0.01f);
    expect_near(semitones_to_ratio(24.0f), 4.0f, 0.01f);
    expect_near(semitones_to_ratio(7.0f), 1.498f, 0.01f);
}

TEST_F(MathTests, RatioToSemitones)
{
    expect_near(ratio_to_semitones(2.0f), 12.0f, 0.01f);
    expect_near(ratio_to_semitones(1.0f), 0.0f);
    expect_near(ratio_to_semitones(0.5f), -12.0f, 0.01f);
    expect_near(ratio_to_semitones(4.0f), 24.0f, 0.01f);
    expect_near(ratio_to_semitones(0.0f), 0.0f);
    expect_near(ratio_to_semitones(-1.0f), 0.0f);
}

//==============================================================================
//                        Edge Cases and Error Conditions
//==============================================================================

TEST_F(MathTests, EdgeCases)
{
    expect_near(db_to_linear(-100.0f), 0.00001f, 0.000001f);
    EXPECT_GT(db_to_linear(100.0f), 10000.0f);

    vec3 very_far(1000000.0f, 1000000.0f, 1000000.0f);
    vec3 origin = vec3::zero();
    EXPECT_GT(distance(origin, very_far), 1000000.0f);
}
