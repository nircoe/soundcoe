#pragma once

#include <soundcoe/core/types.hpp>

namespace soundcoe
{
    namespace math
    {
        float distance(const vec3 &a, const vec3 &b);
        float distance_squared(const vec3 &a, const vec3 &b);
        vec3 normalized(const vec3 &v);
        float length(const vec3 &v);
        float length_squared(const vec3 &v);
        float dot(const vec3 &a, const vec3 &b);
        vec3 cross(const vec3 &a, const vec3 &b);

        float db_to_linear(float db);
        float linear_to_db(float linear);
        float db_to_gain(float db);
        float gain_to_db(float gain);

        float samples_to_time(unsigned int samples, unsigned int sample_rate);
        int time_to_samples(float seconds, unsigned int sample_rate);

        float lerp(float a, float b, float t);
        float clamp(float value, float min = 0.0f, float max = 1.0f);
        float smoothstep(float edge0, float edge1, float x);
        float exponential_fade(float t, float curve = 2.0f);

        float calculate_volume_by_distance(float distance, float max_distance, float rolloff_factor = 1.0f);
        float calculate_pan(const vec3 &listener_position, const vec3 &source_position, const vec3 &listener_forward);

        float semitones_to_ratio(float semitones);
        float ratio_to_semitones(float ratio);
    } // namespace math
} // namespace soundcoe
