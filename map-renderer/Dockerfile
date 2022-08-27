FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y && apt-get install -y --no-install-recommends \
    clang ninja-build cmake pkg-config \
    xvfb libcurl4-openssl-dev libuv1-dev libpng-dev libjpeg-dev libgl1-mesa-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

COPY . /usr/src/maplibre-gl-native

WORKDIR /usr/src/maplibre-gl-native

RUN cmake -S . -B build -GNinja -DMBGL_WITH_MAP_RENDERER=ON \
    && cmake --build build --target map-renderer \
    && cmake --install build

CMD ["sleep", "infinity"]
