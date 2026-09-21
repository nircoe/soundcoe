# soundcoe

C++ audio library for game developers. Thread-safe, zero-config, single-include design with OpenAL backend.

[![Windows](https://github.com/nircoe/soundcoe/actions/workflows/ci-windows.yml/badge.svg)](https://github.com/nircoe/soundcoe/actions/workflows/ci-windows.yml)
[![Linux](https://github.com/nircoe/soundcoe/actions/workflows/ci-linux.yml/badge.svg)](https://github.com/nircoe/soundcoe/actions/workflows/ci-linux.yml)
[![macOS](https://github.com/nircoe/soundcoe/actions/workflows/ci-macos.yml/badge.svg)](https://github.com/nircoe/soundcoe/actions/workflows/ci-macos.yml)
[![Web](https://github.com/nircoe/soundcoe/actions/workflows/ci-web.yml/badge.svg)](https://github.com/nircoe/soundcoe/actions/workflows/ci-web.yml)

## Why soundcoe?

**Simple** - Single `#include <soundcoe.hpp>`, static functions, no setup required  
**Game-Focused** - Scene management, fade effects, spatial audio, handle-based control  
**Thread-Safe** - Call from any thread  
**Performance** - Resource pooling, caching, priority-based allocation  

```cpp
soundcoe::initialize("./audio");
soundcoe::preloadScene("menu");

auto click = soundcoe::playSound("ui_click.wav"); 
auto music = soundcoe::fadeInMusic("theme.ogg", 2.0f);
auto explosion = soundcoe::playSound3D("boom.wav", {10.0f, 0.0f, -20.0f});
```

## Dependencies
- **C++17 or later** - Required language standard
- **CMake 3.22+** - Build system
- **OpenAL-Soft** - Audio backend (automatically fetched)
- **logcoe** - Logging system (automatically fetched)
- **testcoe** - Testing framework (automatically fetched, tests only)

## Platform Support

soundcoe is tested on:
- Windows
- macOS
- Linux
- WebAssembly/Emscripten

## Quick Start

### 1. Add to your project (CMake)

```cmake
include(FetchContent)
FetchContent_Declare(
    soundcoe
    GIT_REPOSITORY https://github.com/nircoe/soundcoe.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(soundcoe)

target_link_libraries(your_target PRIVATE soundcoe)
```

### 2. Set up your audio directory structure

```
your_game/
├── build/
│   ├── your_game.exe        # Your executable
│   └── audio/               # Copy audio directory here (relative to executable)
└── audio/                   # Source audio directory
    ├── general/             # Optional: Always-loaded audio
    │   ├── sfx/             # Sound effects
    │   │   └── ui_click.wav
    │   └── music/
    │       └── main_theme.ogg
    ├── menu/                # Scene-specific audio
    │   ├── sfx/
    │   │   └── menu_select.wav
    │   └── music/
    │       └── menu_ambient.ogg
    └── level1/              # Scene-specific audio
        ├── sfx/
        │   └── explosion.wav
        └── music/
            └── battle_theme.ogg
```

Note: Ensure your audio directory is accessible relative to your executable at runtime (copy it to your build directory during the build process).

### 3. Use in your application

```cpp
#include <soundcoe.hpp>

int main() {
    // Initialize with audio root directory (relative to executable)
    soundcoe::initialize("./audio");
    
    // Load scene audio
    soundcoe::preloadScene("menu");
    
    // Play sounds and music with simple handles
    auto clickHandle = soundcoe::playSound("ui_click.wav");
    auto musicHandle = soundcoe::playMusic("menu_ambient.ogg");
    
    // Advanced audio control
    soundcoe::fadeOutMusic(musicHandle, 2.0f);
    soundcoe::setSoundVolume(clickHandle, 0.8f);
    
    // Scene transitions (load new scene first, then unload previous)
    soundcoe::preloadScene("level1");
    soundcoe::unloadScene("menu");
    
    soundcoe::shutdown();
    return 0;
}
```

For advanced usage examples including 3D spatial audio, fade effects, and custom configurations, see [Architecture Documentation](docs/ARCHITECTURE.md).

## API Reference

### Initialization
```cpp
// Basic initialization
soundcoe::initialize("./audio");  // relative to executable

// Full configuration
soundcoe::initialize(
    "./audio",        // Audio root directory (relative to executable)
    64,              // Max sources (default: 64)
    soundcoe::UNLIMITED_CACHE, // Cache size MB (default: unlimited, cap it once you've profiled real usage)
    "sfx",           // Sound subdirectory inside each scene/general (default: "sfx")
    "music",         // Music subdirectory inside each scene/general (default: "music")
    LogLevel::DEBUG  // Log level (default: DEBUG)
);

// Clean shutdown
soundcoe::shutdown();
```

### Cache Management

soundcoe caches loaded audio files automatically:

```cpp
// Standard cache limits
soundcoe::initialize("./audio", 32, 64);       // 64MB cache limit
soundcoe::initialize("./audio", 32, 256);      // 256MB cache limit

// Development/Testing: measure actual memory usage
soundcoe::initialize("./audio", 32, soundcoe::UNLIMITED_CACHE);
```

Development tip: Use `soundcoe::UNLIMITED_CACHE` during testing to measure your game's peak audio memory usage, then set an appropriate limit for your target platforms.

### Scene Management
```cpp
// Load scene audio
soundcoe::preloadScene("level1");
soundcoe::isSceneLoaded("level1");

// Unload when no longer needed
soundcoe::unloadScene("level1");
```

### Audio Playback
```cpp
// Sound effects
SoundHandle handle = soundcoe::playSound("explosion.wav", 1.0f, 1.0f, false);

// Background music
MusicHandle music = soundcoe::playMusic("background.ogg", 0.8f, 1.0f, true);

// 3D positioned audio
SoundHandle spatial = soundcoe::playSound3D("footstep.wav", soundcoe::Vec3(5.0f, 0.0f, -10.0f));
```

### Audio Control
```cpp
// Playback control
soundcoe::pauseSound(handle);
soundcoe::resumeSound(handle);
soundcoe::stopSound(handle);

// Property modification
soundcoe::setSoundVolume(handle, 0.7f);
soundcoe::setSoundPitch(handle, 1.2f);

// Fade effects
SoundHandle fadeIn = soundcoe::fadeInSound("intro.wav", 3.0f);
soundcoe::fadeOutMusic(musicHandle, 2.5f);
```

### Master Controls
```cpp
// Volume mixing
soundcoe::setMasterVolume(0.8f);
soundcoe::setMasterSoundsVolume(0.9f);
soundcoe::setMasterMusicVolume(0.6f);

// Muting
soundcoe::muteAll();
soundcoe::unmuteAllSounds();
soundcoe::isMuted();
```

## Supported Audio Formats

| Format | Extension | Quality | Use Case |
|--------|-----------|---------|----------|
| **WAV** | `.wav` | Lossless | Short sound effects, audio where quality matters |
| **OGG** | `.ogg` | Compressed | Music, ambient sounds, voice |
| **MP3** | `.mp3` | Compressed | Music, voice, compatibility |

## Thread Safety

soundcoe is thread-safe, so multiple game threads can call it at once:

```cpp
#include <thread>
#include <vector>

void audioWorker(int workerId) {
    for (int i = 0; i < 100; ++i) {
        auto handle = soundcoe::playSound("worker_sound.wav");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        soundcoe::stopSound(handle);
    }
}

int main() {
    soundcoe::initialize("./audio");
    soundcoe::preloadScene("test");
    
    // Multiple threads safely calling soundcoe functions
    std::vector<std::thread> workers;
    for (int i = 0; i < 4; ++i) {
        workers.emplace_back(audioWorker, i);
    }
    
    for (auto& t : workers) {
        t.join();
    }
    
    soundcoe::shutdown();
    return 0;
}
```

## Requirements  

- **Compiler**: C++17 compatible (GCC 7+, Clang 5+, MSVC 2017+)
- **Build System**: CMake 3.22+
- **Platforms**: Windows, Linux, macOS, WebAssembly/Emscripten
- **Audio Hardware**: Required for actual audio playback testing (CI systems are headless)

## Performance Considerations

- **Resource Pooling**: Pre-allocated source pools prevent runtime allocation
- **Scene Management**: Load only needed audio, automatic cleanup on scene transitions
- **Thread Contention**: Per-class mutexes limit contention

## Documentation

- [Architecture](docs/ARCHITECTURE.md) - Internal design and implementation details
- [Contributing](docs/CONTRIBUTING.md) - Development setup and contribution guidelines
- [Roadmap](docs/ROADMAP.md) - Version history and planned features

## License

MIT License - see [LICENSE](LICENSE) file for details. Third-party dependency licenses (notably
OpenAL-Soft's LGPL v2) are documented in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).