# soundcoe - Modern C++ Sound Library for Game Development

## Overview

**soundcoe** is a modern C++ sound library built on OpenAL, designed specifically for game development. This library provides a clean, efficient, and intuitive API for handling audio in games, from simple 2D sound effects to more complex 3D audio environments.

Part of the "gamecoe" series of game development tools, soundcoe focuses on simplicity, performance, and resource efficiency.

## Features

- Load and play various audio formats (WAV, OGG)
- Simple API for sound effects and music
- Spatial audio positioning for 3D environments
- Resource pooling and efficient memory management
- Volume, pitch, and playback controls
- Sound prioritization system
- Modern C++ design with minimal dynamic allocation

## Architecture

### Directory Structure

```
soundcoe/
├── include/
│   └── soundcoe/
│       ├── core/
│       │   ├── audio_context.hpp
│       │   ├── error_handler.hpp
│       │   └── types.hpp
│       ├── resources/
│       │   ├── sound_buffer.hpp
│       │   ├── sound_source.hpp
│       │   └── resource_manager.hpp
│       ├── playback/
│       │   ├── sound_player.hpp
│       │   ├── sound_effect.hpp
│       │   └── music.hpp
│       ├── utils/
│       │   ├── audio_file.hpp
│       │   └── math.hpp
│       └── soundcoe.hpp  // Main include file
├── src/
│   └── [implementation files matching the include structure]
├── examples/
│   ├── basic_playback/
│   ├── 3d_positioning/
│   └── effects_demo/
└── tests/
    ├── core_tests/
    ├── playback_tests/
    └── resources_tests/
```

### Core Components

#### Core Layer

**AudioContext**
- Manages OpenAL device and context initialization
- Singleton class for global access
- Handles lifetime of OpenAL system
- Provides error checking context

**ErrorHandler**
- Provides error checking and reporting for OpenAL operations
- Converts error codes to human-readable messages
- Centralizes error handling and reporting

**Types**
- Contains enums and structs used throughout the library
- SoundState (INITIAL, PLAYING, PAUSED, STOPPED)
- SoundPriority (LOW, MEDIUM, HIGH, CRITICAL)
- Vec3 structure for 3D positioning

#### Resource Layer

**SoundBuffer**
- Manages OpenAL buffer objects
- Loads audio data from files
- Supports different formats (WAV, OGG)
- Tracks buffer properties (format, size, frequency, duration)
- Manages buffer lifecycle

**SoundSource**
- Wraps OpenAL source objects
- Controls individual sound properties (position, volume, pitch)
- Manages playback state
- Provides 3D audio controls
- Handles source lifecycle

**ResourceManager**
- Manages pools of buffers and sources
- Handles resource allocation and deallocation
- Implements priority-based source allocation
- Ensures optimal resource usage
- Caches loaded sounds for reuse

#### Playback Layer

**SoundEffect**
- High-level interface for one-shot sound effects
- Manages buffer and source lifecycle
- Provides simple play/stop/pause controls
- Supports 3D positioning
- Handles looping and other properties

**Music**
- Specialized class for longer audio tracks
- Supports streaming for large files
- Provides additional controls like fading
- Handles looping and playback position

**SoundPlayer**
- Main interface for playing sounds
- Manages global audio settings
- Controls listener properties for 3D audio
- Handles groups of sounds
- Global mute/volume/pause controls

#### Utils Layer

**AudioFile**
- Handles loading and parsing of different audio file formats
- Provides uniform interface for accessing audio data
- Decodes WAV, OGG, MP3 formats to raw PCM data
- Manages file operations

**Math**
- Audio-specific math functions
- Vector operations for spatial audio
- Utilities for audio processing (dB conversion, etc.)

## Coding Conventions

- File names: snake_case
- Classes: PascalCase
- Functions, local variables: camelCase
- Class member variables: m_camelCase
- Static variables: s_camelCase
- Global variables: g_camelCase

## Memory Management Strategy

- Minimize dynamic memory allocation for efficiency
- Use fixed-size arrays and pools where possible
- Self-contained resource management
- Clear ownership hierarchies
- RAII principles for resource management
- Handle resource deallocation automatically
- Encapsulate OpenAL resources from users

## Dependencies

- OpenAL - For audio playback and 3D positioning
- (Optional) libvorbis - For OGG format support
- (Optional) libmpg123 - For MP3 format support

## Testing

The library uses GoogleTest for unit testing:
- Unit tests for each component
- Mock objects for OpenAL abstractions
- Silent tests with null devices for CI/CD

## Roadmap

### Phase 1: Foundation
- Basic audio context management
- WAV file loading and playback
- Simple sound effects system

### Phase 2: Resource Management
- Resource pooling and caching
- Priority-based allocation
- Memory usage optimization

### Phase 3: Advanced Features
- 3D positioning and spatialization
- OGG/MP3 support
- Streaming for music

### Future Features
- DSP effects
- Advanced 3D audio features
- Environmental audio effects

## Usage Example

```cpp
#include <soundcoe/soundcoe.hpp>

int main() {
    // Initialize the sound system
    soundcoe::AudioContext::getInstance().initialize();
    soundcoe::ResourceManager::getInstance().initialize();
    
    // Play a sound effect
    auto effect = soundcoe::SoundPlayer::getInstance().playSound("explosion.wav");
    
    // Play background music
    auto music = soundcoe::SoundPlayer::getInstance().playMusic("background.ogg", 0.8f, true);
    
    // Set 3D listener position
    soundcoe::SoundPlayer::getInstance().setListenerPosition({0.0f, 0.0f, 0.0f});
    
    // Play a 3D positioned sound
    auto effect3d = soundcoe::SoundPlayer::getInstance().playSound("footstep.wav", 
                                                              1.0f, 1.0f, false, 
                                                              {10.0f, 0.0f, 5.0f});
    
    // ... game loop ...
    
    // Clean up
    soundcoe::SoundPlayer::getInstance().shutdown();
    soundcoe::ResourceManager::getInstance().shutdown();
    soundcoe::AudioContext::getInstance().shutdown();
    
    return 0;
}
```

## License

[Your license information here]

## Author

[Your information here]