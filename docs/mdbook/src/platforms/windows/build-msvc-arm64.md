# Build Windows ARM64 with MSCV for MapLibre Native

This guide provides step-by-step instructions for building MapLibre Native on Windows ARM64 systems.

## Prerequisites

1. **Visual Studio 2022** with ARM64 build tools
   - Install "Desktop development with C++" workload
   - Include "MSVC v143 - VS 2022 C++ ARM64 build tools"

2. **CMake** (3.20 or higher)
   - Download from https://cmake.org/download/

3. **Git**
   - Download from https://git-scm.com/download/win

4. **Python** (for build scripts)
   - Download from https://www.python.org/downloads/

## Step 1: Clone the Repository

```bash
git clone https://github.com/maplibre/maplibre-native.git
cd maplibre-native
git submodule update --init --recursive
```

## Step 2: Bootstrap vcpkg and Install Dependencies

Open a **VS2022 PowerShell for ARM64** and run:

```powershell
# Navigate to your maplibre-native directory
cd maplibre-native

# Bootstrap vcpkg if needed
if (!(Test-Path platform\windows\vendor\vcpkg\vcpkg.exe)) {
    cd platform\windows\vendor\vcpkg
    .\bootstrap-vcpkg.bat -disableMetrics
    cd ..\..\..\..
}

# Install required dependencies for ARM64
.\platform\windows\vendor\vcpkg\vcpkg.exe install curl:arm64-windows
.\platform\windows\vendor\vcpkg\vcpkg.exe install libpng:arm64-windows
.\platform\windows\vendor\vcpkg\vcpkg.exe install libjpeg-turbo:arm64-windows
.\platform\windows\vendor\vcpkg\vcpkg.exe install libwebp:arm64-windows
.\platform\windows\vendor\vcpkg\vcpkg.exe install libuv:arm64-windows
.\platform\windows\vendor\vcpkg\vcpkg.exe install icu:arm64-windows
.\platform\windows\vendor\vcpkg\vcpkg.exe install glfw3:arm64-windows
.\platform\windows\vendor\vcpkg\vcpkg.exe install dlfcn-win32:arm64-windows
.\platform\windows\vendor\vcpkg\vcpkg.exe install zlib:arm64-windows
.\platform\windows\vendor\vcpkg\vcpkg.exe install egl-registry:arm64-windows
.\platform\windows\vendor\vcpkg\vcpkg.exe install opengl-registry:arm64-windows

# Note: If vcpkg fails with compiler detection errors, see troubleshooting section
```

### Step 2.1: Copy OpenGL Headers (Required)

The OpenGL headers are platform-independent and need to be copied from x86:

```powershell
# Copy GLES3 headers from x86 to ARM64 (if they exist)
if (Test-Path platform\windows\vendor\vcpkg\installed\x86-windows\include\GLES3) {
    Copy-Item -Path platform\windows\vendor\vcpkg\installed\x86-windows\include\GLES3 `
              -Destination platform\windows\vendor\vcpkg\installed\arm64-windows\include\ `
              -Recurse -Force
}
```

## Step 3: Configure and Build

You can build MapLibre Native with either OpenGL or Vulkan renderer. The Vulkan renderer provides significantly better performance on ARM64 devices.

### Option A: Build with OpenGL (Default)

```powershell
# Clean any previous CMake artifacts:
powershell -Command "Remove-Item -Path CMakeCache.txt, CMakeFiles, DartConfiguration.tcl, build-windows-arm64-opengl -Force -Recurse -ErrorAction SilentlyContinue"

# Configure the project with explicit build directory
# IMPORTANT: You must explicitly set CMAKE_PREFIX_PATH for ARM64 builds
cmake -S . -B build-windows-arm64-opengl --preset windows-arm64-opengl -DCMAKE_PREFIX_PATH="C:/Users/$env:USERNAME/source/repos/maplibre-native-cleanup/platform/windows/vendor/vcpkg/installed/arm64-windows"

# Build the core library
cmake --build build-windows-arm64-opengl --config Release --target mbgl-core

# Build the render executable
cmake --build build-windows-arm64-opengl --config Release --target mbgl-render

# Build the test runner
cmake --build build-windows-arm64-opengl --config Release --target mbgl-test-runner

# Build the GLFW example (if you want to test with a GUI)
cmake --build build-windows-arm64-opengl --config Release --target mbgl-glfw
```

