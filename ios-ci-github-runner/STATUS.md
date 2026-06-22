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

- Dispatch `ios-ci.yml` on `louwers/maplibre-native`.

## Attempts

- Signing bootstrap:
  - Command: `platform/ios/scripts/ios-ci-create-signing-assets.sh`
  - Result: failed before network/API calls because `APPSTORE_ISSUER_ID` is not available in the local environment.
  - Impact: `BUILD_CERTIFICATE_BASE64` and related signing secrets were not generated locally before the first workflow dispatch.

No workflow attempts have been dispatched from this branch yet.
