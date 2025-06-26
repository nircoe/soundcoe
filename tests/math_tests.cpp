#include <gtest/gtest.h>
#include <soundcoe/utils/math.hpp>
#include <soundcoe/core/types.hpp>
#include <cmath>
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
    
    void expectNear(float actual, float expected, float tolerance = 0.0001f)
    {
        EXPECT_NEAR(actual, expected, tolerance);
    }
};

//==============================================================================
//                        Vector Math Functions
//==============================================================================

TEST_F(MathTests, VectorDistance)
{
    Vec3 a(0.0f, 0.0f, 0.0f);
    Vec3 b(3.0f, 4.0f, 0.0f);
    
    expectNear(distance(a, b), 5.0f);
    expectNear(distance(b, a), 5.0f);
    expectNear(distance(a, a), 0.0f);
}

TEST_F(MathTests, VectorDistanceSquared)
{
    Vec3 a(0.0f, 0.0f, 0.0f);
    Vec3 b(3.0f, 4.0f, 0.0f);
    
    expectNear(distanceSquared(a, b), 25.0f);
    expectNear(distanceSquared(b, a), 25.0f);
    expectNear(distanceSquared(a, a), 0.0f);
}

TEST_F(MathTests, VectorNormalized)
{
    Vec3 v(3.0f, 4.0f, 0.0f);
    Vec3 norm = normalized(v);
    
    expectNear(length(norm), 1.0f);
    expectNear(norm.x, 0.6f);
    expectNear(norm.y, 0.8f);
    expectNear(norm.z, 0.0f);
    
    Vec3 zero = Vec3::zero();
    Vec3 normalizedZero = normalized(zero);
    EXPECT_EQ(normalizedZero.x, 0.0f);
    EXPECT_EQ(normalizedZero.y, 0.0f);
    EXPECT_EQ(normalizedZero.z, 0.0f);
}

TEST_F(MathTests, VectorLength)
{
    Vec3 v(3.0f, 4.0f, 0.0f);
    expectNear(length(v), 5.0f);
    
    Vec3 zero = Vec3::zero();
    expectNear(length(zero), 0.0f);
}

TEST_F(MathTests, VectorLengthSquared)
{
    Vec3 v(3.0f, 4.0f, 0.0f);
    expectNear(lengthSquared(v), 25.0f);
    
    Vec3 zero = Vec3::zero();
    expectNear(lengthSquared(zero), 0.0f);
}

TEST_F(MathTests, VectorDot)
{
    Vec3 a(1.0f, 2.0f, 3.0f);
    Vec3 b(4.0f, 5.0f, 6.0f);
    
    expectNear(dot(a, b), 32.0f);
    
    Vec3 x(1.0f, 0.0f, 0.0f);
    Vec3 y(0.0f, 1.0f, 0.0f);
    expectNear(dot(x, y), 0.0f);
}

TEST_F(MathTests, VectorCross)
{
    Vec3 x(1.0f, 0.0f, 0.0f);
    Vec3 y(0.0f, 1.0f, 0.0f);
    Vec3 result = cross(x, y);
    
    expectNear(result.x, 0.0f);
    expectNear(result.y, 0.0f);
    expectNear(result.z, 1.0f);
    
    Vec3 parallel1(1.0f, 2.0f, 3.0f);
    Vec3 parallel2(2.0f, 4.0f, 6.0f);
    Vec3 crossParallel = cross(parallel1, parallel2);
    expectNear(length(crossParallel), 0.0f);
}

//==============================================================================
//                        Decibel Conversions
//==============================================================================

TEST_F(MathTests, DbToLinear)
{
    expectNear(dbToLinear(-6.0f), 0.5f, 0.01f);
    expectNear(dbToLinear(0.0f), 1.0f);
    expectNear(dbToLinear(-20.0f), 0.1f, 0.01f);
    expectNear(dbToLinear(6.0f), 2.0f, 0.01f);
}

TEST_F(MathTests, LinearToDb)
{
    expectNear(linearToDb(0.5f), -6.0f, 0.1f);
    expectNear(linearToDb(1.0f), 0.0f);
    expectNear(linearToDb(0.1f), -20.0f, 0.1f);
    expectNear(linearToDb(2.0f), 6.0f, 0.1f);
    
    EXPECT_EQ(linearToDb(0.0f), -std::numeric_limits<float>::infinity());
    EXPECT_EQ(linearToDb(-1.0f), -std::numeric_limits<float>::infinity());
}

TEST_F(MathTests, DbToGain)
{
    expectNear(dbToGain(0.0f), 1.0f);
    expectNear(dbToGain(-3.0f), 0.5f, 0.01f);
    expectNear(dbToGain(3.0f), 2.0f, 0.01f);
}

TEST_F(MathTests, GainToDb)
{
    expectNear(gainToDb(1.0f), 0.0f);
    expectNear(gainToDb(0.5f), -3.0f, 0.1f);
    expectNear(gainToDb(2.0f), 3.0f, 0.1f);
    
    EXPECT_EQ(gainToDb(0.0f), -std::numeric_limits<float>::infinity());
    EXPECT_EQ(gainToDb(-1.0f), -std::numeric_limits<float>::infinity());
}

//==============================================================================
//                        Sample/Time Conversions
//==============================================================================

TEST_F(MathTests, SamplesToTime)
{
    expectNear(samplesToTime(44100, 44100), 1.0f);
    expectNear(samplesToTime(22050, 44100), 0.5f);
    expectNear(samplesToTime(1000, 0), 0.0f);
}

