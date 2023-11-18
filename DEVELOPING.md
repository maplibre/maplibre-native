# Developing


## Pre-commit hooks

Install [pre-commit](https://pre-commit.com/) and run

```
pre-commit install
```

to install the pre-commit hooks configured in `.pre-commit-config.yml`.

## Render Tests

To check that the output of the rendering is correct, we compare actual rendered PNGs for simple styles with expected PNGs. The content of the tests used to be stored in the MapLibre GL JS repository, which means that GL JS and Native are mostly pixel-identical in their rendering.

The directory structure of the render tests looks like:

```
metrics/
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
sudo apt install ccache cmake ninja-build pkg-config xvfb libcurl4-openssl-dev libglfw3-dev libuv1-dev g++-10 libc++-dev libc++abi-dev libpng-dev libgl1-mesa-dev libgl1-mesa-dri libjpeg-turbo8 libicu-dev libjpeg-dev

cmake . -B build -G Ninja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER=gcc-10 -DCMAKE_CXX_COMPILER=g++-10
cmake --build build -j $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null)
```

Also, if your system supports OpenGL ES 3+, you can now execute the GLFW demo with:

```
MLN_API_KEY=add_maptiler_api_key_here ./build/platform/glfw/mbgl-glfw
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

---

# MapLibre & C++

MapLibre makes use of a common set of C++ files for iOS, macOS, Android, Linux & Qt.  See [`platform/default/src/mbgl/`](`platform/default/src/mbgl/`),
or any of the platform make files:

* [`platform/android/android.cmake`](platform/android/android.cmake)
* [`platform/ios/ios.cmake`](platform/ios/ios.cmake)
* [`platform/linux/linux.cmake`](platform/linux/linux.cmake)
* [`platform/macos/macos.cmake`](platform/macos/macos.cmake)
* [`platform/qt/qt.cmake`](platform/qt/qt.cmake)

## Debugging

* Android developers can use [Android Studio](https://developer.android.com/studio/debug) or [`ndk-gdb`](https://developer.android.com/ndk/guides/ndk-gdb)
* iOS developers can use [Xcode](https://developer.apple.com/support/debugging/).  See also [Advanced Debugging with Xcode and LLDB](https://developer.apple.com/videos/play/wwdc2018/412/).

## Static Analysis

We use [`clang-tidy`](https://clang.llvm.org/extra/clang-tidy/) for static analysis and run it on CI for each pull request. If you want to run it locally use `-DMLN_WITH_CLANG_TIDY=ON` CMake option and just run regular build. For the list of enabled checks please see:
 [`.clang-tidy`](.clang-tidy) and [`test/.clang-tidy`](test/.clang-tidy)(for tests we are less strict and use different set of checks).

## Logging in C++

* Android developers can review NDK logging at [developer.android.com/ndk/reference/group/logging](https://developer.android.com/ndk/reference/group/logging).
* iOS developers can review Logging at [developer.apple.com/documentation/os/logging](https://developer.apple.com/documentation/os/logging).

### Example Cross Compile Logging for Android & iOS

Add this to your header.

```c++
#include <iostream>
#include <sstream>
#define LOG_TAG "# MapLibre "

#ifdef __ANDROID__
  #include <android/log.h>
#endif
```

Then you can use this sample code which compiles for both Android & Xcode.

```c++
std::stringstream message;

// Set your message to log.
message << LOG_TAG << __FUNCTION__ << " req->resource.url = " << req->resource.url << std::endl;

// Logs to Xcode console.
std::cout << message.str();

#ifdef __ANDROID__
// Logs to Android Logcat.
__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s", message.str().c_str());
#endif
```

Which will log the following samples.

#### Android Sample Logging Output

> com.mapbox.mapboxsdk.testapp D/# MapLibre: online_file_source.cpp: # MapLibre: online_file_source.cpp activateRequest req->resource.url = https://api.maptiler.com/tiles/v3/tiles.json

#### Xcode Sample Logging Output

> MapLibre: online_file_source.cpp activateRequest req->resource.url = https://demotiles.maplibre.org/style.json

---

### Autocomplete with `make`

MapLibre makes use of several command line tools for local and cloud builds.
To see what targets exist you have to review the `Makefile`, which can be tedious.

There is a better way with the Zsh.  While in a folder with a `Makefile`, you can type `make` followed by hitting `<tab>` twice.


```zsh
cd platform/android
make android # <tab><tab>
# Example, Android make targets starting with `ap`
# apackage  aproj

# ----

cd platform/ios
make ios # <tab><tab>
# Example, iOS make targets starting with `ios`
# ios ios-lint ios-sanitize ios-test

make macos # <tab><tab>
# Example, macOS make targets starting with `macos`
# macos       macos-lint  macos-test
```

To add this feature add the following to your zsh.

```zsh
# open ~/.zprofile

# Autocomplete for `make` when in a directory
zstyle ':completion:*:*:make:*' tag-order 'targets'
autoload -Uz compinit && compinit
```

### Kotlin and Java compatibility

We are moving the Android SDK to Kotlin, which is backward compatible with Java, but if you need a Java version of the Android SDK there is a `before-kotlin-port` tag available.



