# 2D Game Engine

A lightweight, testable 2D game engine now using GLFW + BGFX

## Features

- ✅ Clean architecture with light abstraction layer
- ✅ Unit tested from day one (Catch2)
- ✅ Support for side-scroller and top-down games
- ✅ Cross-platform (Windows, macOS, Linux)

## Prerequisites

- CMake 3.20+
- C++20 compatible compiler (GCC 11+, Clang 12+, MSVC 2019+)
- Git
*(All third-party libs are fetched automatically: GLFW, BGFX/bx/bimg, GLM, spdlog, Catch2, nlohmann/json, stb headers.)*

## Building
```bash
git clone https://github.com/yourusername/game-engine.git
cd game-engine

cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build

# Run engine
./build/Engine

# Run tests
ctest --test-dir build --output-on-failure
```

## Notes
- Linux: prefers X11; if X11 is missing but GLFW is Wayland-enabled, BGFX falls back to Wayland handles.
- Shaders are compiled via `compile_shaders` target and copied next to the executable.
- Tests live in `/tests` (Catch2). Assets for tests/examples are under `/assets/test_data`.

## Development Workflow

1. Write tests first (TDD encouraged)
2. Implement feature
3. Run `ctest` to verify
4. Commit with passing tests

## Testing

The project uses [Catch2](https://github.com/catchorg/Catch2) for unit testing.

Run specific tests:
```bash
./tests/EngineTests "[transform]"
./tests/EngineTests "[collision]"
```

Verbose output:
```bash
./tests/EngineTests -s
```

## Roadmap

- [x] Project skeleton with CMake
- [x] Transform system with tests
- [x] Time management
- [x] Basic collision math
- [ ] Texture atlas system
- [ ] Entity management
- [ ] Camera system
- [ ] Physics (platformer & top-down)
- [ ] Tile map renderer
- [ ] Input buffering (coyote time, jump buffering)

## Contributing

This is a learning project, but contributions welcome!

## License

MIT License - see LICENSE file