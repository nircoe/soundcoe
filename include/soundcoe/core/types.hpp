#pragma once

#include <soundcoe_config.hpp>
#include <cmath>
#include <cstddef>
#include <limits>
#include <string_view>
#include <utility>

#if SOUNDCOE_USE_LOGCOE
#include <logcoe.hpp>
#endif

namespace soundcoe
{
    using LogLevel = logcoe::LogLevel;

    constexpr std::size_t UNLIMITED_CACHE = std::numeric_limits<std::size_t>::max();

    using sound_handle = std::size_t;
    using music_handle = std::size_t;

    enum class sound_state
    {
        initial,
        playing,
        paused,
        stopped
    };

    enum class sound_priority
    {
        low,
        medium,
        high,
        critical
    };

    struct vec3
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        vec3() = default;
        vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

        [[nodiscard]] static vec3 zero() { return vec3(); }
        [[nodiscard]] static vec3 up() { return vec3(0.0f, 1.0f, 0.0f); }
        [[nodiscard]] static float dot(const vec3 &v, const vec3 &u) { return v.dot(u); }
        [[nodiscard]] static vec3 cross(const vec3 &v, const vec3 &u) { return v.cross(u); }
        [[nodiscard]] static float distance(const vec3 &v, const vec3 &u) { return v.distance(u); }
        [[nodiscard]] static vec3 lerp(const vec3 &v, const vec3 &u, float t) { return v.lerp(u, t); }

        [[nodiscard]] vec3 operator+(const vec3 &other) const { return vec3(x + other.x, y + other.y, z + other.z); }
        [[nodiscard]] vec3 operator-(const vec3 &other) const { return vec3(x - other.x, y - other.y, z - other.z); }
        [[nodiscard]] vec3 operator*(float f) const { return vec3(f * x, f * y, f * z); }
        [[nodiscard]] bool operator==(const vec3 &other) const
        {
            return (x == other.x && y == other.y && z == other.z);
        }
        [[nodiscard]] bool operator!=(const vec3 &other) const { return !(*this == other); }
        void operator+=(const vec3 &other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
        }
        void operator-=(const vec3 &other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
        }
        void operator*=(float f)
        {
            x *= f;
            y *= f;
            z *= f;
        }

        [[nodiscard]] float length() const { return sqrtf((x * x) + (y * y) + (z * z)); }
        [[nodiscard]] float length_squared() const { return (x * x) + (y * y) + (z * z); }
        [[nodiscard]] vec3 normalized() const
        {
            float len = this->length();
            return len == 0.0f ? vec3() : vec3(x / len, y / len, z / len);
        }
        void normalize() { *this = this->normalized(); }
        [[nodiscard]] float distance(const vec3 &other) const { return (*this - other).length(); }
        [[nodiscard]] float distance_squared(const vec3 &other) const { return (*this - other).length_squared(); }
        [[nodiscard]] float dot(const vec3 &other) const { return (x * other.x) + (y * other.y) + (z * other.z); }
        [[nodiscard]] vec3 cross(const vec3 &other) const
        {
            return vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
        }
        [[nodiscard]] vec3 lerp(const vec3 &other, float t) const { return ((*this) * (1 - t)) + (other * t); }
        [[nodiscard]] float angle(const vec3 &other) const { return acosf(this->normalized().dot(other.normalized())); }
    };

    namespace internal
    {
        enum class audio_format
        {
            wav,
            ogg,
            mp3,
            unsupported
        };

        enum class property_type
        {
            volume,
            pitch,
            position,
            velocity
        };

        constexpr std::string_view to_string(audio_format format)
        {
            switch (format)
            {
            case audio_format::wav:
                return "WAV";
            case audio_format::ogg:
                return "OGG";
            case audio_format::mp3:
                return "MP3";
            case audio_format::unsupported:
                return "Unsupported";
            }
            std::unreachable();
        }

        enum class audio_decoder_operation
        {
            open_file,
            decode_audio
        };

        constexpr std::string_view to_string(audio_decoder_operation operation)
        {
            switch (operation)
            {
            case audio_decoder_operation::open_file:
                return "Open File";
            case audio_decoder_operation::decode_audio:
                return "Decode Audio";
            }
            std::unreachable();
        }
    } // namespace internal
} // namespace soundcoe
