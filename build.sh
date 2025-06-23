#!/bin/bash

# Build script for NotationChordHelper VST3 Plugin

set -e  # Exit on any error

echo "Building NotationChordHelper VST3 Plugin..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
echo "Building project..."
cmake --build . --config Release

# Check if the VST3 file was created
if [ -d "VST3/Release/NotationChordHelper.vst3" ]; then
    echo "✅ Build successful! VST3 file created:"
    echo "   $(pwd)/VST3/Release/NotationChordHelper.vst3"
else
    echo "❌ Build failed! VST3 file not found."
    exit 1
fi

echo "Build completed successfully!" 