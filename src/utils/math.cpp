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
        float distance(const vec3 &a, const vec3 &b)
        {
            return a.distance(b);
        }

        float distance_squared(const vec3 &a, const vec3 &b)
        {
            return a.distance_squared(b);
        }

        vec3 normalized(const vec3 &v)
        {
            return v.normalized();
        }

        float length(const vec3 &v)
        {
            return v.length();
        }

        float length_squared(const vec3 &v)
        {
            return v.length_squared();
        }

        float dot(const vec3 &a, const vec3 &b)
        {
            return a.dot(b);
        }

        vec3 cross(const vec3 &a, const vec3 &b)
        {
            return a.cross(b);
        }

        float db_to_linear(float db)
        {
            return powf(10.0f, db / 20.0f);
        }

        float linear_to_db(float linear)
        {
            if(linear <= 0.0f) return -std::numeric_limits<float>::infinity();
            return 20.0f * log10f(linear);
        }

        float db_to_gain(float db)
        {
            return powf(10.0f, db / 10.0f);
        }

        float gain_to_db(float gain)
        {
            if(gain <= 0.0f) return -std::numeric_limits<float>::infinity();
            return 10.0f * log10f(gain);
        }

        float samples_to_time(unsigned int samples, unsigned int sample_rate)
        {
            if(sample_rate == 0) return 0.0f;
            return static_cast<float>(samples) / static_cast<float>(sample_rate);
        }

        int time_to_samples(float seconds, unsigned int sample_rate)
        {
            if(seconds < 0.0f) return 0;
            return static_cast<int>(lroundf(seconds * static_cast<float>(sample_rate)));
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

        float exponential_fade(float t, float curve)
        {
            return powf(clamp(t), curve);
        }

        float calculate_volume_by_distance(float distance, float max_distance, float rolloff_factor)
        {
            if(max_distance <= 0.0f)     return 0.0f;
            distance = fabsf(distance);
            if(distance >= max_distance) return 0.0f;
            if(distance == 0.0f)        return 1.0f;
            float volume_ratio = 1.0f - (distance / max_distance);
            return powf(volume_ratio, rolloff_factor);
        }

        float calculate_pan(const vec3 &listener_position, const vec3 &source_position, const vec3 &listener_forward)
        {
            vec3 direction = source_position - listener_position;
            vec3 listener_right = listener_forward.cross(vec3::up());
            float dot_right = direction.normalized().dot(listener_right.normalized());
            float angle = asinf(dot_right) * (180.0f / static_cast<float>(M_PI));
            return clamp(angle / 90.0f, -1.0f, 1.0f);
        }

        float semitones_to_ratio(float semitones)
        {
            return powf(2.0f, semitones / 12.0f);
        }

        float ratio_to_semitones(float ratio)
        {
            if(ratio <= 0.0f) return 0.0f;
            return 12.0f * log2f(ratio);
        }
    } // namespace math
} // namespace soundcoe