### Option B: Build with Vulkan (Better Performance)

```powershell
# Clean any previous CMake artifacts:
powershell -Command "Remove-Item -Path CMakeCache.txt, CMakeFiles, DartConfiguration.tcl, build-windows-arm64-vulkan -Force -Recurse -ErrorAction SilentlyContinue"

# Configure the project with Vulkan preset
# IMPORTANT: You must explicitly set CMAKE_PREFIX_PATH for ARM64 builds
cmake -S . -B build-windows-arm64-vulkan --preset windows-arm64-vulkan -DCMAKE_PREFIX_PATH="C:/Users/$env:USERNAME/source/repos/maplibre-native-cleanup/platform/windows/vendor/vcpkg/installed/arm64-windows"

# Build the core library
cmake --build build-windows-arm64-vulkan --config Release --target mbgl-core

# Build the render executable
cmake --build build-windows-arm64-vulkan --config Release --target mbgl-render

# Build the test runner
cmake --build build-windows-arm64-vulkan --config Release --target mbgl-test-runner

# Build the GLFW example (if you want to test with a GUI)
cmake --build build-windows-arm64-vulkan --config Release --target mbgl-glfw
```

## Step 4: Copy Required DLLs

The executables need access to vcpkg DLLs at runtime. Copy them to the output directories:

### For OpenGL Build:
```powershell
# Copy DLLs for mbgl-render
Copy-Item -Path platform\windows\vendor\vcpkg\installed\arm64-windows\bin\*.dll `
          -Destination build-windows-arm64-opengl\bin\Release\ -Force

# Copy DLLs for mbgl-test-runner
Copy-Item -Path platform\windows\vendor\vcpkg\installed\arm64-windows\bin\*.dll `
          -Destination build-windows-arm64-opengl\Release\ -Force

# Copy DLLs for mbgl-glfw
Copy-Item -Path platform\windows\vendor\vcpkg\installed\arm64-windows\bin\*.dll `
          -Destination build-windows-arm64-opengl\platform\glfw\Release\ -Force
```

### For Vulkan Build:
```powershell
# Copy DLLs for mbgl-render
Copy-Item -Path platform\windows\vendor\vcpkg\installed\arm64-windows\bin\*.dll `
          -Destination build-windows-arm64-vulkan\bin\Release\ -Force

# Copy DLLs for mbgl-test-runner
Copy-Item -Path platform\windows\vendor\vcpkg\installed\arm64-windows\bin\*.dll `
          -Destination build-windows-arm64-vulkan\Release\ -Force

# Copy DLLs for mbgl-glfw
Copy-Item -Path platform\windows\vendor\vcpkg\installed\arm64-windows\bin\*.dll `
          -Destination build-windows-arm64-vulkan\platform\glfw\Release\ -Force
```

## Step 5: Test the Build

Test commands are the same for both OpenGL and Vulkan builds, just adjust the directory path accordingly.

### Test mbgl-render
```powershell
# For OpenGL:
cd build-windows-arm64-opengl\bin\Release
.\mbgl-render.exe --style https://raw.githubusercontent.com/maplibre/demotiles/gh-pages/style.json --output test.png

# For Vulkan:
cd build-windows-arm64-vulkan\bin\Release
.\mbgl-render.exe --style https://raw.githubusercontent.com/maplibre/demotiles/gh-pages/style.json --output test.png
```

### Run Tests
```powershell
# For OpenGL:
cd build-windows-arm64-opengl\Release
.\mbgl-test-runner.exe

# For Vulkan:
cd build-windows-arm64-vulkan\Release
.\mbgl-test-runner.exe
```

