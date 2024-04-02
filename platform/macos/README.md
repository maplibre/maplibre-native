# MapLibre Native for macOS

[![GitHub Action build status](https://github.com/maplibre/maplibre-native/workflows/macos-ci/badge.svg)](https://github.com/maplibre/maplibre-native/actions/workflows/macos-ci.yml)

Clone the repo:

```sh
git clone --recurse-submodules git@github.com:maplibre/maplibre-native.git
```

Install needed tooling and dependencies with Homebrew. See [macos-ci.yml](https://github.com/maplibre/maplibre-native/blob/main/.github/workflows/macos-ci.yml) for the most up-to-date dependencies.

Optionally configure Bazel:

```sh
cp platform/darwin/bazel/example_config.bzl platform/darwin/bazel/config.bzl
```

Create and open Xcode project:

```sh
bazel run //platform/macos:xcodeproj --@rules_xcodeproj//xcodeproj:extra_common_flags="--//:renderer=metal
xed platform/macos/MapLibre.xcodeproj
```

Build and run AppKit sample app directly from the command line:

```sh
bazel run //platform/macos/app:macos_app --//:renderer=metal
```

---

The MapLibre Organization does not officially support the macOS to the same extent as iOS (see [project tiers](https://github.com/maplibre/maplibre/blob/main/PROJECT_TIERS.md)). However, bug reports and pull requests are certainly welcome.
