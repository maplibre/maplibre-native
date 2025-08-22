# MapLibre Native Development Instructions

**ALWAYS follow these instructions first and only search for additional information if these instructions are incomplete or found to be in error.**

MapLibre Native is a cross-platform C++ library for rendering interactive maps. It supports multiple platforms including Android, iOS, Linux, macOS, Windows, Node.js, and Qt through a monorepo structure with platform-specific SDKs.

## Quick Reference - Essential Commands

### Clone and Setup (Required First Steps)
```bash
# Clone with all submodules (REQUIRED - 35 submodules, ~1 minute)
git clone --recurse-submodules https://github.com/maplibre/maplibre-native.git
cd maplibre-native

# Initialize submodules if not done during clone
git submodule update --init --recursive  # Takes ~1 minute 8 seconds
```

### Linux Development (Ubuntu 24.04 LTS)

#### Dependencies Installation
```bash
# Install dependencies (takes ~1 minute 4 seconds for cargo install)
.github/scripts/install-linux-deps

# Essential packages installed:
# - libcurl4-openssl-dev libuv1-dev libjpeg-dev libpng-dev 
# - libglfw3-dev libwebp-dev libopengl0 mesa-vulkan-drivers llvm
# - armerge (Rust tool for merging static libraries)
```

#### Build Process - NEVER CANCEL builds, they take significant time
```bash
# Configure builds (5-8 seconds each)
cmake --preset linux-opengl     # OpenGL renderer
cmake --preset linux-vulkan     # Vulkan renderer 

# Core library build (SET TIMEOUT TO 60+ MINUTES, NEVER CANCEL)
# Takes ~15 minutes first time, ~7 minutes with ccache  
cmake --build build-linux-opengl --target mbgl-core -j $(nproc)

# Test executables (SET TIMEOUT TO 30+ MINUTES, NEVER CANCEL)
# Takes ~7 minutes
cmake --build build-linux-opengl --target mbgl-test-runner mbgl-render-test-runner mbgl-expression-test -j $(nproc)

# Utility executables (SET TIMEOUT TO 15+ MINUTES, NEVER CANCEL)  
# Takes ~1 minute
cmake --build build-linux-opengl --target mbgl-render mbgl-benchmark-runner mbgl-glfw -j $(nproc)
```

#### Testing and Validation (NEVER CANCEL - Wait for completion)
```bash
# C++ unit tests (SET TIMEOUT TO 15+ MINUTES, NEVER CANCEL)
# Takes 4 minutes 44 seconds - 995 tests, 18 disabled
xvfb-run -a build-linux-opengl/mbgl-test-runner

# Expression tests (very fast - 36ms)
build-linux-opengl/expression-test/mbgl-expression-test

# Render tests (SET TIMEOUT TO 10+ MINUTES, NEVER CANCEL)
# Takes 2 minutes 46 seconds - 1243 passed tests
xvfb-run -a build-linux-opengl/mbgl-render-test-runner --manifestPath=metrics/linux-opengl.json

# Benchmark tests (requires OpenGL context, may fail in headless environment)
build-linux-opengl/mbgl-benchmark-runner --benchmark_min_time=0.01s
```

#### Node.js Bindings
```bash
# Configure Node.js build (downloads headers for multiple Node versions)
# Takes ~8 seconds
cmake --preset linux-opengl -DMLN_WITH_NODE=ON -DMLN_WITH_WERROR=OFF -B build-linux-node

# Build Node.js bindings (SET TIMEOUT TO 60+ MINUTES, NEVER CANCEL)
cmake --build build-linux-node -j $(nproc)
```

### iOS Development (macOS required)

#### Prerequisites
```bash
# Install bazelisk (wrapper for Bazel)
brew install bazelisk

# Verify Bazel version matches .bazelversion file
bazel --version
```

#### Xcode Project Generation  
```bash
# Generate Xcode project for iOS with Metal renderer
# NEVER CANCEL - can take several minutes for dependency resolution
bazel run //platform/ios:xcodeproj --@rules_xcodeproj//xcodeproj:extra_common_flags="--//:renderer=metal"

# Open generated project
xed platform/ios/MapLibre.xcodeproj
```

#### Command Line iOS Build
```bash
# Build and run iOS app in simulator (NEVER CANCEL - can take 30+ minutes)
bazel run //platform/ios:App --//:renderer=metal
```

#### CMake Alternative for iOS
```bash
# Generate iOS Xcode project using CMake
cmake --preset ios -DDEVELOPMENT_TEAM_ID=YOUR_TEAM_ID
xed build-ios/MapLibre\ Native.xcodeproj
```

### Android Development

#### Setup
```bash
# Open Android project in Android Studio
# Project location: platform/android
```

#### Command Line Build
```bash
cd platform/android
# Use Gradle wrapper for builds (SET TIMEOUT TO 60+ MINUTES for first build)
./gradlew assembleDebug
```

### Key Executables and Their Purpose

#### After successful Linux build, you will have:
```bash
# Test runners
build-linux-opengl/mbgl-test-runner           # C++ unit tests (4m 44s runtime)
build-linux-opengl/mbgl-render-test-runner    # Render tests (2m 46s runtime)  
build-linux-opengl/expression-test/mbgl-expression-test  # Expression tests (36ms runtime)

# Utilities
build-linux-opengl/bin/mbgl-render            # Command-line map renderer
build-linux-opengl/mbgl-benchmark-runner      # Performance benchmarks
build-linux-opengl/platform/glfw/mbgl-glfw    # GLFW demo application
```

