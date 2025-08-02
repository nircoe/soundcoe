# soundcoe Architecture & Advanced Usage

## Overview

soundcoe provides a layered architecture designed for game developers who need powerful audio without complexity. This document covers advanced usage patterns, performance optimization, and internal design for contributors.

## Quick Navigation

- [Advanced Usage Examples](#advanced-usage-examples) - 3D audio, fade effects, custom configurations
- [Game Performance Tips](#game-performance-tips) - Optimization strategies for games
- [Internal Architecture](#internal-architecture) - Implementation details for contributors

## Advanced Usage Examples

### Custom Configuration

```cpp
#include <soundcoe.hpp>

int main() {
    // Initialize with custom settings for your game
    soundcoe::initialize(
        "./audio",    // Audio directory (relative to executable)
        64,           // Max simultaneous sources (good for action games)
        128,          // Cache size MB (adjust based on your audio assets)
        "sfx",        // Sound subdirectory inside each scene/general
        "music"       // Music subdirectory inside each scene/general
    );
    
    soundcoe::preloadScene("level1");
    
    // Your game loop here...
    
    soundcoe::shutdown();
    return 0;
}
```

### 3D Spatial Audio

Perfect for first-person shooters, racing games, and immersive experiences:

```cpp
// Set up listener (usually your player/camera)
soundcoe::Vec3 playerPos(0.0f, 1.5f, 0.0f);        // Player position
soundcoe::Vec3 playerVel(2.0f, 0.0f, 0.0f);        // Player velocity (for doppler)
soundcoe::Vec3 forward(0.0f, 0.0f, -1.0f);         // Looking direction
soundcoe::Vec3 up(0.0f, 1.0f, 0.0f);               // Up vector

soundcoe::updateListener(playerPos, playerVel, forward, up);

// Play 3D positioned sounds
soundcoe::Vec3 enemyPos(10.0f, 0.0f, -20.0f);
auto gunshot = soundcoe::playSound3D("gunshot.wav", enemyPos);

soundcoe::Vec3 carPos(-5.0f, 0.0f, 15.0f);
auto engine = soundcoe::playSound3D("car_engine.wav", carPos, 0.8f, 1.0f, true);

// Update positions in your game loop
soundcoe::setSoundPosition(engine, updatedCarPos);
```

### Advanced Fade Effects

Great for dynamic music and smooth audio transitions:

```cpp
// Fade in background music when entering a scene
auto musicHandle = soundcoe::fadeInMusic("battle_theme.ogg", 3.0f);

// Fade between different volume levels during gameplay
soundcoe::fadeToVolumeMusic(musicHandle, 0.3f, 1.5f);  // Fade to 30% over 1.5 seconds

// Fade out when player takes damage or enters stealth
soundcoe::fadeToVolumeMusic(musicHandle, 0.1f, 0.8f);  // Fade to 10% over 0.8 seconds

// Fade out completely when scene ends
soundcoe::fadeOutMusic(musicHandle, 2.0f);

// Fade in sound effects for dramatic effect
auto explosionHandle = soundcoe::fadeInSound("big_explosion.wav", 1.2f);
```

### Master Volume Controls

Essential for game settings and audio mixing:

```cpp
// Global volume control (affects everything)
soundcoe::setMasterVolume(0.8f);

// Separate control for sound effects and music
soundcoe::setMasterSoundsVolume(0.9f);   // Keep SFX loud
soundcoe::setMasterMusicVolume(0.4f);    // Reduce music for gameplay

// Muting system (preserve volume levels)
soundcoe::muteAll();                     // Mute everything
soundcoe::unmuteAllSounds();             // Unmute sounds only
soundcoe::unmuteAllMusic();              // Unmute music only

// Check mute status
if (soundcoe::isMuted()) {
    // Update UI to show muted state
}
```

### Scene-Based Game Audio

Organize your audio by game scenes for better memory management:

```cpp
// Menu scene
soundcoe::preloadScene("menu");
auto menuMusic = soundcoe::playMusic("menu_theme.ogg", 0.6f, 1.0f, true);
auto clickSound = soundcoe::playSound("ui_click.wav");

// Scene transition (load new scene first, then unload current scene)
soundcoe::preloadScene("level1");        // Load new scene first
soundcoe::fadeOutMusic(menuMusic, 1.0f);
soundcoe::unloadScene("menu");           // Unload previous scene after loading new one

// In-game audio for the new scene
auto gameMusic = soundcoe::fadeInMusic("level1_theme.ogg", 2.0f);
auto ambientSound = soundcoe::playSound("forest_ambient.ogg", 0.4f, 1.0f, true);

// Another scene transition (always load new scene before unloading current scene)
soundcoe::preloadScene("level2");        // Load next scene first
soundcoe::unloadScene("level1");         // Then unload current scene
```

### Thread-Safe Game Integration

soundcoe works safely with multi-threaded game engines:

```cpp
#include <thread>
#include <future>

// Audio can be called from any thread
void gameAudioThread() {
    while (gameRunning) {
        // Safe to call from background thread
        soundcoe::update();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void gameLogicThread() {
    // Safe to call from game logic thread
    if (playerShoots) {
        soundcoe::playSound("laser.wav");
    }
    
    if (enemyHit) {
        soundcoe::playSound3D("enemy_death.wav", enemyPosition);
    }
}

// Both threads can safely call soundcoe functions simultaneously
```

## Game Performance Tips

### Scene Memory Management

```cpp
// Always load new scene before unloading current scene
soundcoe::preloadScene("next_scene");       // Load next scene first
soundcoe::unloadScene("current_scene");     // Then unload current scene

// Adjust cache size based on your game's scene complexity
soundcoe::initialize("./audio", 32, 64);     // 64MB cache for simple scenes
soundcoe::initialize("./audio", 64, 256);    // 256MB cache for complex scenes
```

### Source Pool Optimization

```cpp
// Configure source pool based on your game - choose how many sources that you would like! remember it is the maximum concurrent amount.
soundcoe::initialize("./audio", 16);         // 16 sources for games with less concurrent audio
soundcoe::initialize("./audio", 32);         // 32 sources - default
soundcoe::initialize("./audio", 64);         // 64 sources for game with more concurrent audio
```

### Supported Audio Format

- **WAV**
- **OGG**
- **MP3**

### Performance Best Practices

1. **Scene loading order** - Always preload new scenes before unloading current scenes
2. **Preload scenes** before scene transitions
3. **Use 3D audio sparingly** - only for important positional sounds
4. **Limit concurrent sounds** - stop old sounds when playing new ones
5. **Use fade effects** for smooth scene transitions

## Internal Architecture

*This section is for contributors and advanced users interested in soundcoe's implementation.*

### Architecture Overview

soundcoe uses a layered design that hides OpenAL complexity behind a simple static API:

```
Game Code → soundcoe API → Internal Systems → OpenAL
```

### Key Components

**Public API Layer**
- Single `soundcoe.hpp` header
- Static functions only (no objects to manage)
- Thread-safe by design

**Scene Management**
- Maps scene names to directory structures
- Automatic loading/unloading of audio files
- Smart caching with configurable memory limits

**Resource Management**
- Source pool with priority-based allocation
- Buffer caching for loaded audio files
- Automatic cleanup when scenes are unloaded

**Audio Processing**
- Real-time fade effects with precise timing
- 3D spatial audio with distance attenuation
- Master volume controls with separate mixing channels

## Core Components

### Public API Layer
- **File**: `include/soundcoe.hpp`
- **Purpose**: Provides clean, game-developer-friendly static functions
- **Key Features**:
  - Complete black box experience
  - No exposed implementation details
  - Static function API with no object management
  - Single include file for users

### SoundManager (Internal Implementation)
- **File**: `include/soundcoe/playback/sound_manager.hpp`
- **Purpose**: Central singleton managing all audio operations
- **Design Pattern**: Thread-safe singleton with comprehensive state management

#### State Management
SoundManager uses instance variables with thread-safe access patterns and proper mutex protection for all state management.

#### ActiveAudio Structure
```cpp
struct ActiveAudio {
    size_t m_sourceIndex;
    std::string m_filename;
    float m_baseVolume, m_basePitch;
    bool m_loop;
    bool m_isFading;
    float m_fadeDuration, m_fadeElapsed;
};
```

### ResourceManager
- **File**: `include/soundcoe/resources/resource_manager.hpp`
- **Purpose**: Manages OpenAL resources with pooling and caching
- **Key Features**:
  - Source pool with priority-based allocation
  - Intelligent buffer caching with size limits
  - Directory-to-scene mapping
  - Automatic resource cleanup

### Core Layer
- **AudioContext**: OpenAL device and context management
- **ErrorHandler**: Comprehensive error checking and reporting
- **Types**: 3D math (Vec3) and audio enumerations

## Data Flow

### 1. Initialization Process
```
soundcoe::initialize() called
    ↓
Acquire SoundManager mutex
    ↓
Initialize ResourceManager pools
    ↓
Initialize AudioContext (OpenAL)
    ↓
Set default configurations
    ↓
Release mutex
```

### 2. Audio Playback Process
```
soundcoe::playSound() called
    ↓
SoundManager::playSound() internal function
    ↓
Acquire mutex lock
    ↓
Acquire source from ResourceManager pool
    ↓
Load/retrieve buffer from cache
    ↓
Configure OpenAL source properties
    ↓
Start playback and create handle
    ↓
Store in active audio map
    ↓
Release mutex lock
```

### 3. Scene Management Process
```
soundcoe::preloadScene() called
    ↓
Map scene name to directory paths
    ↓
ResourceManager::preloadDirectory()
    ↓
Scan directory for audio files
    ↓
Load and cache audio buffers
    ↓
Update loaded directories list
```

## Thread Safety Implementation

### Mutex Strategy
- **Per-Class Mutex**: Each class (SoundManager, ResourceManager, AudioContext) has its own std::mutex member
- **Lock Scope**: Every public method acquires its class mutex for the entire duration
- **Benefits**:
  - Fine-grained locking reduces contention between different subsystems
  - No deadlock risk due to careful lock ordering
  - Atomic operations within each subsystem

### Performance Considerations
- **Lock Granularity**: Coarse-grained locking for simplicity
- **Lock Duration**: Minimal time holding locks for audio operations
- **OpenAL Serialization**: All OpenAL calls properly serialized

### Thread Safety Guarantees
1. **Handle Consistency**: All threads see consistent handle states
2. **Audio Integrity**: No interleaved or corrupted audio operations
3. **Resource Safety**: No concurrent access to OpenAL objects
4. **State Atomicity**: Configuration changes are atomic

## Cross-Platform Considerations

### OpenAL Backend Selection
Backend selection is handled automatically by the CMake configuration system. Users can optionally configure specific backends using CMake options:

- **Windows**: Use `SOUNDCOE_ENABLE_WASAPI`, `SOUNDCOE_ENABLE_DSOUND`, etc.
- **Linux**: Use `SOUNDCOE_ENABLE_ALSA`, `SOUNDCOE_ENABLE_PULSEAUDIO`, etc.
- **macOS**: Automatic CoreAudio configuration (no user options needed)
- **WebAssembly**: Automatic NULL backend configuration (no user options needed)

### Audio Format Support
- **WAV**: Cross-platform with dr_wav decoder
- **MP3**: Universal compatibility with dr_mp3
- **OGG**: Open-source format with stb_vorbis

### Build System Integration
- **CMake**: FetchContent for automatic dependency management
- **Static Linking**: Self-contained builds with embedded libraries
- **Modular Configuration**: Platform-specific OpenAL backend management via `cmake/openal_config.cmake`
- **Automatic Platform Detection**: CMake platform variables (WIN32, APPLE, UNIX) and CMAKE_SYSTEM_NAME for Emscripten

## Platform Support & Audio Backends

soundcoe uses a sophisticated CMake configuration system that automatically selects appropriate audio backends for each platform.

### Supported Platforms

#### Windows
- **Primary Backends**: WASAPI (Vista+), DirectSound, WinMM
- **Optional Backends**: PortAudio
- **Configuration**: User-configurable via `SOUNDCOE_ENABLE_*` CMake options
- **Testing**: Math-only CI coverage due to audio hardware limitations

#### macOS
- **Backend**: CoreAudio (optimal for Apple platforms)
- **Configuration**: Automatic, no user configuration needed
- **Testing**: Full CI coverage with Apple Clang

#### Linux/Unix
- **Primary Backends**: ALSA, PulseAudio, PipeWire (modern Linux audio stack)
- **Optional Backends**: OSS, JACK, PortAudio
- **Configuration**: User-configurable via `SOUNDCOE_ENABLE_*` CMake options
- **Testing**: Full CI coverage with GCC and Clang, virtual audio setup

#### WebAssembly/Emscripten
- **Backend**: NULL backend (no audio output, for build validation)
- **Configuration**: Automatic detection via `CMAKE_SYSTEM_NAME=Emscripten`
- **Testing**: Multi-host build validation (Ubuntu, Windows, macOS → WebAssembly)
- **Limitations**: Tests auto-disabled due to testcoe compatibility issues

#### Cross-Platform Backends
- **SDL2/SDL3**: Available as alternatives for projects using SDL framework
- **WAVE**: File output backend for testing and debugging
- **PortAudio**: Cross-platform audio I/O library

### Backend Configuration Options

You can customize which audio backends are enabled using CMake options:

#### Windows Configuration
```bash
# Enable specific Windows backends
cmake -B build \
  -DSOUNDCOE_ENABLE_WASAPI=ON \
  -DSOUNDCOE_ENABLE_DSOUND=ON \
  -DSOUNDCOE_ENABLE_WINMM=OFF \
  -DSOUNDCOE_ENABLE_PORTAUDIO=OFF
```

#### Linux Configuration
```bash
# Enable specific Linux backends
cmake -B build \
  -DSOUNDCOE_ENABLE_ALSA=ON \
  -DSOUNDCOE_ENABLE_PULSEAUDIO=ON \
  -DSOUNDCOE_ENABLE_PIPEWIRE=ON \
  -DSOUNDCOE_ENABLE_OSS=OFF \
  -DSOUNDCOE_ENABLE_JACK=OFF \
  -DSOUNDCOE_ENABLE_PORTAUDIO=OFF
```

**Notes:**
- **macOS/WebAssembly**: No configuration options - backends are selected automatically
- **Default values**: Most common backends are enabled by default on each platform
- **Multiple backends**: You can enable multiple backends - OpenAL will choose the best available at runtime

## Memory Management

### Static Storage
- **Lifetime**: All state stored within a static SoundManager instance
- **Initialization**: Lazy initialization on first API call
- **Cleanup**: Explicit cleanup through `shutdown()`

### Resource Management
- **RAII Design**: Automatic cleanup through destructors
- **Shared Buffers**: Shared buffer ownership with reference counting
- **Pool Allocation**: Pre-allocated source pools reduce dynamic allocation

### Caching Strategy
- **LRU Eviction**: Least recently used buffers removed first
- **Size Limits**: Configurable maximum cache size in MB
- **Usage Tracking**: Statistical data for optimization decisions

## Audio Format Integration
- **dr_wav**: High-quality WAV decoding with comprehensive format support
- **dr_mp3**: Efficient MP3 decoding for compressed audio
- **stb_vorbis**: OGG Vorbis support for open-source compression

## 3D Spatial Audio

soundcoe provides 3D spatial audio through OpenAL's built-in capabilities, with listener positioning handled internally by the SoundManager.

### OpenAL 3D Features
- **Source Positioning**: 3D coordinates for each audio source
- **Listener Orientation**: Position, velocity, and orientation vectors
- **Distance Attenuation**: Automatic volume reduction with distance
- **Doppler Effect**: Pitch shifting based on relative velocity
```

## Real-Time Effects System

soundcoe provides real-time audio effects processing, with fade effects handled during the update cycle.

### Effect Types
- **Fade In**: Gradual volume increase from 0 to target
- **Fade Out**: Gradual volume decrease to 0, then stop
- **Fade to Volume**: Transition between any two volume levels

## Error Handling

soundcoe provides comprehensive error handling with detailed logging integration.

### Integration with logcoe
- **Structured Logging**: Consistent error reporting format
- **Context Information**: File names, operations, and parameters
- **Debug Tracing**: Detailed operation logging at debug level

## Performance Characteristics

### Time Complexity
- **Audio Playback**: O(1) for handle operations
- **Scene Loading**: O(n) where n is number of files in directory
- **Resource Lookup**: O(1) hash table lookups for cached resources

### Space Complexity
- **Memory Usage**: Configurable cache size limits
- **Resource Pools**: Fixed-size pools prevent memory growth
- **Handle Storage**: Minimal overhead per active audio instance

### Optimization Strategies
- **Resource Pooling**: Prevents runtime allocation during gameplay
- **Intelligent Caching**: Keeps frequently used audio in memory
- **Priority System**: Ensures important audio plays when resources are limited