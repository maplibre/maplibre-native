# Benchmark

We have created a rendering performance benchmark for Android. The logic for this benchmark is encapsulated in `BenchMarkActivity.kt`.

## Styles

We have hardcoded various styles, which you can override with a `developer-config.xml` by adding some XML with the following structure:

```
<array name="benchmark_style_names">
  <item>Americana</item>
</array>
<array name="benchmark_style_urls">
  <item>https://americanamap.org/style.json</item>
</array>
```

## Retrieving Results

Results are logged to the console as they come in. After the benchmark is done running a `benchmark_results.json` file is generated. You can pull it off the device with for example adb:

```
adb shell "run-as org.maplibre.android.testapp cat files/benchmark_results.json" \
  > benchmark_results.json
```

## Results

The `benchmark_results.json` containing the benchmark results will have the following structure.

```json
{
  "results": [
    {
      "styleName": "AWS Open Data Standard Light",
      "syncRendering": true,
      "thermalState": 0,
      "fps": 34.70237085812839,
      "avgEncodingTime": 19.818289053808947,
      "avgRenderingTime": 7.7432454899654255,
      "low1pEncodingTime": 91.72538138784371,
      "low1pRenderingTime": 26.573758581509203
    },
  ],
  "deviceManufacturer": "samsung",
  "model": "SM-G973F",
  "renderer": "drawable",
  "debugBuild": true,
  "gitRevision": "fc95b79880223e34c2ce80339f698d095e3d63cd",
  "timestamp": 1736454325393
}
```

The meaning of the keys is as follows.

| Key | Description |
|---|---|
| styleName | Name of the style being benchmarked |
| syncRendering | Whether synchronous rendering was used |
| thermalState | Thermal state of the device during benchmark |
| fps | Average frames per second achieved during the benchmark |
| avgEncodingTime | Average time taken for encoding in milliseconds |
| avgRenderingTime | Average time taken for rendering in milliseconds |
| low1pEncodingTime | 1st percentile (worst case) encoding time in milliseconds |
| low1pRenderingTime | 1st percentile (worst case) rendering time in milliseconds |
| deviceManufacturer | Manufacturer of the test device |
| model | Model number/name of the test device |
| renderer | Type of renderer used (`drawable` for Open GL ES, `vulkan` for Vulkan, `legacy` for the legacy Open GL ES rendering backend) |
| debugBuild | Whether the build was a debug build |
| gitRevision | Git commit hash of the code version |
| timestamp | Unix timestamp of when the benchmark was run |

## Large Scale Benchmarks on AWS Device Farm

Sometimes we do a large scale benchmark across a variety of devices on AWS Device Farm. We ran one such test in [November 2024](https://github.com/maplibre/maplibre-native/issues/2787#issuecomment-2466948888) to compare the performance of the then new Vulkan rendering backend against the OpenGL ES backend. There are some scripts in the repo to kick off the tests and to collect and plot the results:

```
scripts/aws-device-farm/aws-device-farm-run.sh
scripts/aws-device-farm/collect-benchmark-outputs.mjs
scripts/aws-device-farm/update-benchmark-db.mjs
scripts/aws-device-farm/plot-android-benchmark-results.py
```

## Continuous Benchmarking

We are running the Android benchmark on every merge with `main`.

You can find the results per commit [here](https://maplibre-native.s3.eu-central-1.amazonaws.com/index.html#android-benchmark-render/) or pull them from our public S3 bucket:

```
aws s3 sync s3://maplibre-native/android-benchmark-render/ .
```

## Benchmarks in Pull Request

To run the benchmarks (for Android) include the following line on a PR comment:

```
!benchmark android
```

A file with the benchmark results will be added to the workflow summary, which you can compare with the previous results in the bucket.
