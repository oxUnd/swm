#!/bin/bash

# SWM Build Script

set -e

echo "================================"
echo "Building Simple Window Manager"
echo "================================"

# Check if config.h exists
if [ ! -f "config.h" ]; then
    echo "Creating config.h from config.example.h..."
    cp config.example.h config.h
fi

# Clean previous build
echo "Cleaning previous build..."
make clean 2>/dev/null || true

# Build
echo "Building..."
make

# Check if build succeeded
if [ -f "swm" ]; then
    echo ""
    echo "================================"
    echo "Build successful!"
    echo "================================"
    echo ""
    echo "Installation options:"
    echo "  1. Install system-wide: sudo make install"
    echo "  2. Run from current directory: ./swm"
    echo ""
    echo "For testing in Xephyr:"
    echo "  Xephyr -br -ac -noreset -screen 1024x768 :1 &"
    echo "  DISPLAY=:1 ./swm"
    echo ""
    echo "See INSTALL.md for more details."
else
    echo "Build failed!"
    exit 1
fi
