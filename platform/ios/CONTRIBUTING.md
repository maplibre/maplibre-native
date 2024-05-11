# Documentation

We use [DocC](https://www.swift.org/documentation/docc) for documentation. To build the documentation locally, run the following command from the root directory of the repository:

```
platform/ios/scripts/docc.sh preview
```

You need to have [aws-cli](https://github.com/aws/aws-cli) installed to download the resources from S3 (see below).

## Resources

Resources like images should not be checked in but should be uploaded to the [S3 Bucket](https://eu-central-1.console.aws.amazon.com/s3/buckets/maplibre-native?region=eu-central-1&bucketType=general&prefix=ios-documentation-resources/&showversions=false). Log in as the `maplibre` IAM User. Enter the following details:

| Field          | Value    |
|----------------|----------|
| Account ID     | maplibre |
| IAM user name  | maplibre |
| Password       | maplibre |

Upload the needed resources and add the filenames to the `resources` array in `platform/ios/scripts/docc.sh`.

```bash
resources=(
  "AddPackageDependencies@2x.png"
  # ...
  # add here
)
```
