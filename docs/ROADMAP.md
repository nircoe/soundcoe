# soundcoe Roadmap

## Implemented Features

- WAV, OGG and MP3 support
- Scene-based audio management with directory organization
- 3D spatial audio with listener and source position and velocity
- Fade effects (fade in/out/to-volume) with real-time processing
- Resource management with pooling, priority allocation, handle-based cleanup and caching
- Cross-platform support (Windows, Linux, macOS, WebAssembly/Emscripten) with OpenAL backend

## Future Plans

### High Priority
- Async operations: Dedicated audio thread with non-blocking scene loading
- Audio streaming: Large music files streamed to reduce memory usage
- Audio effects: Reverb, echo, filtering with real-time processing

### Medium Priority
- Additional audio format support: FLAC, AAC for broader compatibility
- Multi-listener support: Split-screen gaming with multiple audio perspectives

### Future Considerations
- Mobile platform support: iOS and Android support
- Console platform support: PlayStation, Xbox, Nintendo Switch support

## Feature Requests

Have an idea for soundcoe? Please open an issue on GitHub with the "enhancement" label.

## Versioning

soundcoe follows [Semantic Versioning](https://semver.org/) (SemVer).
