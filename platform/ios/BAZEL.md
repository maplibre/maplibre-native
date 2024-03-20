# Building using Bazel

[Bazel](https://bazel.build) is also supported as a build option for getting a packaged release of the xcframework compiled for either static or dynamic linking.

Firstly you will have to ensure that Bazel is installed

`brew install bazelisk`

From there you can use the script in platform/ios/platform/ios/scripts/package-bazel.sh

## There are 4 options:

`cd platform/ios/platform/ios/scripts`

Static xcframework compiled for release (this is default if no parameters are provided):
`./bazel-package.sh --static --release`

Static xcframework compiled for debug:
`./bazel-package.sh --static --debug`

Dynamic xcframework compiled for release:
`./bazel-package.sh --dynamic --release`

Dynamic xcframework compiled for debug:
`./bazel-package.sh --dynamic --debug`

All compiled frameworks will end up in the `bazel-bin/platform/ios/` path from the root of the repo.

Also you can use the link option to ensure that the framework is able to link.

`./bazel-package.sh --link`

#### Bazel build files are placed in a few places throughout the project:

`BUILD.bazel`
- Covering the base cpp in the root `src` directory.

`vendor/BUILD.bazel`
- Covering the submodule dependencies of Maplibre.

`platform/default/BUILD.bazel`
- Covering the cpp dependencies in default.

`platform/darwin/BUILD.bazel`
- Covering the cpp source in platform/default.

`platform/ios/platform/ios/vendor/`
- Covering the iOS specific dependencies.

`platform/ios/BUILD.bazel`
- Covering the source in `platform/ios/platform/ios/src` and `platform/ios/platform/darwin/src` as well as defining all the other BUILD.bazel files and defining the xcframework targets.

## There are also some other areas that make bazel work:

`WORKSPACE`
- Defines the "repo" and the different modules that are loaded in order to compile for Apple.

`.bazelversion`
- Defines the version of bazel used, important for specific support for Apple targets.

`bazel/flags.bzl`
- Defines some compilation flags that are used between the different build files. 