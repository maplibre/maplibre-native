# Release MapLibre iOS

We make iOS releases to GitHub (a downloadable XCFramework), the [Swift Package Index](https://swiftpackageindex.com/maplibre/maplibre-gl-native-distribution) and [CocoaPods](https://cocoapods.org/). Everyone with write access to the repository is able to make releases using the instructions below.

Also see the current [release policy](../release-policy.md).

## Making a release

1. Prepare a PR, see [this PR](https://github.com/maplibre/maplibre-native/pull/3193) as an example.

    - Update the [changelog](https://github.com/maplibre/maplibre-native/blob/main/platform/ios/CHANGELOG.md). The changelog should contain links to all relevant PRs for iOS since the last release. You can use the script below with a [GitHub access token](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/managing-your-personal-access-tokens) with the `public_repo` scope. You will need to filter out PRs that do not relate to iOS.
      ```
      GITHUB_ACCESS_TOKEN=... node scripts/generate-changelog.mjs ios
      ```
      The heading in the changelog must match `## <VERSION>` exactly, or it will not be picked up. For example, for version 6.0.0:
      ```md
      ## 6.0.0
      ```
    - Update the `VERSION` file in `platform/ios/VERSION` with the version to be released. We use [semantic versioning](https://semver.org/), so any breaking changes require a major version bump. Use a minor version bump when functionality has been added in a backward compatible manner, and a patch version bump when the release contains only backward compatible bug fixes.

2. Once the PR is merged the `ios-ci.yml` workflow will detect that the `VERSION` file is changed, and a release will be made automatically.

## Pre-release

Run the `ios-ci` workflow. You can use the [GitHub CLI](https://cli.github.com/manual/gh_workflow_run):

```
gh workflow run ios-ci.yml -f release=pre --ref main
```

Or run the workflow from the Actions tab on GitHub.

The items under the `## main` heading in `platform/ios/CHANGELOG.md` will be used as changelog for the pre-release.
