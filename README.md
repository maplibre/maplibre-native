[![MapLibre Logo](https://maplibre.org/img/maplibre-logo-big.svg)](https://maplibre.org/)

# MapLibre GL Native

MapLibre GL Native is a free and open-source library for publishing maps in your apps and desktop applications on various platforms. Fast displaying of maps is possible thanks to GPU-accelerated vector tile rendering.

This project originated as a fork of Mapbox GL Native, before their switch to a non-OSS license in December 2020. For more information, see: [`FORK.md`](./FORK.md).

**Call for Bounties💰** If you have ideas for new features in MapLibre, you can now nominate them for the MapLibre Bounty Program at https://maplibre.org/news/2022-10-16-call-for-bounties/

<p align="center">
  <img src="https://user-images.githubusercontent.com/649392/211550776-8779041a-7c12-4bed-a7bd-c2ec80af2b29.png" alt="Android device with MapLibre" width="24%">   <img src="https://user-images.githubusercontent.com/649392/211550762-0f42ebc9-05ab-4d89-bd59-c306453ea9af.png" alt="iOS device with MapLibre" width="25%">
</p>

## Getting Started

To get started with MapLibre GL Native, go to your platform below.

## Documentation

The documentation of MapLibre GL Native is a work in progress. To get an architectural overview and to learn about the current state of the project and its path forward read the [MapLibre GL Native Markdown Book](https://maplibre.org/maplibre-gl-native/docs/book/). See below for platform-specific documentation.

## Platforms

- [⭐️ Android](platform/android/README.md) 
- [⭐️ iOS](platform/ios/platform/ios/README.md)
- [GLFW](platform/glfw)
- [Linux](platform/linux/README.md)
- [macOS](platform/ios/platform/macos/README.md)
- [Node.js](platform/node/README.md)
- [Qt](platform/qt/README.md)

Platforms with a ⭐️ are **MapLibre Core Projects** and have a substantial amount of financial resources allocated to them. Learn about the different [project tiers](https://github.com/maplibre/maplibre/blob/main/PROJECT_TIERS.md#project-tiers).

## Contributing

To contribute to MapLibre GL Native, see [`CONTRIBUTING.md`](CONTRIBUTING.md) and (if applicable) the specific instructions for the platform you want to contribute to.

### Getting Involved

Join the `#maplibre-native` Slack channel at OSMUS. Get an invite at https://slack.openstreetmap.us/ 

## Build Status

| SDK                                                           | Build   | Build status                                                                                                                                                                                  |
|---------------------------------------------------------------|---------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [MapLibre GL Native for iOS](platform/ios/) | CI      | [![GitHub Action build status](https://github.com/maplibre/maplibre-gl-native/workflows/ios-ci/badge.svg)](https://github.com/maplibre/maplibre-gl-native/actions/workflows/ios-ci.yml)                   |
| [MapLibre GL Native for iOS](platform/ios/) | Release | [![GitHub Action build status](https://github.com/maplibre/maplibre-gl-native/workflows/ios-release/badge.svg)](https://github.com/maplibre/maplibre-gl-native/actions/workflows/ios-release.yml)         |
| [MapLibre GL Native for Android](platform/android/)      | CI      | [![GitHub Action build status](https://github.com/maplibre/maplibre-gl-native/workflows/android-ci/badge.svg)](https://github.com/maplibre/maplibre-gl-native/actions/workflows/android-ci.yml)           |
| [MapLibre GL Native for Android](platform/android/)     | Release | [![GitHub Action build status](https://github.com/maplibre/maplibre-gl-native/workflows/android-release/badge.svg)](https://github.com/maplibre/maplibre-gl-native/actions/workflows/android-release.yml) |

## The Future of MapLibre GL Native

MapLibre GL Native is being actively developed.

- Check out the [news](https://maplibre.org/news/) on MapLibre's website.
- See the [Design Proposals](https://github.com/louwers/maplibre-gl-native/tree/main/design-proposals) that have been accepted and are being worked on, the most recent being the [Rendering Modularization Design Proposal](https://github.com/louwers/maplibre-gl-native/blob/main/design-proposals/2022-10-27-rendering-modularization.md).

## Sponsors

We thank everyone who supported us financially in the past and special thanks to the people and organizations who support us with recurring donations!

Read more about the MapLibre Sponsorship Program at [https://maplibre.org/sponsors/](https://maplibre.org/sponsors/).

Platinum:

<img src="https://maplibre.org/img/aws-logo.svg" alt="Logo AWS" width="25%"/>


Silver:

<img src="https://maplibre.org/img/meta-logo.svg" alt="Logo Meta" width="50%"/>

Stone:

[MIERUNE Inc.](https://www.mierune.co.jp/?lang=en)

Backers and Supporters:

[![](https://opencollective.com/maplibre/backers.svg?avatarHeight=50&width=600)](https://opencollective.com/maplibre)

## License

**MapLibre GL Native** is licensed under the [BSD 2-Clause License](./LICENSE.md).
