#pragma once

#include <soundcoe/core/types.hpp>

namespace soundcoe
{
    namespace math
    {
        float distance(const Vec3 &a, const Vec3 &b);
        float distanceSquared(const Vec3 &a, const Vec3 &b);
        Vec3 normalized(const Vec3 &v);
        float length(const Vec3 &v);
        float lengthSquared(const Vec3 &v);
        float dot(const Vec3 &a, const Vec3 &b);
        Vec3 cross(const Vec3 &a, const Vec3 &b);

        float dbToLinear(float db);
        float linearToDb(float linear);
        float dbToGain(float db);
        float gainToDb(float gain);

        float samplesToTime(unsigned int samples, unsigned int sampleRate);
        int timeToSamples(float seconds, unsigned int sampleRate);

        float lerp(float a, float b, float t);
        float clamp(float value, float min = 0.0f, float max = 1.0f);
        float smoothstep(float edge0, float edge1, float x);
        float exponentialFade(float t, float curve = 2.0f);

        float calculateVolumeByDistance(float distance, float maxDistance, float rolloffFactor = 1.0f);
        float calculatePan(const Vec3 &listenerPosition, const Vec3 &sourcePosition, const Vec3 &listenerForward);

        float semitonesToRatio(float semitones);
        float ratioToSemitones(float ratio);
    } // namespace math
} // namespace soundcoe