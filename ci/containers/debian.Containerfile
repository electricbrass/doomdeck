FROM docker.io/library/gcc:trixie
ARG TARGETARCH
ARG CMAKE_VERSION=4.4.2
ARG LLVM_VERSION=22

RUN apt-get update && apt-get install -y \
    wget \
    lsb-release \
    build-essential \
    ninja-build \
    git \
    appstream \
    patchelf \
    libsdl3-dev \
    libcatch2-dev \
    libfreetype-dev \
    && rm -rf /var/lib/apt/lists/*

RUN case "$TARGETARCH" in \
        amd64) ARCH="x86_64" ;; \
        arm64) ARCH="aarch64" ;; \
        *) echo "Unsupported arch: $TARGETARCH" && exit 1 ;; \
    esac && \
    URL="https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-${ARCH}.sh"; \
    mkdir -p /opt/cmake && \
    wget -qO /tmp/cmake-install.sh "$URL" && \
    sh /tmp/cmake-install.sh --skip-license --prefix=/opt/cmake --exclude-subdir && \
    rm /tmp/cmake-install.sh

ENV PATH="/opt/cmake/bin:${PATH}"

RUN wget -qO- https://apt.llvm.org/llvm.sh | bash -s -- ${LLVM_VERSION}

ENV PATH="/usr/lib/llvm-${LLVM_VERSION}/bin:${PATH}"

RUN case "$TARGETARCH" in \
        amd64) ARCH="x86_64" ;; \
        arm64) ARCH="aarch64" ;; \
        *) echo "Unsupported arch: $TARGETARCH" && exit 1 ;; \
    esac && \
    URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${ARCH}.AppImage"; \
    wget -qO /tmp/linuxdeploy.AppImage "$URL" && \
    chmod +x /tmp/linuxdeploy.AppImage && \
    /tmp/linuxdeploy.AppImage --appimage-extract && \
    mv squashfs-root /opt/linuxdeploy && \
    ln -s /opt/linuxdeploy/AppRun /usr/local/bin/linuxdeploy && \
    rm /tmp/linuxdeploy.AppImage

RUN case "$TARGETARCH" in \
        amd64) ARCH="x86_64" ;; \
        arm64) ARCH="aarch64" ;; \
        *) echo "Unsupported arch: $TARGETARCH" && exit 1 ;; \
    esac && \
    URL="https://github.com/AppImage/appimagetool/releases/latest/download/appimagetool-${ARCH}.AppImage"; \
    wget -qO /tmp/appimagetool.AppImage "$URL" && \
    chmod +x /tmp/appimagetool.AppImage && \
    /tmp/appimagetool.AppImage --appimage-extract && \
    mv squashfs-root /opt/appimagetool && \
    ln -s /opt/appimagetool/AppRun /usr/local/bin/appimagetool && \
    rm /tmp/appimagetool.AppImage
