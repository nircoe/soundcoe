#pragma once

#include <filesystem>
#include <fstream>
#include <cmath>

#define _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI 3.14159265f
#endif

class TestAudioFiles
{
public:
    inline static std::filesystem::path s_testRootDir;
    inline static std::filesystem::path s_testSubDir1;
    inline static std::filesystem::path s_testSubDir2;
    inline static std::filesystem::path s_generalDir;
    inline static std::filesystem::path s_scene1Dir;
    inline static bool s_filesCreated = false;

    static void createTestFiles()
    {
        if (s_filesCreated)
            return;

        s_testRootDir = std::filesystem::temp_directory_path() / "soundcoe_test_shared";
        
        // For resources_tests.cpp compatibility
        s_testSubDir1 = s_testRootDir / "sounds";
        s_testSubDir2 = s_testRootDir / "music";
        
        // For playback_tests.cpp compatibility  
        s_generalDir = s_testRootDir / "general" / "sfx";
        s_scene1Dir = s_testRootDir / "scene1" / "sfx";
        
        std::filesystem::path generalMusicDir = s_testRootDir / "general" / "music";
        std::filesystem::path scene1MusicDir = s_testRootDir / "scene1" / "music";

        std::filesystem::create_directories(s_testSubDir1);
        std::filesystem::create_directories(s_testSubDir2);
        std::filesystem::create_directories(s_generalDir);
        std::filesystem::create_directories(s_scene1Dir);
        std::filesystem::create_directories(generalMusicDir);
        std::filesystem::create_directories(scene1MusicDir);

        // Files for resources_tests.cpp
        createWavFile(s_testSubDir1 / "test1.wav");
        createWavFile(s_testSubDir1 / "test2.wav");
        createWavFile(s_testSubDir2 / "music1.wav");
        std::ofstream(s_testSubDir1 / "readme.txt") << "Not audio\n";
        
        // Files for playback_tests.cpp
        createWavFile(s_generalDir / "beep.wav");
        createWavFile(s_generalDir / "click.wav");
        createWavFile(s_scene1Dir / "explosion.wav");
        createWavFile(generalMusicDir / "background.wav");
        createWavFile(scene1MusicDir / "battle.wav");

        s_filesCreated = true;
    }

    static void cleanup()
    {
        if (!s_filesCreated)
            return;
        std::error_code ec;
        std::filesystem::remove_all(s_testRootDir, ec);
        s_filesCreated = false;
    }

private:
    static void createWavFile(const std::filesystem::path& filePath)
    {
        std::ofstream file(filePath, std::ios::binary);
        
        // WAV header (44 bytes)
        file.write("RIFF", 4);
        uint32_t fileSize = 36 + 88200; // Header + data
        file.write(reinterpret_cast<const char*>(&fileSize), 4);
        file.write("WAVE", 4);
        
        // Format chunk
        file.write("fmt ", 4);
        uint32_t fmtSize = 16;
        file.write(reinterpret_cast<const char*>(&fmtSize), 4);
        uint16_t audioFormat = 1; // PCM
        file.write(reinterpret_cast<const char*>(&audioFormat), 2);
        uint16_t numChannels = 1;
        file.write(reinterpret_cast<const char*>(&numChannels), 2);
        uint32_t sampleRate = 44100;
        file.write(reinterpret_cast<const char*>(&sampleRate), 4);
        uint32_t byteRate = sampleRate * numChannels * 2;
        file.write(reinterpret_cast<const char*>(&byteRate), 4);
        uint16_t blockAlign = numChannels * 2;
        file.write(reinterpret_cast<const char*>(&blockAlign), 2);
        uint16_t bitsPerSample = 16;
        file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
        
        // Data chunk
        file.write("data", 4);
        uint32_t dataSize = 88200; // ~1 second at 44100 Hz, 16-bit, mono
        file.write(reinterpret_cast<const char*>(&dataSize), 4);
        
        // Simple sine wave data
        for (uint32_t i = 0; i < dataSize / 2; ++i) {
            int16_t sample = static_cast<int16_t>(10000 * sin(2.0 * M_PI * 440.0 * i / sampleRate));
            file.write(reinterpret_cast<const char*>(&sample), 2);
        }
    }
};