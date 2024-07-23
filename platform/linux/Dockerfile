FROM ubuntu:22.04

# When running docker with -u flag, the user should exist in the container to avoid various warnings
RUN groupadd -f user && useradd -m -g user user

# Install build tools and dependencies
RUN    apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        apt-transport-https \
        curl \
        gnupg \
    && curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor >bazel-archive-keyring.gpg \
    && mv bazel-archive-keyring.gpg /usr/share/keyrings \
    && echo "deb [arch=amd64 signed-by=/usr/share/keyrings/bazel-archive-keyring.gpg] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        bazel \
        libcurl4-openssl-dev \
        libglfw3-dev \
        libuv1-dev \
        libpng-dev \
        libicu-dev \
        libjpeg-turbo8-dev \
        libwebp-dev \
        xvfb \
        clang \
        git \
        cmake \
        ccache \
        ninja-build \
        pkg-config

WORKDIR /code
