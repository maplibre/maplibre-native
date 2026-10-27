# get-release-artifacts

Looks up the names under which a branch is released, so that several branches (for example `main` and `next`) can publish side by side without overwriting each other's artifacts, and without those names being hard-coded in the repository.

The mapping from branch to artifact names is a JSON object in the `RELEASE_ARTIFACTS` repository variable.

## Usage

Pass the variable to the action as an input, so that the lookup does not depend on whether the `vars` context is available inside a composite action:

```yaml
- uses: actions/checkout@v7
- id: artifacts
  uses: ./.github/actions/get-release-artifacts
  with:
    config: ${{ vars.RELEASE_ARTIFACTS }}

- run: echo "Publishing ${{ steps.artifacts.outputs.android-artifact-id }}"
```

The action needs a checkout of the repository, because it is a local action.

| Input    | Description                                                                                                                            |
| -------- | -------------------------------------------------------------------------------------------------------------------------------------- |
| `config` | The value of `RELEASE_ARTIFACTS`. If empty, [`default.json`](./default.json) is used.                                                  |
| `branch` | The branch to look up. Defaults to the target branch of a pull request, or else the branch of the workflow run (`github.ref_name`).    |

| Output                      | Description                                                              |
| --------------------------- | ------------------------------------------------------------------------ |
| `branch`                    | The branch that was looked up.                                           |
| `config`                    | The entry of the branch as compact JSON, for use with `fromJSON`.        |
| `android-artifact-id`       | Maven artifact ID of the Android SDK. Replaces `android-sdk`.            |
| `ios-swift-package`         | GitHub repository (`owner/name`) of the Swift package of the XCFramework. |
| `ios-plugins-swift-package` | GitHub repository of the Swift package of the XCFramework with the plugin API. |
| `ios-pod-name`              | Name of the CocoaPod.                                                    |

An output is an empty string if the entry of the branch does not set it.

> [!IMPORTANT]
> If a workflow runs on a tag instead of a branch, `github.ref_name` is the tag, which is not a key of the mapping and fails the lookup. Pass `branch` explicitly in that case. The Android release, for instance, is started on the tag `android-v<version>` by `android-ci.yml`.

## The `RELEASE_ARTIFACTS` variable

Keys are branch names, values are the artifact names of that branch. `android` and `ios` are both optional, and so are their fields, but every value that is present has to be a non-empty string.

```json
{
  "main": {
    "android": {
      "artifactId": "android-sdk"
    },
    "ios": {
      "swiftPackage": "maplibre/maplibre-gl-native-distribution",
      "pluginsSwiftPackage": "maplibre/maplibre-ios-with-plugin-api",
      "podName": "MapLibre"
    }
  },
  "next": {
    "android": {
      "artifactId": "android-sdk-next"
    },
    "ios": {
      "swiftPackage": "maplibre/maplibre-gl-native-distribution-next",
      "pluginsSwiftPackage": "maplibre/maplibre-ios-with-plugin-api-next",
      "podName": "MapLibre-next"
    }
  }
}
```

The names under `next` above are examples. The Swift package repositories have to exist before a release is made to them.

Set it under *Settings → Secrets and variables → Actions → Variables*, or with the GitHub CLI:

```sh
gh variable set RELEASE_ARTIFACTS --body "$(cat release-artifacts.json)"
```

### Defaults

- **The variable is not set (or empty):** [`default.json`](./default.json) is used. It maps `main` to the names that MapLibre Native itself releases under.
- **The branch is not in the mapping:** the action fails. Releasing a branch under the names of another branch would overwrite its artifacts, so there is deliberately no fallback to `main`.

## Forks

A fork that does not set the variable gets the defaults: its `main` branch is released under the same names as `maplibre/maplibre-native`. Those names are not yours to publish to, so a fork that makes releases should set `RELEASE_ARTIFACTS` to its own names:

1. Create a JSON file in the format above with an entry for every branch you release from. A fork that only releases from `main` needs just that one key, for instance:

   ```json
   {
     "main": {
       "android": { "artifactId": "my-android-sdk" },
       "ios": {
         "swiftPackage": "my-org/my-ios-distribution",
         "pluginsSwiftPackage": "my-org/my-ios-with-plugin-api",
         "podName": "MyMapLibre"
       }
     }
   }
   ```

2. Set it as the `RELEASE_ARTIFACTS` variable of your fork, as described above. Repository variables are not copied when forking, so this has to be done once per fork.
3. If your default branch is not called `main`, use its name as the key. A branch without an entry cannot be released.

You do not have to edit any file in the repository.
