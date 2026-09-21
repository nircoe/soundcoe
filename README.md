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
**Performance** - Resource pooling, caching  

```cpp
soundcoe::initialize("./audio");
soundcoe::preloadScene("menu");

auto click = soundcoe::playSound("ui_click.wav"); 
auto music = soundcoe::fadeInMusic("theme.ogg", 2.0f);
auto explosion = soundcoe::playSound3D("boom.wav", {10.0f, 0.0f, -20.0f});
```

## Requirements

- C++20 compiler
- CMake 3.22+
- Windows, Linux, macOS or WebAssembly (Emscripten)

OpenAL-Soft, logcoe (and testcoe for tests) are fetched automatically.

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
└── audio/
    ├── general/             # Optional: Always-loaded audio
    │   ├── sfx/
    │   │   └── ui_click.wav
    │   └── music/
    │       └── main_theme.ogg
    └── menu/                # Scene-specific audio
        ├── sfx/
        │   └── menu_select.wav
        └── music/
            └── menu_ambient.ogg
```

Note: Ensure your audio directory is accessible relative to your executable at runtime (copy it to your build directory during the build process).

### 3. Use in your application

```cpp
#include <soundcoe.hpp>

int main() {
    soundcoe::initialize("./audio");  // relative to executable
    soundcoe::preloadScene("menu");

    auto clickHandle = soundcoe::playSound("ui_click.wav");
    auto musicHandle = soundcoe::playMusic("menu_ambient.ogg");
    soundcoe::fadeOutMusic(musicHandle, 2.0f);

    soundcoe::shutdown();
    return 0;
}
```

For 3D spatial audio, fade effects and backend configuration, see [Architecture Documentation](docs/ARCHITECTURE.md).

## API Reference

### Initialization
```cpp
soundcoe::initialize(
    "./audio",        // Audio root directory (relative to executable)
    64,              // Max sources (default: 64)
    soundcoe::UNLIMITED_CACHE, // Cache size MB (default: unlimited, cap it once you've profiled real usage)
    "sfx",           // Sound subdirectory inside each scene/general (default: "sfx")
    "music",         // Music subdirectory inside each scene/general (default: "music")
    soundcoe::LogLevel::DEBUG  // Log level (default: DEBUG)
);
```

Use `soundcoe::UNLIMITED_CACHE` while developing to measure peak audio memory, then set a limit (in MB) for your target platforms.

On constrained targets, use a lower `maxSources` (default 64) and a real cache limit to save memory.

The full function list is documented in the docstrings of `include/soundcoe.hpp`. When changing scenes, load the next
scene before unloading the previous one.

## Supported Audio Formats

WAV, OGG and MP3 (`.wav`, `.ogg`, `.mp3`).

## Documentation

- [Architecture](docs/ARCHITECTURE.md) - Internal design and implementation details
- [Contributing](docs/CONTRIBUTING.md) - Development setup and contribution guidelines
- [Roadmap](docs/ROADMAP.md) - Implemented and planned features

## License

MIT License - see [LICENSE](LICENSE) file for details. Third-party dependency licenses (notably
OpenAL-Soft's LGPL v2) are documented in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).