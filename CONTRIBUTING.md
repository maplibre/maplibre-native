# Contributing

## Documentation

The [MapLibre Native Developer Docs](https://maplibre.org/maplibre-native/docs/book/) is the go-to reference for developing MapLibre Native.

## Source code checkout

```bash
git clone --recurse-submodules https://github.com/maplibre/maplibre-native.git
```

## Guidelines

If you want to contribute code:

1. Please familiarize yourself with the installation process of your chosen platform.

1. Ensure that existing [pull requests](https://github.com/maplibre/maplibre-native/pulls) and [issues](https://github.com/maplibre/maplibre-native/issues) don’t already cover your contribution or question.

1. Pull requests are gladly accepted. If there are any changes that developers using one of the platforms should be aware of, please update the **main** section of the relevant `CHANGELOG.md`.

## Design Proposals

If you would like to change MapLibre Native in a substantial way, we recommend that you write a Design Proposal. Examples for substantial changes could be if you would like to split the mono-repo or if you would like to introduce shaders written in Metal.

The purpose of a Design Proposal is to collectively think through a problem before starting to implement a solution. Every implementation has advantages and disadvantages. We can discuss them in a Design Proposal, and once we reach an agreement, we follow the guidelines in the Design Proposal and work on the implementation.

The steps for a Design Proposal are the following:

1. Copy the Design Proposal template in the `design-proposals/` folder.
2. Use a filename with the current date and a keyword, e.g., `design-proposals/2022-09-15-metal.md`.
3. Fill out the template and submit a pull request.
4. Discuss the details of your Design Proposal with the community in the pull request. Adjust where needed.
5. Call a vote on the Design Proposal once discussions have settled. People in favor of your Design Proposal shall approve the pull request. People against your Design Proposal shall comment on the pull request with something like "Rejected".
6. Give the community at least 72 hours to vote. If a majority of the people who voted accept your Proposal, it can be merged.

[Accepted design proposals](https://github.com/maplibre/maplibre-native/tree/main/design-proposals).

## Pre-commit hooks

Install [pre-commit](https://pre-commit.com/) and run

```
pre-commit install
```

to install the pre-commit hooks configured in `.pre-commit-config.yml`.
