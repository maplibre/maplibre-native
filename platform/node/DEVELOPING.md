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
cmake . -B build -G Ninja -DMLN_WITH_NODE=ON -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_BUILD_TYPE=Release -DMLN_WITH_OPENGL=OFF -DMLN_WITH_METAL=ON -DMLN_LEGACY_RENDERER=OFF -DMLN_DRAWABLE_RENDERER=ON -DMLN_WITH_WERROR=OFF
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

To publish a new version of the package:
- [ ] make a commit in 'platform/node/' which includes:
    - [ ] an updated version number in [`platform/node/package.json`](package.json#L3)
    - [ ] an entry in [`platform/node/CHANGELOG.md`](CHANGELOG.md) describing the changes in the release
- [ ] create a PR to merge your changes into main. A release will be published and a tag will be created automatically once merged.

The node-release CI publishes when the version listed in `package.json` has not yet been published to npm. If a new version is found it will run with `BUILDTYPE=Release` and publish a binary with `node-pre-gyp`.

Once binaries have been published (which can be verified with `./node_modules/.bin/node-pre-gyp info`), you can run a quick final check to ensure they're being fetched properly by simply running `rm -rf lib && npm install`.

### Preleases

Publishing a prerelease binary can be useful for testing downstream integrations - the workflow is the same except that you need to make sure the release version ends in with the preid 'pre', so that the workflow will automatically publish as a prerelease. 

For example, running the following command , replacing <update_type> with one of the semantic versioning release types (prerelease, prepatch, preminor, premajor):
```bash
npm version <update_type> --preid pre --no-git-tag-version
```
