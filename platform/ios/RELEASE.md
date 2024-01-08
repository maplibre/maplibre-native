# Instructions for making an iOS release

1. Make sure the `VERSION` file in `platform/ios/VERSION` contains the version to be released. We use semantic versioning, so any breaking changes require a major version bump.

2. Update the changelog, which can be found in `platform/ios/CHANGELOG.md`. The heading must match `## <VERSION>` exactly, or it will not be picked up. For example, for version 6.0.0:

```
## 6.0.0
```

3. Run the `ios-ci` workflow. You can use the [GitHub CLI](https://cli.github.com/manual/gh_workflow_run):

```
gh workflow run ios-ci.yml -f release=true --ref main
```

Or run the workflow from the Actions tab on GitHub:

<img width="333" alt="Screenshot 2024-01-07 at 16 23 50" src="https://github.com/maplibre/maplibre-native/assets/649392/a0440e05-c522-49ba-ae85-3fc2193de465">