### Run GLFW Example
```powershell
# For OpenGL:
cd build-windows-arm64-opengl\platform\glfw\Release
.\mbgl-glfw.exe --style https://raw.githubusercontent.com/maplibre/demotiles/gh-pages/style.json

# For Vulkan:
cd build-windows-arm64-vulkan\platform\glfw\Release
.\mbgl-glfw.exe --style https://raw.githubusercontent.com/maplibre/demotiles/gh-pages/style.json
```


## Build Outputs

After a successful build, you'll find:
- `mbgl-core.lib` - The core MapLibre library (~107 MB)
- `mbgl-render.exe` - Command-line renderer
- `mbgl-test-runner.exe` - Test suite executable
- `mbgl-glfw.exe` - Interactive map viewer using GLFW

## Troubleshooting

### vcpkg warnings about mismatched VCPKG_ROOT
This warning can be safely ignored, or you can unset the VCPKG_ROOT environment variable:
```powershell
$env:VCPKG_ROOT = ""
```

### "In-source builds are not permitted" error
This occurs if CMake files were created in the wrong location. Clean up and try again:
```powershell
# From the root directory
Remove-Item -Recurse -Force CMakeCache.txt -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force CMakeFiles -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force vendor\freetype\CMakeCache.txt -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force vendor\freetype\CMakeFiles -ErrorAction SilentlyContinue

# Then reconfigure with explicit directories
cmake -S . -B build-windows-arm64-opengl --preset windows-arm64-opengl
```

### vcpkg fails with "No CMAKE_C_COMPILER could be found"
This is a known vcpkg bug with ARM64. The custom triplet file in `platform/windows/vendor/vcpkg-custom-triplets/arm64-windows.cmake` forces the Visual Studio generator to work around this issue. If you still encounter problems:
1. Ensure you're using the custom triplet by setting `VCPKG_OVERLAY_TRIPLETS`
2. Try using x64 Native Tools Command Prompt instead of ARM64
3. Copy already-built packages from another installation

### CMake Error: Could NOT find CURL (or other packages)
This happens when vcpkg packages are installed but CMake can't find them. Solution:
- Always pass `-DCMAKE_PREFIX_PATH` explicitly when configuring for ARM64
- Use the full path to your vcpkg ARM64 installation directory
- Example: `-DCMAKE_PREFIX_PATH="C:/path/to/maplibre-native/platform/windows/vendor/vcpkg/installed/arm64-windows"`

### Missing GLES3/gl3.h header
This header is platform-independent. If not installed by vcpkg:
```powershell
# First, install opengl-registry for x86 if not present
.\platform\windows\vendor\vcpkg\vcpkg.exe install opengl-registry:x86-windows

# Then copy the headers
Copy-Item -Path platform\windows\vendor\vcpkg\installed\x86-windows\include\GLES3 `
          -Destination platform\windows\vendor\vcpkg\installed\arm64-windows\include\ `
          -Recurse -Force
```

### CMake Error: Target "mbgl-core" links to ICU::data but the target was not found
This is automatically handled by the ARM64 fixes integrated into `platform/windows/windows.cmake`.

### Build times out in command line
The mbgl-core build can take 10-15 minutes on ARM64. If using a CI/CD system, increase timeout limits.

### Executable won't start or exits immediately
This usually means missing DLL dependencies. Make sure you've copied the vcpkg DLLs:
```powershell
# Check what DLLs are required
dumpbin /dependents path\to\your.exe

# Copy all ARM64 DLLs to the executable's directory
Copy-Item -Path platform\windows\vendor\vcpkg\installed\arm64-windows\bin\*.dll `
          -Destination path\to\executable\directory\ -Force
```

## Known Issues

### OpenGL Build
- 4 tests fail out of 1001 (TileLOD-related)
- These failures don't affect normal usage

### Vulkan Build
- Only 1 test fails out of 966 (Map.SetStyleInvalidJSON)
- "Vulkan layers not found" warnings can be ignored (validation layers)
