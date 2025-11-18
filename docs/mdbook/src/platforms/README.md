
# Platforms

This page describes the platforms that MapLibre Native is available on.

## Overview

MapLibre Native uses a monorepo. Source code for all platforms lives in [`maplibre/maplibre-native`](https://github.com/maplibre/maplibre-native) on GitHub.

| Platform | Source | Notes |
|---|---|---|
| Android | [`platform/android`](https://github.com/maplibre/maplibre-native/tree/main/platform/android) | Integrates with the C++ core via JNI. |
| iOS | [`platform/ios`](https://github.com/maplibre/maplibre-native/tree/main/platform/ios), [`platform/darwin`](https://github.com/maplibre/maplibre-native/tree/main/platform/darwin) | Integrates with the C++ core via Objective-C++.  |
| Linux | [`platform/linux`](https://github.com/maplibre/maplibre-native/tree/main/platform/linux) | Used for development. Also widely used in production for raster tile generation. |
| Windows | [`platform/windows`](https://github.com/maplibre/maplibre-native/tree/main/platform/windows) | |
| macOS | [`platform/macos`](https://github.com/maplibre/maplibre-native/tree/main/platform/macos), [`platform/darwin`](https://github.com/maplibre/maplibre-native/tree/main/platform/darwin)  | Mainly used for development. There is some legacy AppKit code. |
| Node.js | [`platform/node`](https://github.com/maplibre/maplibre-native/tree/main/platform/node) | Uses [NAN](https://github.com/nodejs/nan). Available as [@maplibre/maplibre-gl-native](https://www.npmjs.com/package/@maplibre/maplibre-gl-native) on npm. |
| Qt | [maplibre/maplibre-qt](https://github.com/maplibre/maplibre-native/tree/main/platform/qt), [`platform/qt`](https://github.com/maplibre/maplibre-native)  | Only platform that partially split to another repository. |

Of these, **Android** and **iOS** are considered [core projects](https://github.com/maplibre/maplibre/blob/main/PROJECT_TIERS.md) of the MapLibre Organization.
### GLFW

You can find an app that uses GLFW in [`platform/glfw`](https://github.com/maplibre/maplibre-native/tree/main/platform/glfw). It works on macOS, Linux and Windows. The app shows an interactive map that can be interacted with. Since GLFW adds relatively little complexity this app is used a lot for development. You can also learn about the C++ API by studying the source code of the GLFW app.

## Rendering Backends

Originally the project only supported OpenGL 2.0. In 2023, the [renderer was modularized](https://github.com/maplibre/maplibre-native/blob/main/design-proposals/2022-10-27-rendering-modularization.md) allowing for the implementation of alternate rendering backends. The first alternate rendering backend that was implemented was [Metal](https://maplibre.org/news/2024-01-19-metal-support-for-maplibre-native-ios-is-here/), followed by [Vulkan](https://maplibre.org/news/2024-12-12-maplibre-android-vulkan/). In the future other rendering backends could be implemented such as WebGPU.

What platforms support which rendering backend can be found below.


| Platform | OpenGL ES 3.0  | Vulkan 1.0 | Metal  | WebGPU  |
| -------- | -------------- | ---------- | ------ | ------- |
| Android  | ✅             | ✅          | ❌     | ❌      |
| iOS      | ❌             | ❌          | ✅     | ❌      |
| Linux    | ✅             | ✅          | ❌     | ✅      |
| Windows  | ✅             | ❌          | ❌     | ❓      |
| macOS    | ❌             | ✅          | ✅[^1] | ✅      |
| Node.js  | ✅             | ❌          | ✅[^2] | ❌      |
| Qt       | ✅             | ❌          | ❌     | ❌      |

[^1]: Requires MoltenVK. Only available when built via CMake.
[^2]: Issue reported, see [#2928](https://github.com/maplibre/maplibre-native/issues/2928).

## Build Tooling

In 2023 we co-opted Bazel as a build tool (generator), mostly due to it having better support for iOS compared to CMake. Some platforms can use CMake as well as Bazel.

| Platform | CMake | Bazel |
|---|---|---|
| Android | ✅ (via Gradle) | ❌ |
| iOS | ✅[^3] | ✅ |
| Linux | ✅ | ✅ |
| Windows | ✅ | ✅ |
| macOS | ✅ | ✅ |
| Node.js | ✅ | ❌ |
| Qt | ✅ | ❌ |


[^3]: Some targets are supported, see [here](ios/README.md#cmake).
