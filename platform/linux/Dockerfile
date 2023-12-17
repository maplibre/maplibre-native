FROM ubuntu:22.04

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y libcurl4-openssl-dev libglfw3-dev libuv1-dev libpng-dev libicu-dev libjpeg-turbo8-dev libwebp-dev xvfb clang git cmake ccache ninja-build pkg-config && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /root
