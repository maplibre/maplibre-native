# Status: ios-ci GitHub-hosted runner migration

## Current State

- Branch `ios-ci-github-runner` has been reset from `origin/main`.
- Branch `ios-ci-github-runner` has been force-pushed with lease to `louwers/maplibre-native`.
- Local unrelated untracked file `buddhism-discussion.html` is intentionally untouched.
- Added plan/status/log scaffolding under `ios-ci-github-runner/`.
- Added hosted-runner signing scripts under `platform/ios/scripts/`.
- Updated `.github/workflows/ios-ci.yml` so `ios-build` uses `macos-15` and installs Apple signing assets before Bazel/Xcode build steps.
- Validation run locally:
  - `bash -n platform/ios/scripts/ios-ci-create-signing-assets.sh`: passed.
  - `bash -n platform/ios/scripts/ios-ci-install-signing-assets.sh`: passed.
  - `git diff --check`: passed.
  - YAML parse with Ruby: passed.
  - `actionlint`: not available in this workspace.

## Next Action

- Run the signing bootstrap script if App Store Connect credentials are available locally.
- Push changes and dispatch `ios-ci.yml` on `louwers/maplibre-native`.

## Attempts

No workflow attempts have been dispatched from this branch yet.