TEST_F(MathTests, TimeToSamples)
{
    EXPECT_EQ(timeToSamples(1.0f, 44100), 44100);
    EXPECT_EQ(timeToSamples(0.5f, 44100), 22050);
    EXPECT_EQ(timeToSamples(-1.0f, 44100), 0);
}

//==============================================================================
//                        General Math Functions
//==============================================================================

TEST_F(MathTests, Lerp)
{
    expectNear(lerp(0.0f, 10.0f, 0.0f), 0.0f);
    expectNear(lerp(0.0f, 10.0f, 1.0f), 10.0f);
    expectNear(lerp(0.0f, 10.0f, 0.5f), 5.0f);
    expectNear(lerp(0.0f, 10.0f, 0.25f), 2.5f);
}

TEST_F(MathTests, Clamp)
{
    expectNear(clamp(0.5f, 0.0f, 1.0f), 0.5f);
    expectNear(clamp(-0.5f, 0.0f, 1.0f), 0.0f);
    expectNear(clamp(1.5f, 0.0f, 1.0f), 1.0f);
    
    expectNear(clamp(15.0f, 10.0f, 20.0f), 15.0f);
    expectNear(clamp(5.0f, 10.0f, 20.0f), 10.0f);
    expectNear(clamp(25.0f, 10.0f, 20.0f), 20.0f);
}

TEST_F(MathTests, Smoothstep)
{
    expectNear(smoothstep(0.0f, 1.0f, 0.0f), 0.0f);
    expectNear(smoothstep(0.0f, 1.0f, 1.0f), 1.0f);
    expectNear(smoothstep(0.0f, 1.0f, 0.5f), 0.5f);
    
    float linear25 = 0.25f;
    float smooth25 = smoothstep(0.0f, 1.0f, 0.25f);
    EXPECT_LT(smooth25, linear25);
}

TEST_F(MathTests, ExponentialFade)
{
    expectNear(exponentialFade(0.0f), 0.0f);
    expectNear(exponentialFade(1.0f), 1.0f);
    
    expectNear(exponentialFade(0.5f, 1.0f), 0.5f);
    EXPECT_LT(exponentialFade(0.5f, 2.0f), 0.5f);
}

//==============================================================================
//                        Audio-Specific Functions
//==============================================================================

TEST_F(MathTests, VolumeByDistance)
{
    expectNear(calculateVolumeByDistance(0.0f, 100.0f), 1.0f);
    expectNear(calculateVolumeByDistance(100.0f, 100.0f), 0.0f);
    expectNear(calculateVolumeByDistance(50.0f, 100.0f, 1.0f), 0.5f);
    expectNear(calculateVolumeByDistance(150.0f, 100.0f), 0.0f);
    expectNear(calculateVolumeByDistance(50.0f, 0.0f), 0.0f);
    expectNear(calculateVolumeByDistance(50.0f, -10.0f), 0.0f);
    expectNear(calculateVolumeByDistance(-50.0f, 100.0f, 1.0f), 0.5f);
}

TEST_F(MathTests, PanCalculation)
{
    Vec3 listener(0.0f, 0.0f, 0.0f);
    Vec3 forward(0.0f, 0.0f, -1.0f);
    
    Vec3 rightSource(1.0f, 0.0f, 0.0f);
    float rightPan = calculatePan(listener, rightSource, forward);
    EXPECT_GT(rightPan, 0.0f);
    EXPECT_LE(rightPan, 1.0f);
    
    Vec3 leftSource(-1.0f, 0.0f, 0.0f);
    float leftPan = calculatePan(listener, leftSource, forward);
    EXPECT_LT(leftPan, 0.0f);
    EXPECT_GE(leftPan, -1.0f);
    
    Vec3 frontSource(0.0f, 0.0f, -1.0f);
    float frontPan = calculatePan(listener, frontSource, forward);
    expectNear(frontPan, 0.0f, 0.1f);
}

//==============================================================================
//                        Pitch/Musical Math
//==============================================================================

TEST_F(MathTests, SemitonesToRatio)
{
    expectNear(semitonesToRatio(12.0f), 2.0f, 0.01f);
    expectNear(semitonesToRatio(0.0f), 1.0f);
    expectNear(semitonesToRatio(-12.0f), 0.5f, 0.01f);
    expectNear(semitonesToRatio(24.0f), 4.0f, 0.01f);
    expectNear(semitonesToRatio(7.0f), 1.498f, 0.01f);
}

TEST_F(MathTests, RatioToSemitones)
{
    expectNear(ratioToSemitones(2.0f), 12.0f, 0.01f);
    expectNear(ratioToSemitones(1.0f), 0.0f);
    expectNear(ratioToSemitones(0.5f), -12.0f, 0.01f);
    expectNear(ratioToSemitones(4.0f), 24.0f, 0.01f);
    expectNear(ratioToSemitones(0.0f), 0.0f);
    expectNear(ratioToSemitones(-1.0f), 0.0f);
}

//==============================================================================
//                        Edge Cases and Error Conditions
//==============================================================================

TEST_F(MathTests, EdgeCases)
{
    expectNear(dbToLinear(-100.0f), 0.00001f, 0.000001f);
    EXPECT_GT(dbToLinear(100.0f), 10000.0f);
    
    Vec3 veryFar(1000000.0f, 1000000.0f, 1000000.0f);
    Vec3 origin = Vec3::zero();
    EXPECT_GT(distance(origin, veryFar), 1000000.0f);
}