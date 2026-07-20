FROM gcc:trixie
# TODO: maybe add appimagetool?
ARG TARGETARCH
ARG CMAKE_VERSION=4.3.4 # it seems like there's a bug with embed-dir in 4.4.0

RUN apt-get update && apt-get install -y \
    wget \
    lsb-release \
    build-essential \
    ninja-build \
    git \
    patchelf \
    libsdl3-dev \
    libcatch2-dev \
    libfreetype-dev \
    && rm -rf /var/lib/apt/lists/*


RUN if [ "$TARGETARCH" = "amd64" ]; then \
        ARCH="x86_64"; \
    elif [ "$TARGETARCH" = "arm64" ]; then \
        ARCH="aarch64"; \
    else \
        echo "Unsupported arch: $TARGETARCH" && exit 1; \
    fi && \
    URL="https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-${ARCH}.sh"; \
    mkdir -p /opt/cmake && \
    wget -qO /tmp/cmake-install.sh "$URL" && \
    sh /tmp/cmake-install.sh --skip-license --prefix=/opt/cmake --exclude-subdir && \
    rm /tmp/cmake-install.sh

ENV PATH="/opt/cmake/bin:${PATH}"

RUN wget -qO- https://apt.llvm.org/llvm.sh | bash -s -- 22

RUN if [ "$TARGETARCH" = "amd64" ]; then \
        ARCH="x86_64"; \
    elif [ "$TARGETARCH" = "arm64" ]; then \
        ARCH="aarch64"; \
    else \
        echo "Unsupported arch: $TARGETARCH" && exit 1; \
    fi && \
    URL="https://github.com/linuxdeploy/linuxdeploy/releases/latest/download/linuxdeploy-${ARCH}.AppImage"; \
    wget -qO /tmp/linuxdeploy.AppImage "$URL" && \
    chmod +x /tmp/linuxdeploy.AppImage && \
    /tmp/linuxdeploy.AppImage --appimage-extract && \
    mv squashfs-root /opt/linuxdeploy && \
    ln -s /opt/linuxdeploy/AppRun /usr/local/bin/linuxdeploy && \
    rm /tmp/linuxdeploy.AppImage
