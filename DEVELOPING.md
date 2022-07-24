# Developing

This is been rewritten to include:

- How to build for the platforms we support.
- Acceptance criteria for code contributions (style, static asserts)
- How to run the unit tests.
- How to run the benchmarks.
- How to rebaseline baselines metrics.
- How to use GL Native as a 3rd party library in your project.

## Render Tests

To check that the output of the rendering is correct, we compare actual rendered PNGs for simple styles with expected PNGs. The content of the tests is stored in the MapLibre GL JS submodule which means that GL JS and Native are in fact quasi pixel-identical in their rendering.

The directory sturcture of the render tests looks like:

```
maplibre-gl-js/
  test/
    integration/
      render-tests/
        <name-of-style-spec-feature>/
          <name-of-feature-value>/
            expected.png
            style.json
```

After the render test run, the folder will also contain an `actual.png` file and a `diff.png` which is the difference between the expected and the actual image. There is a pixel difference threshold value which is used to decide if a render test passed or failed.

### Building the render test runner

Run the following on linux to build the render test runner:

```
git submodule update --init --recursive --depth 1

sudo apt update
sudo apt install ccache cmake ninja-build pkg-config xvfb libcurl4-openssl-dev libglfw3-dev libuv1-dev g++-10 libc++-9-dev libc++abi-9-dev libpng-dev libgl1-mesa-dev libgl1-mesa-dri libjpeg-turbo8 libicu66 libjpeg-dev

cmake . -B build -G Ninja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER=gcc-10 -DCMAKE_CXX_COMPILER=g++-10
cmake --build build -j $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null)
```


### Running the render tests

On linux, run all render tests with:

```
./build/mbgl-render-test-runner --manifestPath metrics/linux-clang8-release-style.json
```

Or a single test with:

```
./build/mbgl-render-test-runner --manifestPath metrics/linux-clang8-release-style.json --filter "render-tests/fill-visibility/visible"
```

On macOS, run all render tests with:

```
./build/mbgl-render-test-runner --manifestPath metrics/macos-xcode11-release-style.json
```

### Inspecting render test results

The render test results are summarized in a HTML website located next to the manifest file. For example, running `metrics/linux-clang8-release-style.json` produces a summary at `metrics/linux-clang8-release-style.html`.
