# MapLibre Android Tests

## Render Tests

To run the render tests for Android, run the configuration for the `androidRenderTest.app` module.

### Filtering Render Tests

You can filter the tests to run by passing a flag to the file `platform/android/src/test/render_test_runner.cpp`:

```cpp
std::vector<std::string> arguments = {..., "-f", "background-color/literal"};
```

### Viewing the Results

Once the application quits, use the Device Explorer to navigate to `/data/data/org.maplibre.render_test_runner/files`.

<img width="980" alt="image" src="https://github.com/maplibre/maplibre-native/assets/649392/0dc42d4a-6221-46b6-8352-1eb24d466e91">

Double click `android-render-test-runner-style.html`. Right click on the opened tab and select _Open In > Browser_. You should see that a single render test passed.

<img width="801" alt="image" src="https://github.com/maplibre/maplibre-native/assets/649392/33e88999-7787-492f-afd9-f5a3e3fd61f7">

Alternatively to download (and open) the results from the command line, use:

```
filename=android-render-test-runner-style.html
adb shell "run-as org.maplibre.render_test_runner cat files/metrics/$filename" > $filename
open $filename
```

For Vulkan use

```
filename=android-vulkan-render-test-runner-style.html
```

### Updating the Render Tests

Now let's edit `metrics/integration/render-tests/background-color/literal/style.json`, change this line:

```
        "background-color": "red"
```

to

```
        "background-color": "yellow"
```

We need to make sure that the new `data.zip` with the data for the render tests is installed on the device. You can use the following commands:

```
tar chf render-test/android/app/src/main/assets/data.zip --format=zip --files-from=render-test/android/app/src/main/assets/to_zip.txt
adb push render-test/android/app/src/main/assets/data.zip /data/local/tmp/data.zip
adb shell chmod 777 /data/local/tmp/data.zip
adb shell "run-as org.maplibre.render_test_runner unzip -o /data/local/tmp/data.zip -d files"
```

Rerun the render test app and reload the Device Explorer. When you re-open the HTML file with the results you should now see a failing test:

<img width="685" alt="image" src="https://github.com/maplibre/maplibre-native/assets/649392/303ad75a-cf74-4b8c-927c-c9bd59a79de4">

Now download the `actual.png` in `metrics/integration/render-tests/background-color/literal` with the Device Explorer. Replace the corresponding `expected.png` on your local file system. Upload the new render test data again and run the test app once more.

<img width="800" alt="image" src="https://github.com/maplibre/maplibre-native/assets/649392/04734c6a-9cf1-489a-b9bb-8d857581261c">

Of we don't want to commit this change. But know you can add and debug (Android) render tests.

## Instrumentation Tests

To run the instrumentation tests, choose the "Instrumentation Tests" run configuration.

Your device needs remain unlocked for the duration of the tests.

## C++ Unit Tests

There is a separate Gradle project that contains a test app which runs the C++ Unit Tests. It does not depend on the Android platform implementations.

You can find the project in `test/android.` You can open this project in Android Studio and run the C++ Tests on an Android device or Simulator.

To run a particular set of tests you can modify the `--gtest_filter` flag in `platform/android/src/test/test_runner.cpp`. See the [GoogleTest documentation](https://google.github.io/googletest/advanced.html#running-a-subset-of-the-tests) for details how to use this flag.

### AWS Device Farm

The instrumentation tests and C++ unit tests are running on AWS Device Farm. To see the results and the logs, go to:

[https://us-west-2.console.aws.amazon.com/devicefarm/home?region=us-east-1#/mobile/projects/20687d72-0e46-403e-8f03-0941850665bc/runs](https://us-west-2.console.aws.amazon.com/devicefarm/home?region=us-east-1#/mobile/projects/20687d72-0e46-403e-8f03-0941850665bc/runs).

Use the following login details (this is a read-only account):

|            |            |
|------------|------------|
| Alias      | `maplibre`   |
| Username   | `maplibre`   |
| Password   | `maplibre`   |
