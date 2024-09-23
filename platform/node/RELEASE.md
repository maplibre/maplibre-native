# Instructions for making a Node release

1. Open a command prompt to 'platform/node'

cd platform/node

2. Change the version number in package.json. on the command line, in the package root directory, run the following command, replacing <update_type> with one of the semantic versioning release types (prerelease, prepatch, preminor, premajor, patch, minor, major):

npm version <update_type> --preid pre --no-git-tag-version

3. Update the changelog, which can be found in `platform/node/CHANGELOG.md`. The heading must match `## <VERSION>` exactly, or it will not be picked up. For example, for version 6.0.0:

```
## 6.0.0
```

4. Create a PR to merge your changes into the main branch. Once merged the 'node-release' workflow will automaticlly check if the release has been published on npm. If the release has not yet been published, the workflow will build the node binaries and upload them to a new github release, then publish a new npm release.
