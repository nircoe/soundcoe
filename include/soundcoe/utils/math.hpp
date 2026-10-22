#pragma once

#include <soundcoe/core/types.hpp>

namespace soundcoe
{
    namespace math
    {
        [[nodiscard]] float distance(const vec3 &a, const vec3 &b);
        [[nodiscard]] float distance_squared(const vec3 &a, const vec3 &b);
        [[nodiscard]] vec3 normalized(const vec3 &v);
        [[nodiscard]] float length(const vec3 &v);
        [[nodiscard]] float length_squared(const vec3 &v);
        [[nodiscard]] float dot(const vec3 &a, const vec3 &b);
        [[nodiscard]] vec3 cross(const vec3 &a, const vec3 &b);

        [[nodiscard]] float db_to_linear(float db);
        [[nodiscard]] float linear_to_db(float linear);
        [[nodiscard]] float db_to_gain(float db);
        [[nodiscard]] float gain_to_db(float gain);

        [[nodiscard]] float samples_to_time(unsigned int samples, unsigned int sample_rate);
        [[nodiscard]] int time_to_samples(float seconds, unsigned int sample_rate);

        [[nodiscard]] float lerp(float a, float b, float t);
        [[nodiscard]] float clamp(float value, float min = 0.0f, float max = 1.0f);
        [[nodiscard]] float smoothstep(float edge0, float edge1, float x);
        [[nodiscard]] float exponential_fade(float t, float curve = 2.0f);

        [[nodiscard]] float calculate_volume_by_distance(float distance, float max_distance,
                                                         float rolloff_factor = 1.0f);
        [[nodiscard]] float calculate_pan(const vec3 &listener_position, const vec3 &source_position,
                                          const vec3 &listener_forward);

        [[nodiscard]] float semitones_to_ratio(float semitones);
        [[nodiscard]] float ratio_to_semitones(float ratio);
    } // namespace math
} // namespace soundcoe
