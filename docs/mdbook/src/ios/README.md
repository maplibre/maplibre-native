# MapLibre iOS Developer Guide

## Bazel

[Bazel](https://bazel.build/) is used for building on iOS.

You can generate an Xcode project thanks to [rules_xcodeproj](https://github.com/MobileNativeFoundation/rules_xcodeproj) intergration.

You need to install [bazelisk](https://github.com/bazelbuild/bazelisk), which is a wrapper around Bazel which ensures that the version specified in `.bazelversion` is used.

```
brew install bazelisk
```

### Creating `config.bzl`

Configure Bazel, otherwise the default config will get used.

```
cp platform/darwin/bazel/example_config.bzl platform/darwin/bazel/config.bzl
```

You need to set your `BUNDLE_ID_PREFIX` to be unique (ideally use a domain that you own in reverse domain name notation).

You can keep leave the `APPLE_MOBILE_PROVISIONING_PROFILE_NAME` alone.

Set the Team ID to the Team ID of your Apple Developer Account (paid or unpaid both work). If you do not know your Team ID, go to your [Apple Developer account](https://developer.apple.com/account), log in, and scroll down to find your Team ID.

If you don't already have a developer account, continue this guide and let Xcode generate a provisioning profile for you. You will need to update the Team ID later once a certificate is generated.

## Create the Xcode Project

Run the following commands:

```
bazel run //platform/ios:xcodeproj --@rules_xcodeproj//xcodeproj:extra_common_flags="--//:renderer=metal"
xed platform/ios/MapLibre.xcodeproj
```

Then once in Xcode, click on "MapLibre" on the left, then "App" under Targets, then "Signing & Capabilities" in the tabbed menu.
Confirm that no errors are shown.

<img width="921" alt="image" src="https://github.com/polvi/maplibre-native/assets/649392/a1ef30cb-97fc-429a-acee-194436f3fb8a">

Try to run the example App in the simulator and on a device to confirm your setup works.

> [!IMPORTANT]
> The Bazel configuration files are the source of truth of the build configuration. All changes to the build settings need to be done through Bazel, not in Xcode.

### Troubleshooting

#### Provisioning Profiles

If you get a Python `KeyError` when processing provisioning profiles, you probably have some _really_ old or corrupted profiles.

Have a look through `~/Library/MobileDevice/Provisioning\ Profiles` and remove any expired profiles. Removing all profiles here can also resolve some issues.

#### Cleaning Bazel environments

You should almost never have to do this, but sometimes problems can be solved with:

```
bazel clean --expunge
```

## Using Bazel from the Command Line

It is also possible to build and run the test application in a simulator from the command line without opening Xcode.

```
bazel run //platform/ios:App --//:renderer=metal
```

You can also build targets from the command line. For example, if you want to build your own XCFramework, see the 'Build XCFramework' step in the [iOS CI workflow](../../.github/workflows/ios-ci.yml).

## CMake

It is also possible to generate an Xcode project using CMake. As of February 2025, targets `mbgl-core`, `ios-sdk-static` and `app` (Objective-C development app) are supported.

```
cmake --preset ios -DDEVELOPMENT_TEAM_ID=YOUR_TEAM_ID
xed build-ios/MapLibre\ Native.xcodeproj
```

## Distribution

MapLibre iOS is distributed as an XCFramework via the [maplibre/maplibre-gl-native-distribution](https://github.com/maplibre/maplibre-gl-native-distribution) repository. See [Release MapLibre iOS](./release.md) for the release process. Refer to the [`ios-ci.yml`](https://github.com/maplibre/maplibre-native/blob/main/.github/workflows/ios-ci.yml) workflow for an up-to-date recipe for building an XCFramework. As of February 2025 we use:

```
bazel build --compilation_mode=opt --features=dead_strip,thin_lto --objc_enable_binary_stripping \
  --apple_generate_dsym --output_groups=+dsyms --//:renderer=metal //platform/ios:MapLibre.dynamic --embed_label=maplibre_ios_"$(cat VERSION)"
```
