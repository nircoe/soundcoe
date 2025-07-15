#include <soundcoe/utils/math.hpp>
#include <cmath>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace soundcoe
{
    namespace math
    {
        float distance(const Vec3 &a, const Vec3 &b)
        {
            return a.distance(b);
        }

        float distanceSquared(const Vec3 &a, const Vec3 &b)
        {
            return a.distanceSquared(b);
        }

        Vec3 normalized(const Vec3 &v)
        {
            return v.normalized();
        }

        float length(const Vec3 &v)
        {
            return v.length();
        }

        float lengthSquared(const Vec3 &v)
        {
            return v.lengthSquared();
        }

        float dot(const Vec3 &a, const Vec3 &b)
        {
            return a.dot(b);
        }

        Vec3 cross(const Vec3 &a, const Vec3 &b)
        {
            return a.cross(b);
        }

        float dbToLinear(float db)
        {
            return powf(10.0f, db / 20.0f);
        }

        float linearToDb(float linear)
        {
            if(linear <= 0.0f) return -std::numeric_limits<float>::infinity();
            return 20.0f * log10f(linear);
        }

        float dbToGain(float db)
        {
            return powf(10.0f, db / 10.0f);
        }

        float gainToDb(float gain)
        {
            if(gain <= 0.0f) return -std::numeric_limits<float>::infinity();
            return 10.0f * log10f(gain);
        }

        float samplesToTime(unsigned int samples, unsigned int sampleRate)
        {
            if(sampleRate == 0) return 0.0f;
            return static_cast<float>(samples) / static_cast<float>(sampleRate);
        }

        int timeToSamples(float seconds, unsigned int sampleRate)
        {
            if(seconds < 0.0f) return 0;
            return static_cast<int>(lroundf(seconds * static_cast<float>(sampleRate)));
        }

        float lerp(float a, float b, float t)
        {
            return (a * (1 - t)) + (b * t);
        }

        float clamp(float value, float min, float max)
        {
            return  (value < min) ? min :
                    (value > max) ? max :
                                    value;
        }

        float smoothstep(float edge0, float edge1, float x)
        {
            float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
            return t * t * (3.0f - (2.0f * t));
        }

        float exponentialFade(float t, float curve)
        {
            return powf(clamp(t), curve);
        }

        float calculateVolumeByDistance(float distance, float maxDistance, float rolloffFactor)
        {
            if(maxDistance <= 0.0f)     return 0.0f;
            distance = fabsf(distance);
            if(distance >= maxDistance) return 0.0f;
            if(distance == 0.0f)        return 1.0f;
            float volumeRatio = 1.0f - (distance / maxDistance);
            return powf(volumeRatio, rolloffFactor);
        }

        float calculatePan(const Vec3 &listenerPosition, const Vec3 &sourcePosition, const Vec3 &listenerForward)
        {
            Vec3 direction = sourcePosition - listenerPosition;
            Vec3 listenerRight = listenerForward.cross(Vec3::up());
            float dotRight = direction.normalized().dot(listenerRight.normalized());
            float angle = asinf(dotRight) * (180.0f / M_PI);
            return clamp(angle / 90.0f, -1.0f, 1.0f);
        }

        float semitonesToRatio(float semitones)
        {
            return powf(2.0f, semitones / 12.0f);
        }

        float ratioToSemitones(float ratio)
        {
            if(ratio <= 0.0f) return 0.0f;
            return 12.0f * log2f(ratio);
        }
    } // namespace math
} // namespace soundcoe