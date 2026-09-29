# soundcoe Architecture

## Overview

This document covers how soundcoe works internally, backend setup, and a few advanced usage examples.

## Advanced Usage Examples

### 3D Spatial Audio

```cpp
// Set up listener (usually your player/camera)
soundcoe::vec3 player_pos(0.0f, 1.5f, 0.0f);        // Player position
soundcoe::vec3 player_vel(2.0f, 0.0f, 0.0f);        // Player velocity (for doppler)
soundcoe::vec3 forward(0.0f, 0.0f, -1.0f);         // Looking direction
soundcoe::vec3 up(0.0f, 1.0f, 0.0f);               // Up vector

soundcoe::update_listener(player_pos, player_vel, forward, up);

// Play 3D positioned sounds
soundcoe::vec3 enemy_pos(10.0f, 0.0f, -20.0f);
auto gunshot = soundcoe::play_sound3d("gunshot.wav", enemy_pos);

soundcoe::vec3 car_pos(-5.0f, 0.0f, 15.0f);
auto engine = soundcoe::play_sound3d("car_engine.wav", car_pos, soundcoe::vec3::zero(), 0.8f, 1.0f, true);

// Update positions in your game loop
soundcoe::vec3 updated_car_pos(-5.0f, 0.0f, 10.0f);
soundcoe::set_sound_position(engine, updated_car_pos);
```

### Advanced Fade Effects

```cpp
auto music_handle_ = soundcoe::fade_in_music("battle_theme.ogg", 3.0f);
soundcoe::fade_to_volume_music(music_handle_, 0.3f, 1.5f);  // Fade to 30% over 1.5 seconds
soundcoe::fade_out_music(music_handle_, 2.0f);
```

### Master Volume Controls

```cpp
soundcoe::set_master_volume(0.8f);
soundcoe::set_master_sounds_volume(0.9f);
soundcoe::set_master_music_volume(0.4f);

// Muting preserves volume levels
soundcoe::mute_all();
soundcoe::unmute_all_sounds();             // Unmute sounds only
soundcoe::unmute_all_music();              // Unmute music only
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
- **Purpose**: Free functions in the `soundcoe` namespace, forwarding to a `sound_manager` singleton

### sound_manager (Internal Implementation)
- **File**: `include/soundcoe/playback/sound_manager.hpp`
- **Purpose**: Central singleton managing all audio operations

All state is instance data protected by a mutex. `update()` runs fade handling, then cleans up inactive audio.
A fade out stops the source when the volume reaches 0.

### resource_manager
- **File**: `include/soundcoe/resources/resource_manager.hpp`
- **Purpose**: Manages OpenAL resources with pooling and caching
- Source pool: when no source is free, a stopped source is reused. Otherwise the lowest-priority, oldest source is
  stopped and taken, unless every source outranks the new sound
- Buffer cache with a size limit
- Scene directory loading and unloading
- Audio decoders in `src/resources/audio_data.cpp`: WAV via dr_wav, MP3 via dr_mp3, OGG via stb_vorbis

### Core Layer
- **audio_context**: OpenAL device and context management
- **error_handler**: Error checking and reporting
- **Types**: 3D math (vec3) and audio enumerations

## Data Flow

### 1. Initialization Process
```
soundcoe::initialize() called
    ↓
Lock, init logging, validate root dir
    ↓
Init OpenAL context (resource_manager::initialize -> audio_context::initialize)
    ↓
Create source pool, set listener gain
    ↓
Preload general/ if it exists
```

### 2. Audio Playback Process
```
soundcoe::play_sound() called
    ↓
Get buffer from cache (get_buffer)
    ↓
Acquire source from the pool (acquire_source)
    ↓
Configure source and start playback
    ↓
Store handle in active audio map
```

### 3. Scene Management Process
```
soundcoe::preload_scene() called
    ↓
The scene name is used directly as a subdirectory of the audio root
    ↓
resource_manager::preload_directory()
    ↓
Scan directory for audio files
    ↓
Load and cache audio buffers
    ↓
Update loaded directories list
```

## Thread Safety Implementation

### Mutex Strategy
- sound_manager, resource_manager and audio_context each have one `std::mutex`
- Every public method locks its class mutex for the whole call
- Locks nest in one direction: sound_manager, then resource_manager, then audio_context.
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
Failures return false or invalid handles, lower layers throw via error_handler.
