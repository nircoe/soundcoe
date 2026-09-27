#pragma once

#include <cmath>
#include <string_view>
#include <sstream>
#include <limits>
#include <soundcoe_config.hpp>
#if SOUNDCOE_USE_LOGCOE
#include <logcoe.hpp>
#endif

namespace soundcoe
{
    using LogLevel = logcoe::LogLevel;

    constexpr size_t UNLIMITED_CACHE = std::numeric_limits<size_t>::max();

    using sound_handle = size_t;
    using music_handle = size_t;

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
        vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

        static vec3 zero() { return vec3(); }
        static vec3 up() { return vec3(0.0f, 1.0f, 0.0f); }
        static float dot(const vec3 &v, const vec3 &u) { return v.dot(u); }
        static vec3 cross(const vec3 &v, const vec3 &u) { return v.cross(u); }
        static float distance(const vec3 &v, const vec3 &u) { return v.distance(u); }
        static vec3 lerp(const vec3 &v, const vec3 &u, float t) { return v.lerp(u, t); }

        vec3 operator+(const vec3 &other) const { return vec3(x + other.x, y + other.y, z + other.z); }
        vec3 operator-(const vec3 &other) const { return vec3(x - other.x, y - other.y, z - other.z); }
        vec3 operator*(float f) const { return vec3(f * x, f * y, f * z); }
        bool operator==(const vec3 &other) const { return (x == other.x && y == other.y && z == other.z); }
        bool operator!=(const vec3 &other) const { return !(*this == other); }
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

        float length() const { return sqrtf((x * x) + (y * y) + (z * z)); }
        float length_squared() const { return (x * x) + (y * y) + (z * z); }
        vec3 normalized() const
        {
            float len = this->length();
            return len == 0.0f ? vec3() : vec3(x / len, y / len, z / len);
        }
        void normalize() { *this = this->normalized(); }
        float distance(const vec3 &other) const { return (*this - other).length(); }
        float distance_squared(const vec3 &other) const { return (*this - other).length_squared(); }
        float dot(const vec3 &other) const { return (x * other.x) + (y * other.y) + (z * other.z); }
        vec3 cross(const vec3 &other) const { return vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x); }
        vec3 lerp(const vec3 &other, float t) const { return ((*this) * (1 - t)) + (other * t); }
        float angle(const vec3 &other) const { return acosf(this->normalized().dot(other.normalized())); }
    };

    namespace detail
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
            default:
                return "";
            }
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
            default:
                return "";
            }
        }
    } // namespace detail
} // namespace soundcoe
