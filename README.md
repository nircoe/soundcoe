# soundcoe

C++ audio library for game developers, built on OpenAL. Thread-safe, single include.

[![Windows](https://github.com/nircoe/soundcoe/actions/workflows/ci-windows.yml/badge.svg)](https://github.com/nircoe/soundcoe/actions/workflows/ci-windows.yml)
[![Linux](https://github.com/nircoe/soundcoe/actions/workflows/ci-linux.yml/badge.svg)](https://github.com/nircoe/soundcoe/actions/workflows/ci-linux.yml)
[![macOS](https://github.com/nircoe/soundcoe/actions/workflows/ci-macos.yml/badge.svg)](https://github.com/nircoe/soundcoe/actions/workflows/ci-macos.yml)
[![Web](https://github.com/nircoe/soundcoe/actions/workflows/ci-web.yml/badge.svg)](https://github.com/nircoe/soundcoe/actions/workflows/ci-web.yml)

## Why soundcoe?

- Single `#include <soundcoe.hpp>` with free functions
- Scenes, fade effects, 3D audio and handle-based control
- Thread-safe, callable from any thread
- Source pooling and buffer caching

## Requirements

- C++23 compiler with `<expected>` (GCC 12+, Clang 16+ with libc++, Apple Clang from Xcode 15+, MSVC 2022 17.3+)
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
    if (!soundcoe::initialize("./audio") || !soundcoe::preload_scene("menu"))  // relative to executable
        return 1;

    auto click = soundcoe::play_sound("ui_click.wav");
    auto music = soundcoe::play_music("menu_ambient.ogg");
    if (!click || !music)
        return 1;
    if (!soundcoe::fade_out_music(*music, 2.0f))
        return 1;

    soundcoe::shutdown();
    return 0;
}
```

For 3D spatial audio, fade effects and backend configuration, see [Architecture Documentation](docs/ARCHITECTURE.md).

## Error Handling

Functions that can fail return `std::expected<T, soundcoe::error>` (`std::expected<void, soundcoe::error>` when there
is no value). Check the result before using it. The error has a `code` (a `soundcoe::error_code`) and a `message`.
Errors are also logged once when they happen.

```cpp
auto click = soundcoe::play_sound("missing.wav");
if (!click) {
    if (click.error().code == soundcoe::error_code::file_not_found)
        std::cerr << click.error().message << '\n';
}
```

## API Reference

### Initialization
```cpp
auto result = soundcoe::initialize(
    "./audio",        // Audio root directory (relative to executable)
    64,              // Max sources (default: 64)
    soundcoe::UNLIMITED_CACHE, // Cache size MB (default: unlimited, cap it once you've profiled real usage)
    "sfx",           // Sound subdirectory inside each scene/general (default: "sfx")
    "music",         // Music subdirectory inside each scene/general (default: "music")
    soundcoe::LogLevel::DEBUG  // Log level (default: DEBUG)
);
```

Use `soundcoe::UNLIMITED_CACHE` while developing to measure peak audio memory, then set a limit (in MB) for your target platforms.

On constrained targets, use a lower `max_sources` (default 64) and a real cache limit to save memory.

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
