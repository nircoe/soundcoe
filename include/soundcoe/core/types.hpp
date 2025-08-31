#pragma once

#include <logcoe.hpp>
#include <cmath>
#include <string_view>
#include <sstream>
#include <limits>

namespace soundcoe
{
    using LogLevel = logcoe::LogLevel;

    constexpr size_t UNLIMITED_CACHE = std::numeric_limits<size_t>::max();

    using SoundHandle = size_t;
    using MusicHandle = size_t;

    enum class SoundState
    {
        Initial,
        Playing,
        Paused,
        Stopped
    };

    enum class SoundPriority
    {
        Low,
        Medium,
        High,
        Critical
    };

    struct Vec3
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        Vec3() = default;
        Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

        static Vec3 zero() { return Vec3(); }
        static Vec3 up() { return Vec3(0.0f, 1.0f, 0.0f); }
        static float dot(const Vec3 &v, const Vec3 &u) { return v.dot(u); }
        static Vec3 cross(const Vec3 &v, const Vec3 &u) { return v.cross(u); }
        static float distance(const Vec3 &v, const Vec3 &u) { return v.distance(u); }
        static Vec3 lerp(const Vec3 &v, const Vec3 &u, float t) { return v.lerp(u, t); }

        Vec3 operator+(const Vec3 &other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
        Vec3 operator-(const Vec3 &other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
        Vec3 operator*(float f) const { return Vec3(f * x, f * y, f * z); }
        bool operator==(const Vec3 &other) const { return (x == other.x && y == other.y && z == other.z); }
        bool operator!=(const Vec3 &other) const { return !(*this == other); }
        void operator+=(const Vec3 &other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
        }
        void operator-=(const Vec3 &other)
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
        float lengthSquared() const { return (x * x) + (y * y) + (z * z); }
        Vec3 normalized() const
        {
            float len = this->length();
            return len == 0.0f ? Vec3() : Vec3(x / len, y / len, z / len);
        }
        void normalize() { *this = this->normalized(); }
        float distance(const Vec3 &other) const { return (*this - other).length(); }
        float distanceSquared(const Vec3 &other) const { return (*this - other).lengthSquared(); }
        float dot(const Vec3 &other) const { return (x * other.x) + (y * other.y) + (z * other.z); }
        Vec3 cross(const Vec3 &other) const { return Vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x); }
        Vec3 lerp(const Vec3 &other, float t) const { return ((*this) * (1 - t)) + (other * t); }
        float angle(const Vec3 &other) const { return acosf(this->normalized().dot(other.normalized())); }
    };

    namespace detail
    {
        enum class AudioFormat
        {
            Wav,
            Ogg,
            Mp3,
            Unsupported
        };

        enum class PropertyType
        {
            Volume,
            Pitch,
            Position,
            Velocity
        };

        constexpr std::string_view toString(AudioFormat format)
        {
            switch (format)
            {
            case AudioFormat::Wav:
                return "WAV";
            case AudioFormat::Ogg:
                return "OGG";
            case AudioFormat::Mp3:
                return "MP3";
            default:
                return "";
            }
        }

        enum class AudioDecoderOperation
        {
            OpenFile,
            DecodeAudio
        };

        constexpr std::string_view toString(AudioDecoderOperation operation)
        {
            switch (operation)
            {
            case AudioDecoderOperation::OpenFile:
                return "Open File";
            case AudioDecoderOperation::DecodeAudio:
                return "Decode Audio";
            default:
                return "";
            }
        }
    } // namespace detail
} // namespace soundcoe