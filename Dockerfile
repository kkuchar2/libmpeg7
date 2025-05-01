FROM ubuntu:22.04

# Avoid prompts during install
ENV DEBIAN_FRONTEND=noninteractive

# Install only what's needed
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    openjdk-21-jdk \
    libopencv-dev \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

# Set JAVA_HOME and PATH
ENV JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64
ENV PATH="$JAVA_HOME/bin:$PATH"

WORKDIR /project