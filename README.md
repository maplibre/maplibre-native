[![MapLibre Logo](https://maplibre.org/img/maplibre-logo-big.svg)](https://maplibre.org/)

# MapLibre Native

[![codecov](https://codecov.io/github/maplibre/maplibre-native/branch/main/graph/badge.svg?token=8ZQRRY56ZA)](https://codecov.io/github/maplibre/maplibre-native) [![](https://img.shields.io/badge/Slack-%23maplibre--native-2EB67D?logo=slack)](https://slack.openstreetmap.us/)

MapLibre Native is a free and open-source library for publishing maps in your apps and desktop applications on various platforms. Fast displaying of maps is possible thanks to GPU-accelerated vector tile rendering.

This project originated as a fork of Mapbox GL Native, before their switch to a non-OSS license in December 2020. For more information, see: [`FORK.md`](./FORK.md).

<p align="center">
  <img src="https://user-images.githubusercontent.com/649392/211550776-8779041a-7c12-4bed-a7bd-c2ec80af2b29.png" alt="Android device with MapLibre" width="24%">   <img src="https://user-images.githubusercontent.com/649392/211550762-0f42ebc9-05ab-4d89-bd59-c306453ea9af.png" alt="iOS device with MapLibre" width="25%">
</p>

## Getting Started

To get started with MapLibre Native, go to your platform below.

## Documentation

The documentation of MapLibre Native is a work in progress. To get an architectural overview and to learn about the current state of the project and its path forward read the [MapLibre Native Markdown Book](https://maplibre.org/maplibre-native/docs/book/). See below for platform-specific documentation.

## Platforms

- [⭐️ Android](platform/android/README.md)
- [⭐️ iOS](platform/ios/README.md)
- [GLFW](platform/glfw)
- [Linux](platform/linux/README.md)
- [Node.js](platform/node/README.md)
- [Qt](platform/qt/README.md)
- [Windows](platform/windows/README.md)
- [macOS](platform/macos/README.md)

Platforms with a ⭐️ are **MapLibre Core Projects** and have a substantial amount of financial resources allocated to them. Learn about the different [project tiers](https://github.com/maplibre/maplibre/blob/main/PROJECT_TIERS.md#project-tiers).

## Renderer Modularization & Metal

![image-metal](https://user-images.githubusercontent.com/53421382/214308933-66cd4efb-b5a5-4de3-b4b4-7ed59045a1c3.png)

MapLibre Native is being actively developed. Our big goal for 2023 is to modularize the OpenGL renderer and implement a Metal graphics backend (https://developer.apple.com/metal/). This will improve the performance and yield lower power consumption on iOS devices. At the same time, the Metal preparations will help us in the implementation of a Vulkan graphics backend.

Your help in preparing the codebase for the latest graphics backends is more than welcome. Feel free to reach out if you are interested in joining the effort!

- Check out the [news](https://maplibre.org/news/) on MapLibre's website.
- See the [Design Proposals](https://github.com/maplibre/maplibre-native/tree/main/design-proposals) that have been accepted and are being worked on, the most recent ones being the [Rendering Modularization Design Proposal](design-proposals/2022-10-27-rendering-modularization.md) and the [Metal Port Design Proposal](design-proposals/2022-11-29-metal-port.md).

## Contributing

To contribute to MapLibre Native, see [`CONTRIBUTING.md`](CONTRIBUTING.md) and (if applicable) the specific instructions for the platform you want to contribute to.

### Getting Involved

Join the `#maplibre-native` Slack channel at OSMUS. Get an invite at https://slack.openstreetmap.us/

### Bounties 💰

Thanks to our sponsors, we are able to award bounties to developers making contributions toward certain [bounty directions](https://github.com/maplibre/maplibre/issues?q=is%3Aissue+is%3Aopen+label%3A%22bounty+direction%22). To get started doing bounties, refer to the [step-by-step bounties guide](https://maplibre.org/roadmap/step-by-step-bounties-guide/).

We thank everyone who supported us financially in the past and special thanks to the people and organizations who support us with recurring donations!

Read more about the MapLibre Sponsorship Program at [https://maplibre.org/sponsors/](https://maplibre.org/sponsors/).

Gold:

<a href="https://aws.com"><img src="https://maplibre.org/img/aws-logo.svg" alt="Logo AWS" width="25%"/></a>

<a href="https://meta.com"><img src="https://maplibre.org/img/meta-logo.svg" alt="Logo Meta" width="25%"/></a>

Silver:

<a href="https://www.mierune.co.jp/?lang=en"><img src="https://maplibre.org/img/mierune-logo.svg" alt="Logo MIERUNE" width="25%"/></a>

<a href="https://komoot.com/"><img src="https://maplibre.org/img/komoot-logo.svg" alt="Logo komoot" width="25%"/></a>

<a href="https://www.jawg.io/"><img src="https://maplibre.org/img/jawgmaps-logo.svg" alt="Logo JawgMaps" width="25%"/></a>

<a href="https://www.radar.com/"><img src="https://maplibre.org/img/radar-logo.svg" alt="Logo Radar" width="25%"/></a>

<a href="https://www.microsoft.com/"><img src="https://maplibre.org/img/msft-logo.svg" alt="Logo Microsoft" width="25%"/></a>

Backers and Supporters:

[![](https://opencollective.com/maplibre/backers.svg?avatarHeight=50&width=600)](https://opencollective.com/maplibre)

## License

**MapLibre Native** is licensed under the [BSD 2-Clause License](./LICENSE.md).
