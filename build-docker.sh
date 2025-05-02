#!/bin/bash
set -e

# Color definitions for output
COLOR_CYAN="\e[96m"
COLOR_GREEN="\e[92m"
COLOR_RED="\e[91m"
COLOR_RESET="\e[0m"

echo -e "${COLOR_CYAN}Creating the necessary directories...${COLOR_RESET}"
# Create the necessary directories if they don't exist
mkdir -p java/libs

echo -e "${COLOR_CYAN}Building the project using Docker...${COLOR_RESET}"
# Build the project and tag the image for later use
docker compose up --build

# Tag the builder container for reuse by the run script
docker tag $(docker ps -aq --filter "label=com.docker.compose.service=libmpeg7-build" | head -n 1) libmpeg7-build:latest

# Check if the build artifacts exist
LIBMPEG7_SO="./lib/build/x64/Release/libmpeg7.so"
MPEG7_APP_RELEASE="./lib/build/x64/Release/mpeg7_app"
MPEG7_APP_DEBUG="./lib/build/x64/Debug/mpeg7_app"

# Check for library first
if [ -f "$LIBMPEG7_SO" ]; then
    echo -e "${COLOR_GREEN}Build successful! Found libmpeg7.so${COLOR_RESET}"
    
    # Copy the shared library to Java libs directory
    cp "$LIBMPEG7_SO" ./java/libs/
    echo "Copied libmpeg7.so to ./java/libs/"
else
    echo -e "${COLOR_RED}ERROR: libmpeg7.so was not created in $LIBMPEG7_SO${COLOR_RESET}"
    echo "Check the build logs for errors."
    exit 1
fi

# Check for the executable
if [ -f "$MPEG7_APP_RELEASE" ]; then
    echo -e "${COLOR_GREEN}Found release executable: $MPEG7_APP_RELEASE${COLOR_RESET}"
    chmod +x "$MPEG7_APP_RELEASE"
elif [ -f "$MPEG7_APP_DEBUG" ]; then
    echo -e "${COLOR_GREEN}Found debug executable: $MPEG7_APP_DEBUG${COLOR_RESET}"
    chmod +x "$MPEG7_APP_DEBUG"
else
    echo -e "${COLOR_RED}WARNING: mpeg7_app executable was not found.${COLOR_RESET}"
    echo "The shared library was built successfully, but the executable may not be available."
fi

echo -e "${COLOR_GREEN}Build completed. Output is in the ./lib/build directory${COLOR_RESET}"
echo -e "${COLOR_CYAN}You can now run the application using ./run.sh${COLOR_RESET}"

# Make the run script executable
if [ -f "./run.sh" ]; then
    chmod +x ./run.sh
fi