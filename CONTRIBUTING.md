# Contributing

## Documentation

- For a high-level overview of MapLibre GL Native, check out the [MapLibre GL Native Markdown Book](https://maplibre.org/maplibre-gl-native/docs/book/).
- See [`DEVELOPING.md`](DEVELOPING.md) for getting started with development.

## Source code checkout

```bash
git clone --recurse-submodules https://github.com/maplibre/maplibre-gl-native.git
```

## Building

MapLibre GL Native shares a single C++ core library with all platforms. To build it, we utilize CMake.

To build, run the following from the root directory
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DMBGL_WITH_CORE_ONLY=ON -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DMBGL_WITH_COVERAGE=ON
```

`CMAKE_BUILD_TYPE=Debug` will build debug artifacts. You can opt to omit it if that is not necessary.
`MBGL_WITH_CORE_ONLY=ON` will build only the core libraries.
Built artifacts should be available on `build` folder.


## Guidelines

If you want to contribute code:

1. Please familiarize yourself with the installation process of your chosen platform.

1. Ensure that existing [pull requests](https://github.com/maplibre/maplibre-gl-native/pulls) and [issues](https://github.com/maplibre/maplibre-gl-native/issues) don’t already cover your contribution or question.

1. Pull requests are gladly accepted. If there are any changes that developers using one of the GL SDKs should be aware of, please update the **main** section of the relevant `CHANGELOG.md`.

4. Prefix your commit messages with the platform(s) your changes affect, e.g. `[ios]`.

Please note the special instructions for contributing new source code files, asset files, or user-facing strings to MapLibre GL Native for [iOS](platform/ios/CONTRIBUTING.md), [Android](platform/android/DEVELOPING.md) or [macOS](platform/ios/platform/macos/DEVELOPING.md).

## Design Proposals

If you would like to change MapLibre GL Native in a substantial way, we recommend that you write a Design Proposal. Examples for substantial changes could be if you would like to split the mono-repo or if you would like to introduce shaders written in Metal.

The purpose of a Design Proposal is to collectively think through a problem before starting to implement a solution. Every implementation has advantages and disadvantages. We can discuss them in a Design Proposal, and once we reach an agreement, we follow the guidelines in the Design Proposal and work on the implementation.

The steps for a Design Proposal are the following:

1. Copy the Design Proposal template in the `design-proposals/` folder.
2. Use a filename with the current date and a keyword, e.g., `design-proposals/2022-09-15-metal.md`.
3. Fill out the template and submit a pull request.
4. Discuss the details of your Design Proposal with the community in the pull request. Adjust where needed.
5. Call a vote on the Design Proposal once discussions have settled. People in favor of your Design Proposal shall approve the pull request. People against your Design Proposal shall comment on the pull request with something like "Rejected".
6. Give the community at least 72 hours to vote. If a majority of the people who voted accept your Proposal, it can be merged.

## Semantic Versioning

MapLibre uses tags for releases based on [SemVer](https://semver.org) versioning.  This is useful for checking out a particular released version for feature enhancements or debugging.

You can list available tags by issuing the command `git tag`, then use the result

```bash
# 1. Obtain a list of tags, which matches to release versions
git tag

# 2.  Set a convenience variable with the desired TAG
# TAG=android-v9.4.2
# TAG=android-v9.5.2
TAG=ios-v5.12.0
# TAG=ios-v5.12.0-pre.1

# 3.  Check out a particular TAG
git checkout tags/$TAG -b $TAG

# 4. build, debug or enhance features based on the tag
# clean, if you need to troubleshoot build dependencies by using `make clean`
```
