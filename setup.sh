#!/bin/bash
set -e

echo "╔════════════════════════════════════════╗"
echo "║    Game Engine - Quick Setup Script   ║"
echo "╚════════════════════════════════════════╝"
echo ""

# Check for SFML
if ! pkg-config --exists sfml-all; then
    echo "⚠️  SFML not found. Please install SFML first."
    echo "   Ubuntu/Debian: sudo apt-get install libsfml-dev"
    echo "   macOS:         brew install sfml"
    exit 1
fi

echo "✓ SFML found"

# Create build directory
mkdir -p build
cd build

echo "Configuring with CMake..."
cmake .. -DBUILD_TESTS=ON

echo "Building..."
cmake --build . -j$(nproc)

echo ""
echo "╔════════════════════════════════════════╗"
echo "║           Build Complete! ✓            ║"
echo "╚════════════════════════════════════════╝"
echo ""
echo "Run the engine:    ./Engine"
echo "Run tests:         ctest --output-on-failure"
echo "Or:                ./EngineTests"
echo ""