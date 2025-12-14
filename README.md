# 2D Game Engine

A lightweight, testable 2D game engine built with SFML and modern C++17.

## Features

- ✅ Clean architecture with light abstraction layer
- ✅ Unit tested from day one (Catch2)
- ✅ Support for side-scroller and top-down games
- ✅ Cross-platform (Windows, macOS, Linux)

## Prerequisites

- CMake 3.16+
- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- SFML 2.5+ development libraries

### Installing SFML

**Ubuntu/Debian:**
```bash
sudo apt-get install libsfml-dev
```

**macOS (Homebrew):**
```bash
brew install sfml
```

**Windows:**
- Download from [SFML website](https://www.sfml-dev.org/download.php)
- Or use vcpkg: `vcpkg install sfml`

## Building
```bash
# Clone the repository
git clone https://github.com/yourusername/game-engine.git
cd game-engine

# Create build directory
mkdir build && cd build

# Configure
cmake .. -DBUILD_TESTS=ON

# Build
cmake --build .

# Run engine
./src/engine

# Run tests
ctest --output-on-failure
# Or directly:
./tests/EngineTests
```

## Project Structure
```
src/
├── core/          # Core systems (time, transforms, resources)
├── math/          # Math utilities (vectors, rectangles, collision)
├── entities/      # Entity system (coming soon)
└── rendering/     # Rendering systems (coming soon)

tests/
├── core/          # Core system tests
└── math/          # Math tests
```

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