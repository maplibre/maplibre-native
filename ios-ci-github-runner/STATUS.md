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
  - Second attempt with provided App Store Connect issuer/key/private key reached App Store Connect but failed with HTTP `403` while creating an iOS development certificate.
  - Confirmed Apple response: `FORBIDDEN_ERROR.PLA_NOT_ACCEPTED`; Apple says the team's Account Holder, Bart Louwers, must agree to the latest Program License Agreement.
  - Impact: `BUILD_CERTIFICATE_BASE64` and `P12_PASSWORD` are still missing from `louwers/maplibre-native`; `BUILD_PROVISION_PROFILE_BASE64` and `KEYCHAIN_PASSWORD` already exist.
  - After the Apple Program License Agreement was accepted, the script reached certificate/profile creation.
  - Fixed script portability and cleanup issues:
    - macOS `base64` encoding required `base64 -i`.
    - The interrupted first successful Apple request left an API-created iOS Development certificate without a matching local private key.
    - Duplicate CI provisioning profiles had to be deleted before creating a fresh profile.
  - Successful bootstrap:
    - Deleted orphaned API-created iOS Development certificate `Z5U4Z7576C`.
    - Deleted duplicate iOS CI provisioning profile `922V8AGXB7`.
    - Created a new `MapLibre iOS CI` signing certificate and `MapLibre iOS CI Wildcard Development` profile.
    - Updated `BUILD_CERTIFICATE_BASE64`, `P12_PASSWORD`, `BUILD_PROVISION_PROFILE_BASE64`, `KEYCHAIN_PASSWORD`, and `IOS_BUNDLE_ID_PREFIX` on `louwers/maplibre-native`.

- Workflow attempt 1:
  - Run ID: `27955381487`
  - URL: https://github.com/louwers/maplibre-native/actions/runs/27955381487
  - Branch: `ios-ci-github-runner`
  - Head SHA: `9024ab87ed89b75ca6480dfbd6fed101a203918b`
  - Created: `2026-06-22T13:14:15Z`
  - `ios-build` job ID: `82723010744`
  - `ios-build` result: failed in `Install Apple signing assets` because `BUILD_CERTIFICATE_BASE64` was empty.
  - Logs: `ios-ci-github-runner/logs/27955381487/ios-build.log`
  - Overall workflow status at last check: still `in_progress` because other jobs were still running.
  - Next step: superseded by workflow attempt 2 after signing secrets were created.

- Workflow attempt 2:
  - Run ID: `27961865104`
  - URL: https://github.com/louwers/maplibre-native/actions/runs/27961865104
  - Branch: `ios-ci-github-runner`
  - Head SHA: `44e59488472ccf59b1477d512126b42336a4ef5c`
  - Created: `2026-06-22T14:54:37Z`
  - Overall result: failed.
  - `ios-build` job ID: `82745657503`
  - `ios-build` progress: signing, Swift app build, symbol check, plist lint, and iOS tests passed.
  - Failure: `Build RenderTest .ipa and .xctest for AWS Device Farm` failed after `xcodebuild` succeeded because `mv RenderTestApp.app Payload/RenderTestApp.app` returned `Permission denied`.
  - Logs: `ios-ci-github-runner/logs/27961865104/ios-build.log`
  - Fix:
    - Moved `Build RenderTest .ipa and .xctest for AWS Device Farm` immediately after Bazel cache restore so the next run reaches it sooner.
    - Changed RenderTest and CppUnitTests packaging to copy app bundles into `Payload` with `ditto` instead of moving them from Xcode/Bazel build output directories.
  - Next step: push the workflow fix and dispatch attempt 3.

- Workflow attempt 3:
  - Run ID: `27967320700`
  - URL: https://github.com/louwers/maplibre-native/actions/runs/27967320700
  - Branch: `ios-ci-github-runner`
  - Head SHA: `5cb892d0a980647f72745934b25a33f688a994cb`
  - Created: `2026-06-22T16:18:55Z`
  - Status at dispatch check: `queued`
  - Next step: stop and wait for `continue`; then inspect completion status with `gh`.
