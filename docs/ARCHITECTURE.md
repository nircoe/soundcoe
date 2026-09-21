# soundcoe Architecture

## Overview

This document covers how soundcoe works internally, backend setup, and a few advanced usage examples.

## Advanced Usage Examples

### 3D Spatial Audio

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
auto engine = soundcoe::playSound3D("car_engine.wav", carPos, soundcoe::Vec3::zero(), 0.8f, 1.0f, true);

// Update positions in your game loop
soundcoe::Vec3 updatedCarPos(-5.0f, 0.0f, 10.0f);
soundcoe::setSoundPosition(engine, updatedCarPos);
```

### Advanced Fade Effects

```cpp
auto musicHandle = soundcoe::fadeInMusic("battle_theme.ogg", 3.0f);
soundcoe::fadeToVolumeMusic(musicHandle, 0.3f, 1.5f);  // Fade to 30% over 1.5 seconds
soundcoe::fadeOutMusic(musicHandle, 2.0f);
```

### Master Volume Controls

```cpp
soundcoe::setMasterVolume(0.8f);
soundcoe::setMasterSoundsVolume(0.9f);
soundcoe::setMasterMusicVolume(0.4f);

// Muting preserves volume levels
soundcoe::muteAll();
soundcoe::unmuteAllSounds();             // Unmute sounds only
soundcoe::unmuteAllMusic();              // Unmute music only
```

## Internal Architecture

### Architecture Overview

soundcoe hides OpenAL behind a simple free-function API:

```
Game Code → soundcoe API → Internal Systems → OpenAL
```

## Core Components

### Public API Layer
- **File**: `include/soundcoe.hpp`
- **Purpose**: Free functions in the `soundcoe` namespace, forwarding to a `SoundManager` singleton

### SoundManager (Internal Implementation)
- **File**: `include/soundcoe/playback/sound_manager.hpp`
- **Purpose**: Central singleton managing all audio operations

All state is instance data protected by a mutex. `update()` runs fade handling, then cleans up inactive audio.
A fade out stops the source when the volume reaches 0.

### ResourceManager
- **File**: `include/soundcoe/resources/resource_manager.hpp`
- **Purpose**: Manages OpenAL resources with pooling and caching
- Source pool: when no source is free, a stopped source is reused. Otherwise the lowest-priority, oldest source is
  stopped and taken, unless every source outranks the new sound
- Buffer cache with a size limit
- Scene directory loading and unloading
- Audio decoders in `src/resources/audio_data.cpp`: WAV via dr_wav, MP3 via dr_mp3, OGG via stb_vorbis

### Core Layer
- **AudioContext**: OpenAL device and context management
- **ErrorHandler**: Error checking and reporting
- **Types**: 3D math (Vec3) and audio enumerations

## Data Flow

### 1. Initialization Process
```
soundcoe::initialize() called
    ↓
Lock, init logging, validate root dir
    ↓
Init OpenAL context (ResourceManager::initialize -> AudioContext::initialize)
    ↓
Create source pool, set listener gain
    ↓
Preload general/ if it exists
```

### 2. Audio Playback Process
```
soundcoe::playSound() called
    ↓
Get buffer from cache (getBuffer)
    ↓
Acquire source from the pool (acquireSource)
    ↓
Configure source and start playback
    ↓
Store handle in active audio map
```

### 3. Scene Management Process
```
soundcoe::preloadScene() called
    ↓
The scene name is used directly as a subdirectory of the audio root
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
- SoundManager, ResourceManager and AudioContext each have one `std::mutex`
- Every public method locks its class mutex for the whole call
- Locks nest in one direction: SoundManager, then ResourceManager, then AudioContext.
  Locks are held for the whole call, including file decode on a cache miss.

## Platform Support & Audio Backends

soundcoe uses a CMake configuration system that automatically selects audio backends for each platform.
Dependencies come from FetchContent, with backend setup in `cmake/openal_soft.cmake`.
OpenAL-Soft is linked dynamically (not on Emscripten, see below). dr_libs and stb are embedded.

### Supported Platforms

#### Windows
- **Primary Backends**: WASAPI, DirectSound, WinMM
- **Optional Backends**: PortAudio
- **Configuration**: User-configurable via `SOUNDCOE_ENABLE_*` CMake options

#### macOS
- **Backend**: CoreAudio (native to Apple platforms)
- **Configuration**: Automatic, no user configuration needed

#### Linux/Unix
- **Primary Backends**: ALSA, PulseAudio, PipeWire
- **Optional Backends**: OSS, JACK, PortAudio
- **Configuration**: User-configurable via `SOUNDCOE_ENABLE_*` CMake options
- **Other UNIX**: FreeBSD and similar systems use OpenAL-Soft backend autodetection

#### WebAssembly/Emscripten
- **Backend**: Emscripten's built-in OpenAL (WebAudio). OpenAL-Soft is not fetched, so there is no NULL backend
- **Configuration**: Automatic detection via `CMAKE_SYSTEM_NAME=Emscripten`

#### Other Backends
SDL2/SDL3 and WAVE have no `SOUNDCOE_ENABLE_*` options. At the moment they are reachable only with
`SOUNDCOE_AUTODETECT_BACKENDS=ON`.

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

Options can also be set with `set()` before soundcoe is added. This works the same when soundcoe is pulled in by
another library such as gamecoe, as long as you set them before adding that library:
```cmake
set(SOUNDCOE_ENABLE_PULSEAUDIO OFF)
FetchContent_MakeAvailable(soundcoe)  # or gamecoe
```

#### Headless / autodetect modes
```bash
# Null backend only, no audio output (used by CI)
cmake -B build -DSOUNDCOE_ONLY_NULL_BACKEND=ON
ALSOFT_DRIVERS=null ./build/tests/soundcoe_tests
```
`ALSOFT_DRIVERS=null` is needed because OpenAL-Soft skips the null backend by default.
`SOUNDCOE_AUTODETECT_BACKENDS=ON` lets OpenAL-Soft pick whatever backends it finds.
The two options cannot both be ON.

**Notes:**
- **macOS/WebAssembly**: No configuration options, backends are selected automatically

## Memory Management

- The singleton is constructed on first use. Call `initialize()` before anything else, calls made before it fail
  with a not-initialized error.
- Cleanup is explicit through `shutdown()`.
- The cache owns buffers (`unique_ptr`). A manual reference count tracks sources using each one.
- Source pools are pre-created, so playback does not create sources. Buffers are still decoded at runtime on a
  cache miss. Only the source pool is fixed in size, buffer memory is unbounded unless a cache limit is set.
- Eviction prefers unused buffers (refcount 0), then lower-priority, then least recently accessed. Buffers still in
  use can be evicted and their sources are detached.

## Error Handling

Logging goes through logcoe (optional, `SOUNDCOE_USE_LOGCOE`). Messages are prefixed `Class::method`.
Failures return false or invalid handles, lower layers throw via ErrorHandler.
