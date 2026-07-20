FROM ubuntu:26.04
# I'll need to get a newer cmake from
# https://github.com/Kitware/CMake/releases/download/v4.4.0/cmake-4.4.0-linux-x86_64.sh
# or https://github.com/Kitware/CMake/releases/download/v4.4.0/cmake-4.4.0-linux-aarch64.sh
# and clang from https://apt.llvm.org/
ARG TARGETARCH

RUN apt-get update && apt-get install -y \
    wget \
    clang \
    cmake \
    build-essential \
    ninja-build \
    git \
    libsdl3-dev \
    libcatch2-dev \
    libfreetype-dev \
    rm -rf /var/lib/apt/lists/*

RUN if [ "$TARGETARCH" = "amd64" ]; then \
        URL="https://github.com/linuxdeploy/linuxdeploy/releases/latest/download/linuxdeploy-x86_64.AppImage"; \
    elif [ "$TARGETARCH" = "arm64" ]; then \
        URL="https://github.com/linuxdeploy/linuxdeploy/releases/latest/download/linuxdeploy-aarch64.AppImage"; \
    else \
        echo "Unsupported arch: $TARGETARCH" && exit 1; \
    fi && \
    wget -O /usr/local/bin/linuxdeploy "$URL" && \
    chmod +x /usr/local/bin/linuxdeploy
