# Plan: Run ios-ci on GitHub-hosted macOS

## Goal

Move the `.github/workflows/ios-ci.yml` `ios-build` job from the preconfigured self-hosted Apple runner to a GitHub-hosted macOS runner, then iterate through GitHub Actions failures until the workflow passes on the `louwers/maplibre-native` fork.

## Branch and Tracking

- Work on branch `ios-ci-github-runner`.
- Push all branch updates to `louwers/maplibre-native`.
- Store workflow logs under `ios-ci-github-runner/logs/`.
- Track execution status in `ios-ci-github-runner/STATUS.md`.

## Implementation

- Reset `ios-ci-github-runner` from current `origin/main`.
- Change the `ios-build` job from `[self-hosted, macOS, ARM64]` to `macos-15`.
- Add `platform/ios/scripts/ios-ci-create-signing-assets.sh` to create Apple signing assets from App Store Connect credentials and push them as GitHub Actions secrets/variables to `louwers/maplibre-native`.
- Add `platform/ios/scripts/ios-ci-install-signing-assets.sh` to install the certificate/profile on the hosted runner and generate `platform/darwin/bazel/config.bzl`.
- Keep the existing Bazel, Xcode, artifact, size-test, and DocC steps unchanged unless logs prove a hosted-runner-specific fix is required.

## Execution Loop

1. Run `platform/ios/scripts/ios-ci-create-signing-assets.sh` locally with App Store Connect credentials available in the environment.
2. Push the workflow/script changes to `louwers/maplibre-native`.
3. Dispatch the workflow with `gh workflow run ios-ci.yml --repo louwers/maplibre-native --ref ios-ci-github-runner`.
4. Stop after dispatch.
5. On each `continue`, inspect the latest run with `gh run view`.
6. If the run failed, download logs into `ios-ci-github-runner/logs/<run-id>/`, diagnose the first actionable failure, make the smallest fix, update this status log, push, and dispatch again.
7. Repeat until `ios-ci.yml` finishes with conclusion `success`.

## Acceptance Criteria

- `ios-ci.yml` on `louwers/maplibre-native` branch `ios-ci-github-runner` completes successfully.
- The successful run uses a GitHub-hosted macOS runner for `ios-build`.
- Signing material is installed only from Actions secrets and runner temporary files.
- Logs for failed attempts are preserved under `ios-ci-github-runner/logs/`.
