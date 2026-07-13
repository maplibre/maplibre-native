# Developing the MapLibre Native Node.js module

This document explains how to build the [Node.js](https://nodejs.org/) bindings for [MapLibre Native](../../README.md) for contributing to the development of the bindings themselves. If you just want to use the module, you can simply install it via `npm`; see [README.md](README.md) for installation and usage instructions.

## Building

To develop these bindings, you’ll need to build them from source. Building requires the prerequisites listed in either
the [macOS](../macos/INSTALL.md#requirements), [Linux](../linux/README.md#prerequisites), or [Windows](../windows/README.md#prerequisites) install documentation, depending
on the target platform.

First you'll need to install dependencies:


#### MacOS

```bash
brew install \
  cmake \
  ccache \
  ninja \
  pkg-config \
  glfw3 \
  libuv
```

#### Linux (Ubuntu)

```bash
sudo apt-get install -y \
  build-essential \
  clang \
  cmake \
  ccache \
  ninja-build \
  pkg-config \
  libcurl4-openssl-dev \
  libglfw3-dev \
  libuv1-dev \
  libpng-dev \
  libicu-dev \
  libjpeg-turbo8-dev \
  libwebp-dev \
  xvfb
/usr/sbin/update-ccache-symlinks
```

### Compiling

To compile the Node.js bindings and install module dependencies, from the repository root directory, first run:

#### MacOS

```bash
cmake . -B build -G Ninja -DMLN_WITH_NODE=ON -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_BUILD_TYPE=Release -DMLN_WITH_OPENGL=OFF -DMLN_WITH_METAL=ON -DMLN_WITH_WERROR=OFF
```

#### Linux

```bash
cmake . -B build -G Ninja -DMLN_WITH_NODE=ON -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc-12 -DCMAKE_CXX_COMPILER=g++-12
```

### Building

Finally, build:
```bash
cmake --build build -j "$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null)"
```

## Testing

To test the Node.js bindings:

```bash
npm test
```

## Merging your pull request

To clean up your pull request and prepare it for merging, update your local `main` branch, then run `git rebase -i main` from your pull request branch to squash/fixup commits as needed. When your work is ready to be merged, you can run `git merge --ff-only YOUR_BRANCH` from `main` or click the green merge button in the GitHub UI, which will automatically squash your branch down into a single commit before merging it.

## Publishing
See [`RELEASE.md`](RELEASE.md) for instructions on publishing a node release.
