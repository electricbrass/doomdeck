FROM ubuntu:26.04
# TODO: maybe add appimagetool?
ARG TARGETARCH

RUN apt-get update && apt-get install -y \
    wget \
    lsb-release \
    clang \
    cmake \
    build-essential \
    ninja-build \
    git \
    patchelf \
    libsdl3-dev \
    libcatch2-dev \
    libfreetype-dev \
    && rm -rf /var/lib/apt/lists/*

# RUN if [ "$TARGETARCH" = "amd64" ]; then \
#         URL="https://github.com/Kitware/CMake/releases/download/v4.4.0/cmake-4.4.0-linux-x86_64.sh"; \
#     elif [ "$TARGETARCH" = "arm64" ]; then \
#         URL="https://github.com/Kitware/CMake/releases/download/v4.4.0/cmake-4.4.0-linux-aarch64.sh"; \
#     else \
#         echo "Unsupported arch: $TARGETARCH" && exit 1; \
#     fi && \
#     mkdir -p /opt/cmake && \
#     wget -qO /tmp/cmake-install.sh "$URL" && \
#     sh /tmp/cmake-install.sh --skip-license --prefix=/opt/cmake --exclude-subdir && \
#     rm /tmp/cmake-install.sh

# ENV PATH="/opt/cmake/bin:${PATH}"

# RUN wget -qO- https://apt.llvm.org/llvm.sh | bash -s -- 22

RUN if [ "$TARGETARCH" = "amd64" ]; then \
        URL="https://github.com/linuxdeploy/linuxdeploy/releases/latest/download/linuxdeploy-x86_64.AppImage"; \
    elif [ "$TARGETARCH" = "arm64" ]; then \
        URL="https://github.com/linuxdeploy/linuxdeploy/releases/latest/download/linuxdeploy-aarch64.AppImage"; \
    else \
        echo "Unsupported arch: $TARGETARCH" && exit 1; \
    fi && \
    wget -qO /tmp/linuxdeploy.AppImage "$URL" && \
    chmod +x /tmp/linuxdeploy.AppImage && \
    /tmp/linuxdeploy.AppImage --appimage-extract && \
    mv squashfs-root /opt/linuxdeploy && \
    ln -s /opt/linuxdeploy/AppRun /usr/local/bin/linuxdeploy && \
    rm /tmp/linuxdeploy.AppImage
