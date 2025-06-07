#pragma once

namespace soundcoe
{
    enum class SoundState
    {
        INITIAL,
        PLAYING,
        PAUSED,
        STOPPED
    };

    enum class SoundPriority
    {
        LOW,
        MEDIUM,
        HIGH,
        CRITICAL
    };

    struct Vec3
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        Vec3() = default;
        Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) { }
    };
} // namespace soundcoe