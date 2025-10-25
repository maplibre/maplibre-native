# macOS

MapLibre Native can be built for macOS. This is mostly used for development.

> [!NOTE]
> There are some [AppKit](https://developer.apple.com/documentation/appkit) APIs for macOS the source tree. However those are not actively maintained. There is an [discussion](https://github.com/maplibre/maplibre-native/discussions/3414) on whether we should remove this code.

## File Structure

| Path                        | Description                                                              |
|-----------------------------|--------------------------------------------------------------------------|
| `platform/darwin`          | Shared code between macOS and iOS                                        |
| `platform/darwin/core`     | iOS/macOS specific implementations for interfaces part of the MapLibre Native C++ Core |
| `platform/macos`           | macOS specific code                                                      |
| `platform/macos/app`       | AppKit based example app                                                 |

## Getting Started

Clone the repo:

```sh
git clone --recurse-submodules git@github.com:maplibre/maplibre-native.git
```

Make sure the following Homebrew packages are installed:

```sh
brew install bazelisk webp libuv webp icu4c jpeg-turbo glfw libuv
brew link icu4c --force
```

You can get started building the project for macOS using either Bazel or CMake.

## Bazel

Configure Bazel (optional):

```sh
cp platform/darwin/bazel/example_config.bzl platform/darwin/bazel/config.bzl
```

Run the GLFW app with a style of your choice:

```sh
bazel run --//:renderer=metal //platform/glfw:glfw_app -- --style https://sgx.geodatenzentrum.de/gdz_basemapworld_vektor/styles/bm_web_wld_col.json
```

Create and open Xcode project:

```sh
bazel run //platform/macos:xcodeproj --@rules_xcodeproj//xcodeproj:extra_common_flags="--//:renderer=metal"
xed platform/macos/MapLibre.xcodeproj
```

## CMake

Configure CMake:

```sh
cmake --preset macos-metal
```

Build and run the render tests:

```sh
cmake --build build-macos --target mbgl-render-test-runner
build-macos/mbgl-render-test-runner --manifestPath=metrics/macos-xcode11-release-style.json
```

Build and run the C++ Tests:

```sh
cmake --build build-macos-metal --target mbgl-test-runner
npm install && node test/storage/server.js  # required test server
# in another terminal
build-macos-metal/mbgl-test-runner
```

Create and open an Xcode project with CMake:

```sh
cmake --preset macos-metal-xcode
xed build-macos-metal-xcode/MapLibre\ Native.xcodeproj
```

Configure project for Vulkan (make sure [MoltenVK](https://github.com/KhronosGroup/MoltenVK) is installed):

```sh
cmake --preset macos-vulkan
```

Build and run `mbgl-render` (simple command line utility for rendering maps):

```sh
cmake --build build-macos-vulkan --target mbgl-render
build-macos-vulkan/bin/mbgl-render -z 7 -x -74 -y 41 --style https://americanamap.org/style.json
open out.png
```
