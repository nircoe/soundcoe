# Contributing to soundcoe

Thank you for contributing to soundcoe!

## Quick Start

```bash
# 1. Fork soundcoe on GitHub to your account
# 2. Clone your fork
git clone https://github.com/YOUR_USERNAME/soundcoe.git
cd soundcoe

# 3. Build with tests
cmake -B build -DSOUNDCOE_BUILD_TESTS=ON
cmake --build build

# 4. Run tests
./build/tests/soundcoe_tests
```

## Testing

```bash
# Run all tests
./build/tests/soundcoe_tests

# Run specific test suite
./build/tests/soundcoe_tests --suite=SoundManagerTests

# Run specific test
./build/tests/soundcoe_tests --test=SoundManagerTests.FadeInSound
```

## Project Structure

```
soundcoe/
├── include/
│   ├── soundcoe.hpp                # Public black box API
│   └── soundcoe/
│       ├── core/                   # AudioContext, ErrorHandler, Types
│       ├── resources/              # ResourceManager, SoundBuffer, SoundSource
│       ├── utils/                  # Math utilities
│       └── playback/               # SoundManager singleton
├── src/                            # Implementation files
├── external/                       # Third-party libraries (dr_libs, stb)
├── tests/                          # Comprehensive test suite
└── docs/                          # Documentation
```

## Continuous Integration

All pull requests are automatically tested on Windows, Linux, and macOS with multiple compilers.

**Windows CI Limitation**: Windows CI runs only math tests (Vec3Tests, MathTests) due to lack of audio backend support in GitHub Actions runners. Full audio testing requires local development machines.

**Note**: Always test actual audio playback locally before submitting pull requests.

## Making Changes

### Code Style
- Use 4 spaces for indentation (no tabs)
- Follow existing naming conventions:
  - File names: `snake_case`
  - Classes: `PascalCase`  
  - Functions/locals: `camelCase`
  - Members: `m_camelCase`
  - Statics: `s_camelCase`
  - Globals: `g_camelCase`
- Keep lines under 120 characters
- Add comments for complex audio algorithms
- Use `const` and `constexpr` where appropriate

### Example Code Style
```cpp
// Good
namespace soundcoe {
    namespace detail {
        class SoundManager {
        private:
            mutable std::mutex m_mutex;
            std::unordered_map<SoundHandle, ActiveAudio> m_activeSounds;
            
        public:
            SoundHandle playSound(const std::string& filename, float volume = 1.0f) {
                std::lock_guard<std::mutex> lock(m_mutex);
                return play(m_activeSounds, filename, volume, ...);
            }
        };
    }
}

// Avoid
namespace soundcoe{
class SoundManager{
SoundHandle playSound(string filename,float volume){
std::lock_guard<std::mutex> lock(mutex);
return play(activeSounds,filename,volume);
}
};
```

### Testing Guidelines
- Add tests for new features in appropriate test files
- Ensure thread safety tests pass for concurrent operations
- Test edge cases and error conditions
- **CI testing**: Focus on API functionality, resource management, and mathematical calculations
- **Local testing required**: Verify actual audio playback and format compatibility on development machines
- Include tests for 3D spatial audio calculations
- Test scene management and resource cleanup

### Pull Request Process

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-audio-feature`)
3. Make your changes following the code style
4. Add/update tests as needed
5. Run tests locally and ensure they pass
6. Commit with clear, descriptive messages following the [Component]: format
7. Push to your fork (`git push origin feature/amazing-audio-feature`)
8. Open a Pull Request from your fork to the main repository with title matching the commit format

### Commit Messages

Follow this format for consistency across the project:

**Format:**
```
[Component]: Brief description (#PR_NUMBER)

- Add/Implement specific feature 1
- Add/Implement specific feature 2  
- Add comprehensive testing coverage
```

**Style Rules:**
- **Header**: `[Component]: Action description (#PR_NUMBER)`
- **Components**: Core, Resources, Playback, API, CI, Build, Docs, Production, Fix, etc.
- **Action verbs**: "Add", "Implement", "Fix", "Update", "Remove"
- **Bullet points**: Group related features, start with action verbs
- **Technical details**: Include key technical aspects and architecture decisions
- **Testing**: Always mention testing coverage when applicable

**Examples:**
```
[Core]: Initialize audio system foundation (#1)
[Resources]: Implement resource management and utilities (#2)  
[Playback]: Implement complete audio playback system and public API (#3)
[CI]: Add GitHub Actions workflow for automated testing and builds (#4)
[API]: Fix static function definitions in soundcoe.cpp (#6)
[Production]: Make soundcoe production-ready for external projects (#8)
```

### Pull Request Titles

PR titles should match the commit message format:
- Use the same `[Component]: Brief description` format
- PR title will become the merge commit message
- Keep titles concise and descriptive

**Example PR titles:**
- `[API]: Add streaming support for large audio files`
- `[Core]: Fix memory leak in resource manager`
- `[Docs]: Update installation instructions`

## Platform Testing

Test your changes locally on your development platform. The CI will handle cross-platform build verification.

## Adding New Features

1. **Add to public API**: Update `include/soundcoe.hpp` with static wrapper functions
2. **Implement internally**: Add to appropriate layer (Core/Resources/Playback) with thread safety
3. **Add tests**: Include functional and thread safety tests
4. **Update docs**: Add examples to README.md or ARCHITECTURE.md if needed

## Thread Safety Guidelines

When modifying soundcoe:

1. **Always Use Mutex**: Every function that accesses shared state must lock the appropriate mutex
2. **Minimize Lock Duration**: Perform operations efficiently under lock
3. **Avoid Nested Locks**: Each class uses its own mutex to prevent cross-class deadlocks
4. **Test Concurrency**: Add thread safety tests for new functionality

### Thread Safety Checklist
- [ ] Function acquires appropriate mutex before accessing shared state
- [ ] Lock scope covers all state modifications
- [ ] No blocking operations while holding lock
- [ ] Thread safety test added for new functionality

## Debugging Tips

### Common Issues
- **Build Failures**: Check C++17 compiler compatibility and CMake version
- **Test Failures**: Verify audio file permissions and format validity
- **Thread Issues**: Use thread sanitizer when available
- **Audio Problems**: Check OpenAL backend and driver compatibility

### Debugging Tools
```bash
# Build with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Run with thread sanitizer (GCC/Clang)
cmake -DCMAKE_CXX_FLAGS="-fsanitize=thread" ..

# Run with address sanitizer
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address" ..
```

### Debugging Audio Issues
- Use different audio formats to isolate decoder problems
- **Local testing required**: Test with different OpenAL backends on development machines
- Check system audio permissions and hardware availability
- Use logging to trace resource allocation
- **Note**: CI systems are headless - actual audio playback must be tested locally

## Performance Considerations

When contributing:

- **Minimize Lock Contention**: Keep critical sections small
- **Avoid Dynamic Allocation**: Use object pools during gameplay
- **Efficient Audio Loading**: Consider streaming for large files
- **OpenAL Efficiency**: Batch operations when possible

## Documentation Standards

- Update README.md for user-facing changes
- Add inline comments for complex algorithms
- Update architecture documentation for internal changes
- Include examples for new API functions
- Maintain consistent documentation style

## Questions?

Feel free to open an issue for:
- Bug reports with reproduction steps
- Feature requests with game development use cases
- Questions about the codebase
- Discussion about implementation approaches
- Help with development setup

We're here to help make contributing to soundcoe as smooth as possible!