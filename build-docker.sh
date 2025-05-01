#!/bin/bash
set -e

# Create the necessary directories if they don't exist
mkdir -p java/libs

# Build the project using Docker
docker compose up --build

# Check if the build artifacts exist
if [ -f "./lib/build/x64/Release/libmpeg7.so" ]; then
    echo "Build successful! Found libmpeg7.so"
    
    # Copy the shared library to Java libs directory
    cp ./lib/build/x64/Release/libmpeg7.so ./java/libs/
    echo "Copied libmpeg7.so to ./java/libs/"
    echo "Build completed. Output is in ./lib/build directory"
else
    echo "ERROR: libmpeg7.so was not created in ./lib/build/x64/Release/"
    echo "Check the build logs for errors."
    exit 1
fi