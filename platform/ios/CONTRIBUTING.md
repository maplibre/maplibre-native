# Contributing

## Downloading Source

Download the source and install all submodules if you have not already, by running the following from the root of the repository. 

```
git clone --recurse-submodules git@github.com:maplibre/maplibre-native.git
cd maplibre-native
```

## Bazel

[Bazel](https://bazel.build/) is used for building on iOS.

You can generate an Xcode project thanks to [rules_xcodeproj](https://github.com/MobileNativeFoundation/rules_xcodeproj) intergration. 

You need to install [bazelisk](https://github.com/bazelbuild/bazelisk), which is a wrapper around Bazel which ensures that the version specified in `.bazelversion` is used.

```
brew install bazelisk
```

### Creating `config.bzl`

> [!NOTE]  
> This is optional.

You may want to configure Bazel, otherwise the default config gets used.

```
cp platform/darwin/bazel/example_config.bzl platform/darwin/bazel/config.bzl
```

You need to set your `BUNDLE_ID_PREFIX` to be unique (ideally use a domain that you own in reverse domain name notation).

You can keep leave the `APPLE_MOBILE_PROVISIONING_PROFILE_NAME` alone.

Set the Team ID to the Team ID of your Apple Developer Account (paid or unpaid both work).
If you do not know your Team ID, go to your [Apple Developer account](https://developer.apple.com/account), log in, and scroll down to find your Team ID.

If you don't already have a developer account, continue this guide and let Xcode generate a provisioning profile for you. You will need to update the Team ID later once a certificate is generated.

If you are encountering build errors due to provisioning profiles, skip down to the troubleshooting section.

## Create the Xcode Project

Run the following commands:

```
bazel run //platform/ios:xcodeproj --@rules_xcodeproj//xcodeproj:extra_common_flags="--//:renderer=metal"
xed platform/ios/MapLibre.xcodeproj
```

Then once in Xcode, click on "MapLibre" on the left, then "App" under Targets, then "Signing & Capabilities" in the tabbed menu. 
Confirm that no errors are shown:

<img width="921" alt="image" src="https://github.com/polvi/maplibre-native/assets/649392/a1ef30cb-97fc-429a-acee-194436f3fb8a">

Try to run the example App in the simulator and on a device to confirm your setup works.

### Troubleshooting Provisioning Profiles

If you get a Python `KeyError` when processing provisioning profiles, you probably have some _really_ old or corrupted profiles.

Have a look through `~/Library/MobileDevice/Provisioning\ Profiles` and remove any expired profiles.

## Using Bazel from the Command Line

It is also possible to build and run the test application in a simulator from the command line without opening Xcode.

```
bazel run //platform/ios:App
```

If you want to build your own XCFramework, see the 'Build XCFramework' step in the [iOS CI workflow](../../.github/workflows/ios-ci.yml).

## Render Tests

To run the render tests, run the `RenderTest` target from iOS.

When running in a simulator, use

```
# check for 'DataContainer' of the app with `*.maplibre.RenderTestApp` id
xcrun simctl listapps booted
```

to get the data directory of the render test app. This allows you to inspect test results. When adding new tests, the generated expectations and `actual.png` file can be copied into the source directory from here.

## Documentation

We use [DocC](https://www.swift.org/documentation/docc) for documentation. To build the documentation locally, run the following command from the root directory of the repository:

```
platform/ios/scripts/docc.sh preview
```

You need to have [aws-cli](https://github.com/aws/aws-cli) installed to download the resources from S3 (see below).

### Resources

Resources like images should not be checked in but should be uploaded to the [S3 Bucket](https://s3.eu-central-1.amazonaws.com/maplibre-native/index.html#ios-documentation-resources/). You can share a `.zip` with all files that should be added in the PR.

If you want to get direct access you need an AWS account to get permissions to upload files. Create an account and authenticate with aws-cli. Share account's ARN that you can get with

```
aws sts get-caller-identity
```

When the needed resources are uploaded the filenames need to be added to the `resources` array in `platform/ios/scripts/docc.sh`.

```bash
resources=(
  "AddPackageDependencies@2x.png"
  # ...
  # add here
)
```

### Examples

The code samples in the documentation should ideally be compiled on CI so they do not go out of date.

Fence your example code with

```swift
// #-example-code(LineTapMap)
...
// #-end-example-code
```

Prefix your documentation code block with

````md
<!-- include-example(LineTapMap) -->

```swift
...
```
````

Then the code block will be updated when you run:

```sh
node scripts/update-ios-examples.mjs
```

You need to add the relevant example paths and documentation paths to that file.