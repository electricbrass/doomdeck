FROM ubuntu:26.04
ARG TARGETARCH

RUN apt-get update && apt-get install -y \
    wget \
    clang \
    cmake \
    build-essential \
    ninja-build \
    git \
    libsdl3-dev \
    libsdl3-image-dev \
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
