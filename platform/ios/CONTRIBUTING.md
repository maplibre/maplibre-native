# Documentation

We use [DocC](https://www.swift.org/documentation/docc) for documentation. To build the documentation locally, run the following command from the root directory of the repository:

```
platform/ios/scripts/docc.sh preview
```

You need to have [aws-cli](https://github.com/aws/aws-cli) installed to download the resources from S3 (see below).

## Resources

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

You need to add the relevant example paths and documentation paths to that file.