# Release MapLibre Android

We make MapLibre Android releases as a downloadable asset on [GitHub](https://github.com/maplibre/maplibre-native/releases?q=android&expanded=true) as well as to [Maven Central](https://central.sonatype.com/artifact/org.maplibre.gl/android-sdk/versions).

Also see the current [release policy](../../release-policy.md).

## Making a release

To make an Android release, do the following:

1. Prepare a PR.

    - Update [`CHANGELOG.md`](https://github.com/maplibre/maplibre-native/blob/main/platform/android/CHANGELOG.md) in a PR, see for example [this PR](https://github.com/maplibre/maplibre-native/pull/3194). The changelog should contain links to all relevant PRs for Android since the last release. You can use the script below with a [GitHub access token](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/managing-your-personal-access-tokens) with the `public_repo` scope. You will need to filter out PRs that do not relate to Android and categorize PRs as features or bugfixes.
        ```
        GITHUB_ACCESS_TOKEN=... node scripts/generate-changelog.mjs android
        ```
        The heading in the changelog must match `## <VERSION>` exactly, or it will not be picked up. For example, for version 9.6.0:
        ```md
        ## 9.6.0
        ```

    - Update `android/VERSION` with the new version.

2. Once the PR is merged, the [`android-release.yml`](https://github.com/maplibre/maplibre-native/blob/main/.github/workflows/android-release.yml) workflow will run automatically to make the release. It builds every published [build flavor](./android-build-flavors.md) as a release and a debug build, and signs the publications of each with the `publish<Variant>Publications` Gradle task (e.g. `publishOpenglWithPluginApiReleasePublications`). Each build uploads its publications to Maven Central as a deployment, which Maven Central validates but does not publish yet. Only when all builds have succeeded does the workflow publish all deployments. Deployments of a release that is abandoned after a failure can be dropped in the [Central Portal](https://central.sonatype.com/publishing/deployments).
