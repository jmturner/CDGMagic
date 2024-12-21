#!/bin/bash

# Exit on error
set -e

# Set default values for optional variables
CC=${CC:-gcc}
CXX=${CXX:-g++}
CFLAGS=${CFLAGS:-"-Wall -O2"}
CXXFLAGS=${CXXFLAGS:-"-Wall -O2"}
LDFLAGS=${LDFLAGS:-""}

# Check for required dependencies
echo "Checking for required dependencies..."

# Check if FLTK is installed
if ! pkg-config --exists fltk; then
    echo "FLTK development libraries not found. Please install libfltk1.3-dev."
    exit 1
fi

# Check if PortAudio is installed
if ! pkg-config --exists portaudio-2.0; then
    echo "PortAudio development libraries not found. Please install portaudio19-dev."
    exit 1
fi

# Set up directories
echo "Setting up build directories..."
mkdir -p build
cd build

# Configure the build system (you can modify the flags based on your needs)
echo "Configuring the project using GCC..."
../configure CC=$CC CXX=$CXX CFLAGS="$CFLAGS" CXXFLAGS="$CXXFLAGS" LDFLAGS="$LDFLAGS"

echo "Configuration completed successfully."

# Finish by returning to the project root directory
cd ..
