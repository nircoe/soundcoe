#pragma once

#include <filesystem>
#include <fstream>
#include <cmath>
#include <cstdint>
#include <system_error>

#define _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI 3.14159265f
#endif

class test_audio_files
{
public:
    inline static std::filesystem::path s_test_root_dir;
    inline static std::filesystem::path s_test_sub_dir1;
    inline static std::filesystem::path s_test_sub_dir2;
    inline static std::filesystem::path s_general_dir;
    inline static std::filesystem::path s_scene1_dir;
    inline static bool s_files_created = false;

    static void create_test_files()
    {
        if (s_files_created)
            return;

        s_test_root_dir = std::filesystem::temp_directory_path() / "soundcoe_test_shared";

        // For resources_tests.cpp compatibility
        s_test_sub_dir1 = s_test_root_dir / "sounds";
        s_test_sub_dir2 = s_test_root_dir / "music";

        // For playback_tests.cpp compatibility
        s_general_dir = s_test_root_dir / "general" / "sfx";
        s_scene1_dir = s_test_root_dir / "scene1" / "sfx";

        std::filesystem::path general_music_dir = s_test_root_dir / "general" / "music";
        std::filesystem::path scene1_music_dir = s_test_root_dir / "scene1" / "music";

        std::filesystem::create_directories(s_test_sub_dir1);
        std::filesystem::create_directories(s_test_sub_dir2);
        std::filesystem::create_directories(s_general_dir);
        std::filesystem::create_directories(s_scene1_dir);
        std::filesystem::create_directories(general_music_dir);
        std::filesystem::create_directories(scene1_music_dir);

        // Files for resources_tests.cpp
        create_wav_file(s_test_sub_dir1 / "test1.wav");
        create_wav_file(s_test_sub_dir1 / "test2.wav");
        create_wav_file(s_test_sub_dir2 / "music1.wav");
        std::ofstream(s_test_sub_dir1 / "readme.txt") << "Not audio\n";

        // Files for playback_tests.cpp
        create_wav_file(s_general_dir / "beep.wav");
        create_wav_file(s_general_dir / "click.wav");
        create_wav_file(s_scene1_dir / "explosion.wav");
        create_wav_file(general_music_dir / "background.wav");
        create_wav_file(scene1_music_dir / "battle.wav");

        s_files_created = true;
    }

    static void cleanup()
    {
        if (!s_files_created)
            return;
        std::error_code ec;
        std::filesystem::remove_all(s_test_root_dir, ec);
        s_files_created = false;
    }

private:
    static void create_wav_file(const std::filesystem::path& file_path)
    {
        std::ofstream file(file_path, std::ios::binary);

        // WAV header (44 bytes)
        file.write("RIFF", 4);
        uint32_t file_size = 36 + 88200; // Header + data
        file.write(reinterpret_cast<const char*>(&file_size), 4);
        file.write("WAVE", 4);

        // Format chunk
        file.write("fmt ", 4);
        uint32_t fmt_size = 16;
        file.write(reinterpret_cast<const char*>(&fmt_size), 4);
        uint16_t audio_format = 1; // PCM
        file.write(reinterpret_cast<const char*>(&audio_format), 2);
        uint16_t num_channels = 1;
        file.write(reinterpret_cast<const char*>(&num_channels), 2);
        uint32_t sample_rate = 44100;
        file.write(reinterpret_cast<const char*>(&sample_rate), 4);
        uint32_t byte_rate = sample_rate * num_channels * 2;
        file.write(reinterpret_cast<const char*>(&byte_rate), 4);
        uint16_t block_align = num_channels * 2;
        file.write(reinterpret_cast<const char*>(&block_align), 2);
        uint16_t bits_per_sample = 16;
        file.write(reinterpret_cast<const char*>(&bits_per_sample), 2);

        // Data chunk
        file.write("data", 4);
        uint32_t data_size = 88200; // ~1 second at 44100 Hz, 16-bit, mono
        file.write(reinterpret_cast<const char*>(&data_size), 4);

        // Simple sine wave data
        for (uint32_t i = 0; i < data_size / 2; ++i) {
            int16_t sample = static_cast<int16_t>(10000 * sin(2.0 * M_PI * 440.0 * i / sample_rate));
            file.write(reinterpret_cast<const char*>(&sample), 2);
        }
    }
};
