FROM ubuntu:22.04

# Install build tools and dependencies
RUN    apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        apt-transport-https \
        curl \
        gnupg \
        build-essential \
        libsqlite3-dev \
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
        pkg-config \
    && : # end of the RUN cmd - easier to keep a colon at the end of the list, than to keep the backslashes in check

# This could also be `.../releases/latest/download/bazelisk-linux-amd64` for the latest version, but for predictability better hardcode it
RUN curl -fsSL https://github.com/bazelbuild/bazelisk/releases/download/v1.20.0/bazelisk-linux-amd64 -o /usr/local/bin/bazel \
    && chmod +x /usr/local/bin/bazel \
    && :

# When running docker with -u flag, the user should exist in the container to avoid various warnings
RUN groupadd -f user && useradd -m -g user user

WORKDIR /app
