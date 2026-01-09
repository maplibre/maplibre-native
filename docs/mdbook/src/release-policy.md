# Release Policy

> [!NOTE]
> The following release policiy applies specifically to MapLibre **Android** and MapLibre **iOS**.

- We use [semantic versioning](https://semver.org/). Breaking changes will always result in a major release.
- Despite having extensive tests in place, as a FOSS project we have limited QA testing capabilities. When major changes took place we may opt to put out a pre-release to let the community help with testing.
- In principle the `main` branch should always be in a releasable state.
- The release process is automated and documented (see [Release MapLibre iOS](./ios/release.md) and [Release MapLibre Android](./android/release.md)). Anyone with write access should be able to push out a release.
- There is no fixed release cadence, but you are welcome to request a release on any of the communication channels.
- We do not have long-term support (LTS) releases.
- If you need a feature or a bugfix ported to and old version of MapLibre, you need to do the backporting yourself (see steps below).

## Backporting

We understand that MapLibre is used in large mission critical applications where updating to the latest version is not always immediately possible. We do not have the capacity to offer LTS releases, but we do want to facilitate backporting.

1. Create an issue and request that a branch is created from the release you want to target. Also mention the feature or bugfix you want to backport.
2. Once the branch is created, make a PR that includes the feature or bugfix and that targets this branch. Also update the relevant changelog.
3. When the PR is approved and merged, a release is attempted. If the release workflow significantly changed and the release fails, you may need to help to backport changes to the release workflow as well.

The branch names for older versions follow a pattern as follows: `platform-x.x.x` (e.g. [`android-10.x.x`](https://github.com/maplibre/maplibre-native/tree/android-10.x.x) for the MapLibre Native Android 10.x.x release series). These branches have some minimal branch protection (a pull request is required to push changes to them).
