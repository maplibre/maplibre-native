# Contributing

If you have a usage question for a product built on MapLibre GL (such as our iOS or Android SDKs), please visit #maplibre Slack channel at [osmus.slack.com](https://osmus.slack.com/). Feel free to [join here](https://osmus-slack.herokuapp.com/).

If you want to contribute code:

1. Please familiarize yourself with the [install process](README.md#installation).

1. Ensure that existing [pull requests](https://github.com/maplibre/maplibre-gl-native/pulls) and [issues](https://github.com/maplibre/maplibre-gl-native/issues) don’t already cover your contribution or question.

1. Pull requests are gladly accepted. If there are any changes that developers using one of the GL SDKs should be aware of, please update the **master** section of the relevant changelog(s):
  * [MapLibre Maps SDK for iOS](platform/ios/CHANGELOG.md)
  * [MapLibre Maps SDK for macOS](platform/macos/CHANGELOG.md)

1. Prefix your commit messages with the platform(s) your changes affect: `[ios]` or `[macos]`.

Please note the special instructions for contributing new source code files, asset files, or user-facing strings to the [iOS SDK](platform/ios/DEVELOPING.md) or [macOS SDK](platform/macos/DEVELOPING.md).

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
