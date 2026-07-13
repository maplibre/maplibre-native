# iOS Documentation

We use [DocC](https://www.swift.org/documentation/docc) for the MapLibre iOS documentation. The live documentation site can be found [here](https://maplibre.org/maplibre-native/ios/latest/documentation/maplibre/).

## Resources

You need to have [aws-cli](https://github.com/aws/aws-cli) installed to download the resources from S3 (see below). Run the following command:

```
aws s3 sync --no-sign-request "s3://maplibre-native/ios-documentation-resources" "platform/ios/MapLibre.docc/Resources"
```

Then, to build the documentation locally, run the following command:

```
platform/ios/scripts/docc.sh preview
```

Resources like images should not be checked in but should be uploaded to the [S3 Bucket](https://s3.eu-central-1.amazonaws.com/maplibre-native/index.html#ios-documentation-resources/). You can share a `.zip` with all files that should be added in the PR.

If you want to get direct access you need an AWS account to get permissions to upload files. Create an account and authenticate with aws-cli. Share the account ARN that you can get with

```
aws sts get-caller-identity
```

## Examples

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
