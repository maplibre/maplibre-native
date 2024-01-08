# Instructions for making an iOS release

We make iOS releases to GitHub (a downloadable XCFramework), the [Swift Package Index](https://swiftpackageindex.com/maplibre/maplibre-gl-native-distribution) and [CocoaPods](https://cocoapods.org/). Everyone with write access to the repository is able to make releases using the instructions below.

## Pre-release

Run the `ios-ci` workflow. You can use the [GitHub CLI](https://cli.github.com/manual/gh_workflow_run):

```
gh workflow run ios-ci.yml -f release=pre --ref main
```

Or run the workflow from the Actions tab on GitHub.

The items under the `## main` heading in `platform/ios/CHANGELOG.md` will be used as changelog for the pre-release. 

## Full release

1. Make sure the `VERSION` file in `platform/ios/VERSION` contains the version to be released. We use semantic versioning, so any breaking changes require a major version bump.

2. Update the changelog, which can be found in `platform/ios/CHANGELOG.md`. The heading must match `## <VERSION>` exactly, or it will not be picked up. For example, for version 6.0.0:

```md
## 6.0.0
```

3. Run the `ios-ci` workflow. You can use the [GitHub CLI](https://cli.github.com/manual/gh_workflow_run):

```
gh workflow run ios-ci.yml -f release=full --ref main
```

Or run the workflow from the Actions tab on GitHub:

<img width="367" alt="Screenshot 2024-01-08 at 11 00 30" src="https://github.com/maplibre/maplibre-native/assets/649392/ae791f04-f805-4544-b33a-44d8b04e0836">