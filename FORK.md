# Fork

MapLibre Native is a community led fork derived from [mapbox-gl-native](https://github.com/mapbox/mapbox-gl-native) prior to their switch to a non-OSS license. The fork also includes Maps SDK for iOS and MacOS (forked from [mapbox-gl-native-ios](https://github.com/mapbox/mapbox-gl-native-ios)) and Android SDK (forked from [mapbox-gl-native-android](https://github.com/mapbox/mapbox-gl-native-android)). These platform-specific SDKs were merged under platform directory and they reference mapbox-gl-native directly, not as a submodule.

Beside merging in platform specific SDKs, the following changes were made compared to original mapbox projects:

* The code was upgraded so that it can be built using latest clang compiler / Xcode 12.
* CI/CD was migrated from CircleCI to GitHub Actions.
* Along with GitHub releases, binaries are distributed as follows:
    * The iOS binaries distribution was upgraded from fat packages to Swift package containing XCFramework.
    * The Android binaries are distributed to GitHub maven package repository.

> The mapbox-gl-native was forked from [d60fd30 - mgbl 1.6.0](https://github.com/mapbox/mapbox-gl-native/tree/d60fd302b1f6563e7d16952f8855122fdcc85f73), mapbox-gl-native-ios from [a139216](https://github.com/mapbox/mapbox-gl-native-ios/commit/a139216) and mapbox-gl-native-android from [4c12fb2](https://github.com/mapbox/mapbox-gl-native-android/commit/4c12fb2c)

## Thank you Mapbox 🙏🏽

We'd like to acknowledge the amazing work Mapbox has contributed to open source. The open source community is sad to part ways with them, but we simultaneously feel grateful for everything they already contributed. We're proud to develop on the shoulders of giants, thank you Mapbox 🙇🏽‍♀️.

Please keep in mind: Unauthorized backports are the biggest threat to the MapLibre project. It is unacceptable to backport code from any Mapbox project with a non-free license. If you are unsure about this issue, [please ask](https://github.com/maplibre/maplibre-native/discussions)!
