# Render Tests

> [!NOTE]
> See also [Android Tests](./android/android-tests.md#render-tests) and [iOS Tests](./ios/ios-tests.md#render-tests) for some platform-specific information on the render tests.

Render tests verify the correctness and consistency of MapLibre Native's rendering. Note that 'render test' is a bit of a misnomer, because there are various types of tests that do not really test rendering behavior that we sometimes call render tests. Examples are [expression tests and query tests](#metricsintergration). In addition, these 'render tests' allow a wide variety of operations and probes (which write out metrics) for things like GPU memory allocations, memory usage, network requests, FPS, so these tests are really quite a bit more versatile than just verifying rendering behavior.

## Render Test Runner CLI Options

When using CMake, the render test runner is an executable available as `mbgl-render-test-runner` in the build directory.

| Option / Argument                                  | Description                                                                                                                                                                                                                                                                                          | Required? |
| :------------------------------------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :-------- |
| `-h`, `--help`                                     | Display the help menu and exit.                                                                                                                                                                                                                                                                      | No        |
| `-r`, `--recycle-map`                              | Toggle reusing the map object between tests. If set, the map object is reused; otherwise, it's reset for each test.                                                                                                                                                                                        | No        |
| `-s`, `--shuffle`                                  | Toggle shuffling the order of tests based on the manifest file.                                                                                                                                                                                                                                      | No        |
| `-o`, `--online`                                   | Toggle online mode. If set, tests can make network requests. By default (`--online` not specified), tests run in offline mode, forcing resource loading from the cache only.                                                                                                                           | No        |
| `--seed <uint32_t>`                                | Set the seed for shuffling tests. Only relevant if `--shuffle` is also used. Defaults to `1` if `--shuffle` is used without `--seed`.                                                                                                                                                                   | No        |
| `-p`, `--manifestPath <string>`                    | Specifies the path to the test manifest JSON file, which defines test configurations, paths, and potentially filters/ignores.                                                                                                                                                                          | Yes       |
| `-f`, `--filter <string>`                          | Provides a regular expression used to filter which tests (based on their path/ID) should be run. Only tests matching the regex will be executed.                                                                                                                                                     | No        |
| `-u`, `--update default \| platform \| metrics \| rebaseline` | Sets the mode for updating test expectation results: <br> - `default`: Updates generic render test expectation images/JSON. <br> - `platform`: Updates platform-specific render test expectation images/JSON. <br> - `metrics`: Updates expected metrics for the configuration defined by the manifest. <br> - `rebaseline`: Updates or creates expected metrics for the configuration defined by the manifest. | No        |


## Source Code Organization

### `render-test`

This directory contains the C++ source code for common render test runner, manifest parser and CLI tool.

- `render-test/android`: standalone Gradle project with a app that runs the render test runner.
- `render-tests/ios`: source code for Objective-C app that encapsulates the render test runner.
property: `background-color`, `line-width`, etc., with a second level of directories below that for individual tests.

### `metrics`

The JSON files in this directory are the manifests (to be passed to render test CLI tool). This directory also contains many directories that store metrics used by the tests. Other files/directories include:

- `cache-style.db`, `cache-metrics.db`: pre-populated cache (SQLite database file) so the tests can run offline. You may need to update this database when you need a new resource available during test executation.
- `binary-size`: binary-size checks. Not used right now, see [#3379](https://github.com/maplibre/maplibre-native/issues/3379).
- `expectations`: expectations for various platforms. E.g. `expectations/platform-android` is referenced by the `android-render-test-runner-metrics.json` manifest under its `expectation_paths` key.
- `ignores`: contains JSON files with as key a test path and as value a reason why a test is ignored. For example:
   ```
   {
      "expression-tests/collator/accent-equals-de": "Locale-specific behavior changes based on platform."
   }
   ```
   Manifests can have a key `ignore_paths` with an array of ignore files. For example:
   ```
   {
      ...
      "ignore_paths": [
         "ignores/platform-all.json",
         "ignores/platform-linux.json",
         "ignores/platform-android.json"
      ],
      ...
   }
   ```

#### `metrics/intergration`

- `data`, `geojson`, `glyphs`, `image`, `sprites`, `styles`, `tiles`, `tilesets`, `video`: various data used by the render tests.
- `expression-tests`: tests that verify the behavior of [expressions](https://maplibre.org/maplibre-style-spec/expressions/) of the MapLibre Style Spec.
- `query-tests`: these tests test the behavior of the `queryRenderedFeatures` API which, as the name suggests, allow you to query the rendered features.
- `render-tests`: location of render tests. Each render test contains a `style.json` and an `expected.png` image. This directory tree is generally organized by [style specification](https://maplibre.org/maplibre-style-spec/) properties.

## Running tests

To run the entire integration test suite (both render or query tests), from within the maplibre-native directory on Linux run the command:

```
./build/mbgl-render-test-runner --manifestPath metrics/linux-clang8-release-style.json
```

### Running specific tests

To run a subset of tests or an individual test, you can pass a specific subdirectory to the `mbgl-render-test-runner` executable. For example, to run all the tests for a given property, e.g. `circle-radius`:

```
$ build-macos-vulkan/mbgl-render-test-runner --manifestPath=metrics/macos-xcode11-release-style.json -f 'circle-radius/.*'
* passed query-tests/circle-radius/feature-state
* passed query-tests/circle-radius/zoom-and-property-function
* passed query-tests/circle-radius/property-function
* passed query-tests/circle-radius/outside
* passed query-tests/circle-radius/tile-boundary
* passed query-tests/circle-radius/inside
* passed query-tests/circle-radius/multiple-layers
* passed render-tests/circle-radius/zoom-and-property-function
* passed render-tests/circle-radius/literal
* passed render-tests/circle-radius/property-function
* passed render-tests/circle-radius/default
* passed render-tests/circle-radius/function
* passed render-tests/circle-radius/antimeridian
13 passed (100.0%)
Results at: /Users/bart/src/maplibre-native/metrics/macos-xcode11-release-style.html
```

## Writing new tests

To add a new render test:

1. Create a new directory `test/integration/render-tests/<property-name>/<new-test-name>`

2. Create a new `style.json` file within that directory, specifying the map to load. Feel free to copy & modify one of the existing `style.json` files from the `render-tests` subdirectories.

3. Generate an `expected.png` image from the given style with
   ```
   $ ./build/mbgl-render-test-runner --update default --manifestPath=... -f '<property-name>/<new-test-name>'
   ```

4. Manually inspect `expected.png` to verify it looks as expected.

5. Commit the new `style.json` and `expected.png`.
