# MapLibre Android Examples

To build the Examples Documentation you need to have Docker installed.

From `platform/android`, run:

```
make mkdocs
```

Next, visit http://localhost:8000/maplibre-native/android/examples/

## Snippets

We use [a Markdown extension for snippets](https://facelessuser.github.io/pymdown-extensions/extensions/snippets/#snippet-sections). This way code can be referenced instead of copy pasted into the documentation. This avoids code examples from becoming out of date or failing to compile. The syntax is as follows:

````kotlin
// --8<-- [start:fun]
fun double(x: Int): Int {
    return 2 * x
}
// --8<-- [end:fun]
````

Next, you'll be able to reference that piece of code in Markdown like so:

```
--8<-- "example.kt:fun"
```

Where `example.kt` is the path to the file.

## Static Assets

Static assets are ideally uploaded to the [MapLibre Native S3 Bucket](https://maplibre-native.s3.eu-central-1.amazonaws.com/index.html#android-documentation-resources/).

Please open an issue with the ARN of your AWS account to get upload privileges.

Use the CDN URL `https://dwxvn1oqw6mkc.cloudfront.net/` instead of linking to the S3 bucket directly.
