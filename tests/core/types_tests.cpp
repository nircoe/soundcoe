#include <soundcoe/core/types.hpp>
#include <gtest/gtest.h>

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265f
#endif

using namespace soundcoe;

//==============================================================================
//                    Vec3Tests - vec3 math operations tests
//==============================================================================

class Vec3Tests : public ::testing::Test
{
};

//==============================================================================
//                                 Construction
//==============================================================================

TEST_F(Vec3Tests, Construction)
{
    // Test 1: default and component constructors
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

    // Test 2: zero() is all zeros
    {
        vec3 zero = vec3::zero();
        EXPECT_EQ(zero.x, 0.0f);
        EXPECT_EQ(zero.y, 0.0f);
        EXPECT_EQ(zero.z, 0.0f);
    }
}

//==============================================================================
//                                  Arithmetic
//==============================================================================

TEST_F(Vec3Tests, Arithmetic)
{
    // Test 1: adding two vectors
    {
        vec3 v1(1.0f, 2.0f, 3.0f);
        vec3 v2(4.0f, 5.0f, 6.0f);
        vec3 result = v1 + v2;

        EXPECT_EQ(result.x, 5.0f);
        EXPECT_EQ(result.y, 7.0f);
        EXPECT_EQ(result.z, 9.0f);
    }

    // Test 2: subtracting two vectors
    {
        vec3 v1(5.0f, 7.0f, 9.0f);
        vec3 v2(1.0f, 2.0f, 3.0f);
        vec3 result = v1 - v2;

        EXPECT_EQ(result.x, 4.0f);
        EXPECT_EQ(result.y, 5.0f);
        EXPECT_EQ(result.z, 6.0f);
    }

    // Test 3: multiplying by a scalar
    {
        vec3 v(2.0f, 3.0f, 4.0f);
        vec3 result = v * 2.0f;

        EXPECT_EQ(result.x, 4.0f);
        EXPECT_EQ(result.y, 6.0f);
        EXPECT_EQ(result.z, 8.0f);
    }

    // Test 4: equal and not equal operators
    {
        vec3 v1(1.0f, 2.0f, 3.0f);
        vec3 v2(1.0f, 2.0f, 3.0f);
        vec3 v3(1.0f, 2.0f, 4.0f);

        EXPECT_TRUE(v1 == v2);
        EXPECT_FALSE(v1 == v3);
        EXPECT_TRUE(v1 != v3);
        EXPECT_FALSE(v1 != v2);
    }

    // Test 5: +=, -= and *=
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
}

//==============================================================================
//                           Length and Normalization
//==============================================================================

TEST_F(Vec3Tests, LengthAndNormalization)
{
    // Test 1: length of a vector
    {
        vec3 v(3.0f, 4.0f, 0.0f);
        EXPECT_FLOAT_EQ(v.length(), 5.0f);

        vec3 zero = vec3::zero();
        EXPECT_FLOAT_EQ(zero.length(), 0.0f);
    }

    // Test 2: normalized() copy
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

    // Test 3: normalize() in place
    {
        vec3 v(3.0f, 4.0f, 0.0f);
        v.normalize();

        EXPECT_FLOAT_EQ(v.length(), 1.0f);
        EXPECT_FLOAT_EQ(v.x, 0.6f);
        EXPECT_FLOAT_EQ(v.y, 0.8f);
        EXPECT_FLOAT_EQ(v.z, 0.0f);
    }
}

//==============================================================================
//                              Distance and Lerp
//==============================================================================

TEST_F(Vec3Tests, DistanceAndLerp)
{
    // Test 1: distance is the same both ways
    {
        vec3 v1(0.0f, 0.0f, 0.0f);
        vec3 v2(3.0f, 4.0f, 0.0f);

        EXPECT_FLOAT_EQ(v1.distance(v2), 5.0f);
        EXPECT_FLOAT_EQ(v2.distance(v1), 5.0f);
    }

    // Test 2: lerp at 0, 0.25, 0.5 and 1
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
}

//==============================================================================
//                              Products and Angle
//==============================================================================

TEST_F(Vec3Tests, Products)
{
    // Test 1: dot product
    {
        vec3 v1(1.0f, 2.0f, 3.0f);
        vec3 v2(4.0f, 5.0f, 6.0f);

        float dot = v1.dot(v2);
        EXPECT_FLOAT_EQ(dot, 32.0f);

        vec3 perpendicular1(1.0f, 0.0f, 0.0f);
        vec3 perpendicular2(0.0f, 1.0f, 0.0f);
        EXPECT_FLOAT_EQ(perpendicular1.dot(perpendicular2), 0.0f);
    }

    // Test 2: cross product
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

    // Test 3: angle between vectors
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
}

//==============================================================================
//                                Static Methods
//==============================================================================

TEST_F(Vec3Tests, StaticMethods)
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
