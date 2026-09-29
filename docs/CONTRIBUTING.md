# Contributing to soundcoe

## Quick Start

```bash
# Build with tests
cmake -B build -DSOUNDCOE_BUILD_TESTS=ON
cmake --build build

# Run tests
./build/tests/soundcoe_tests
./build/tests/soundcoe_tests --suite=SoundManagerTests
./build/tests/soundcoe_tests --test=SoundManagerTests.FadeInSound
```

## Project Structure

```
soundcoe/
├── include/
│   ├── soundcoe.hpp                # Public black box API
│   └── soundcoe/
│       ├── core/                   # AudioContext, ErrorHandler, Types
│       ├── resources/              # resource_manager, sound_buffer, sound_source
│       ├── utils/                  # Math utilities
│       └── playback/               # sound_manager singleton
├── src/                            # Implementation files
├── cmake/                          # Modular CMake configuration (openal_soft.cmake, etc.)
├── external/                       # Third-party libraries (dr_libs, stb)
├── tests/                          # Tests
└── docs/                          # Documentation
```

## Continuous Integration

- Linux and Windows run the full suite headlessly with `-DSOUNDCOE_ONLY_NULL_BACKEND=ON` and `ALSOFT_DRIVERS=null`,
  because OpenAL-Soft skips the null backend by default.
- macOS uses real CoreAudio.
- Web is build-only. Tests are not built (testcoe and backward-cpp conflict).

The null backend has no audible output, so verify playback locally.

## Making Changes

### Code Style
- Indent with 4 spaces (a Tab key that inserts 4 spaces is fine)
- Follow existing naming conventions:
  - File names: `snake_case`
  - Classes/structs/enums (types and enumerators): `snake_case`
  - Functions/locals/parameters: `snake_case`
  - Members: `m_snake_case`
  - Statics: `s_snake_case`
  - Globals: `g_snake_case`

  Sometimes a parameter's natural name collides with its own type name under this all-snake_case
  scheme, like a `vec3` parameter that would naturally be called `vec3`. In that case, add a
  trailing underscore to the parameter name (`vec3_`), never a leading one (those are reserved for
  the compiler) and never an `m_`/`s_`/`g_` prefix (those already mean something else). This
  convention change lines soundcoe up with gamecoe's style, since gamecoe is soundcoe's main
  consumer.
- Keep lines under 120 characters

### Testing Guidelines
- Add tests for new features in the appropriate test files
- Ensure thread safety tests pass for concurrent operations
- Verify actual audio playback locally, CI has no audible output

### Commit Messages

PR title uses the commit format below, it becomes the squashed commit message.

Follow this format for consistency across the project:

Format:
```
[Category]: Brief description

- Add/Implement specific feature 1
- Add/Implement specific feature 2
- Add tests for the new behavior
```

Style rules:
- Header: `[Category]: Action description`
- Categories, for example: API, Build, Feature, Docs, Production, Fix, CI, Core. Use a new one if none fits
- Action verbs: "Add", "Implement", "Fix", "Update", "Remove"
- Bullet points: Group related features, start with action verbs

Examples:
```
[Core]: Initialize audio system foundation (#1)
[Resources]: Implement resource management and utilities (#2)  
[Playback]: Implement complete audio playback system and public API (#3)
[CI]: Add GitHub Actions workflow for automated testing and builds (#4)
[API]: Fix static function definitions in soundcoe.cpp (#6)
[Production]: Make soundcoe production-ready for external projects (#8)
```

## Adding New Features

1. Add to public API: Update `include/soundcoe.hpp` with a free function forwarding to the internal layer
2. Implement internally: Add to appropriate layer (Core/Resources/Playback) with thread safety
3. Add tests: Include functional and thread safety tests
4. Update docs: Update README.md for user-facing changes and ARCHITECTURE.md for internal changes

## Thread Safety Guidelines

When modifying soundcoe:

1. Lock the class mutex in every public method that touches shared state.
2. Keep the lock nesting direction: sound_manager, then resource_manager, then audio_context. Never call back up.

## Questions?

Have a question or an idea? Please open an issue.