#### Using mbgl-render utility
```bash
# Basic usage (requires network access for style/tiles)
xvfb-run -a build-linux-opengl/bin/mbgl-render \
  --style https://raw.githubusercontent.com/maplibre/demotiles/gh-pages/style.json \
  --output out.png

# Get full help
build-linux-opengl/bin/mbgl-render --help
```

## Platform-Specific Instructions

### Linux Build Variants
```bash
# Available CMake presets:
cmake --list-presets

# Key presets:
# linux-opengl   - OpenGL renderer (default for Linux development)
# linux-vulkan   - Vulkan renderer (experimental)
```

### macOS Development
```bash
# macOS presets
cmake --preset macos           # Metal renderer
cmake --preset macos-vulkan    # Vulkan renderer via MoltenVK
cmake --preset macos-node      # Node.js bindings
cmake --preset macos-core      # Core library only
```

### Windows Development  
```bash
# Windows presets
cmake --preset windows-opengl      # OpenGL renderer
cmake --preset windows-vulkan      # Vulkan renderer
cmake --preset windows-egl         # EGL renderer
```

## Common Validation Steps

### Always run these after making changes:
```bash
# 1. Build core library
cmake --build build-linux-opengl --target mbgl-core -j $(nproc)

# 2. Run unit tests to verify no regressions
xvfb-run -a build-linux-opengl/mbgl-test-runner

# 3. Test basic rendering functionality
xvfb-run -a build-linux-opengl/bin/mbgl-render \
  --style https://raw.githubusercontent.com/maplibre/demotiles/gh-pages/style.json \
  --output test.png
```

### Generate and check render test results
```bash
# Run render tests and check HTML report
xvfb-run -a build-linux-opengl/mbgl-render-test-runner --manifestPath=metrics/linux-opengl.json
# View results at: metrics/linux-opengl.html
```

## Important Repository Structure

### Core C++ Code
- `include/` - Public C++ API headers
- `src/` - C++ implementation files  
- `platform/` - Platform-specific SDK code

### Platform Directories
- `platform/android/` - Android SDK
- `platform/ios/` - iOS SDK  
- `platform/darwin/` - Shared macOS/iOS code
- `platform/node/` - Node.js bindings
- `platform/linux/` - Linux development platform
- `platform/qt/` - Qt bindings
- `platform/glfw/` - GLFW demo/development app

### Build and Test
- `test/` - C++ unit tests
- `render-test/` - Image-based render tests  
- `expression-test/` - Expression feature tests
- `benchmark/` - Performance benchmarks
- `metrics/` - Test manifests and baseline images

### Build Configuration  
- `CMakeLists.txt` - Main CMake configuration
- `CMakePresets.json` - Platform-specific build presets
- `BUILD.bazel` - Bazel build configuration (primarily for iOS)

## Time Expectations (CRITICAL - NEVER CANCEL)

### First-time setup:
- Git submodules: ~1 minute 8 seconds
- Linux dependencies: ~1 minute 4 seconds  
- CMake configure: ~5-8 seconds

### Build times (SET APPROPRIATE TIMEOUTS):
- Core library: ~15 minutes first build, ~7 minutes with ccache
- Test targets: ~7 minutes
- Utility targets: ~1 minute
- Node.js bindings: varies by platform

### Test execution times:
- C++ unit tests: 4 minutes 44 seconds (995 tests)
- Render tests: 2 minutes 46 seconds (1243 tests)
- Expression tests: 36 milliseconds (322 tests)

## Platform Support Matrix

| Platform | OpenGL | Metal | Vulkan | Build System |
|----------|--------|--------|--------|--------------|
| Linux    | ✅     | ❌     | ✅     | CMake        |
| macOS    | ❌     | ✅     | ✅ *   | CMake        |
| iOS      | ❌     | ✅     | ❌     | Bazel/CMake  |
| Android  | ✅     | ❌     | ✅     | Gradle       |
| Windows  | ✅     | ❌     | ✅     | CMake        |
| Node.js  | ✅     | ❌     | ✅ **  | CMake        |

*Requires MoltenVK. Only available when built via CMake.  
**Issue reported, see maplibre/maplibre-native#2928.

## Known Issues and Workarounds

### Build Issues
- **ccache not found**: Install with `sudo apt-get install -y ccache`
- **Benchmark tests fail**: Requires proper OpenGL context (expected in headless environments)
- **Bazel network issues**: Expected in offline environments
- **HarfBuzz warnings**: Cosmetic, can be ignored

### Test Issues  
- **Some render tests fail**: Expected - 7 errored tests are baseline
- **18 disabled unit tests**: Expected - these are platform-specific or require special setup

## Documentation References

- [MapLibre Native Developer Documentation](https://maplibre.org/maplibre-native/docs/book/)
- [Android Developer Guide](https://maplibre.org/maplibre-native/docs/book/platforms/android/index.html)  
- [iOS Developer Guide](https://maplibre.org/maplibre-native/docs/book/platforms/ios/index.html)
- [Linux Guide](https://maplibre.org/maplibre-native/docs/book/platforms/linux/index.html)
- [Render Tests Documentation](render-tests.md)

## Renderer Selection

The project supports multiple rendering backends:
- **OpenGL**: Mature, widely supported, default for Linux/Android
- **Metal**: Apple platforms only, high performance  
- **Vulkan**: Modern, cross-platform, experimental on some platforms

Choose renderer via CMake presets or Bazel flags:
```bash
# CMake examples
cmake --preset linux-opengl   # OpenGL
cmake --preset linux-vulkan   # Vulkan

# Bazel examples  
bazel run //platform/ios:App --//:renderer=metal
```

**When in doubt, use OpenGL renderer for maximum compatibility.**