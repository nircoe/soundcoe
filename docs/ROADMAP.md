# soundcoe Roadmap

## Version History

### v0.1.0 - Initial Release
- ✅ Thread-safe audio library with complete test suite validation
- ✅ Multiple audio format support (WAV, OGG, MP3) with dr_wav, dr_mp3, stb_vorbis
- ✅ Game developer-focused API with static wrapper functions and intuitive terminology
- ✅ Scene-based audio management with directory organization
- ✅ 3D spatial audio with distance attenuation and doppler effects
- ✅ Advanced fade effects (fade in/out/to-volume) with real-time processing
- ✅ Intelligent resource management with pooling, priority allocation, handle-based cleanup and caching
- ✅ Master volume controls for sounds, music, and overall audio
- ✅ Cross-platform support (Windows, Linux, macOS) with OpenAL backend
- ✅ Comprehensive error handling with detailed logging integration
- ✅ CMake integration with FetchContent support

## Future Plans

### High Priority
- ⏳ **Async operations** - Dedicated audio thread with non-blocking scene loading for maximum game performance
- ⏳ **Audio streaming** - Large music files streamed to reduce memory usage
- ⏳ **Advanced audio effects** - Reverb, echo, filtering with real-time processing

### Medium Priority
- ⏳ **Additional audio format support** - FLAC, AAC for broader compatibility
- ⏳ **Multi-listener support** - Split-screen gaming with multiple audio perspectives

### Future Considerations
- ⏳ **Mobile platform support** - iOS and Android support
- ⏳ **Console platform support** - PlayStation, Xbox, Nintendo Switch support

## Feature Requests

Have an idea for soundcoe? Please open an issue on GitHub with the "enhancement" label.

## Versioning

soundcoe follows [Semantic Versioning](https://semver.org/):
- MAJOR version for incompatible API changes
- MINOR version for backwards-compatible functionality additions
- PATCH version for backwards-compatible bug fixes