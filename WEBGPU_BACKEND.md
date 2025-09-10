# WebGPU Backend for MapLibre Native

This document describes the WebGPU backend implementation for MapLibre Native.

## Overview

The WebGPU backend provides a modern, cross-platform graphics API implementation that can work with either Dawn (Google's implementation) or wgpu (Mozilla's Rust-based implementation). The backend is designed to be flexible and not tied strongly to either implementation.

## Architecture

The WebGPU backend follows the same architecture as the existing OpenGL, Metal, and Vulkan backends:

- **Backend Type**: Added `WebGPU` to the `gfx::Backend::Type` enum
- **Context**: `webgpu::Context` implements the `gfx::Context` interface
- **Renderer Backend**: `webgpu::RendererBackend` implements the `gfx::RendererBackend` interface
- **Abstraction Layer**: `webgpu::BackendImpl` provides an abstraction over Dawn/wgpu differences

## Building with WebGPU

### Enable WebGPU Backend

```bash
cmake -DMLN_WITH_WEBGPU=ON ..
```

### Choose Implementation (Dawn or wgpu)

```bash
# Use Dawn (default)
cmake -DMLN_WITH_WEBGPU=ON -DMLN_WEBGPU_BACKEND=dawn ..

# Use wgpu
cmake -DMLN_WITH_WEBGPU=ON -DMLN_WEBGPU_BACKEND=wgpu ..
```

## Dependencies

### Option 1: Dawn

Dawn can be integrated as a git submodule or as a prebuilt library.

```bash
# Add Dawn as a submodule
git submodule add https://dawn.googlesource.com/dawn vendor/dawn
```

### Option 2: wgpu

wgpu-native provides prebuilt binaries or can be built from source.

```bash
# Download prebuilt wgpu-native
wget https://github.com/gfx-rs/wgpu-native/releases/download/v0.19.0/wgpu-linux-x86_64-release.zip
unzip wgpu-linux-x86_64-release.zip -d vendor/wgpu-native
```

### Option 3: WebGPU Headers Only

For development without a specific backend:

```bash
git submodule add https://github.com/webgpu-native/webgpu-headers vendor/webgpu-headers
```

## Implementation Status

### Completed
- ✅ Backend type enum addition
- ✅ Basic context and renderer backend structure
- ✅ CMake build configuration
- ✅ Abstraction layer for Dawn/wgpu flexibility

### TODO
- [ ] Implement shader compilation (WGSL)
- [ ] Implement drawable and drawable builder
- [ ] Implement texture and buffer management
- [ ] Implement render pass and command encoding
- [ ] Implement uniform buffer management
- [ ] Port shaders to WGSL
- [ ] Implement layer groups and tile layer groups
- [ ] Platform-specific surface creation (GLFW, Android, iOS, etc.)
- [ ] Performance optimizations
- [ ] Testing and validation

## File Structure

```
include/mbgl/webgpu/
├── backend_impl.hpp      # Abstraction layer interface
├── buffer_resource.hpp   # Buffer resource definition
├── context.hpp          # WebGPU context implementation
└── renderer_backend.hpp # WebGPU renderer backend

src/mbgl/webgpu/
├── backend_impl.cpp     # Abstraction layer implementation
├── context.cpp         # Context implementation
└── renderer_backend.cpp # Renderer backend implementation

vendor/
├── webgpu.cmake        # WebGPU vendor configuration
├── dawn/              # Dawn submodule (optional)
├── wgpu-native/       # wgpu prebuilt binaries (optional)
└── webgpu-headers/    # WebGPU headers (optional)
```

## Platform Support

WebGPU backend can theoretically support all platforms that Dawn or wgpu support:
- Windows (D3D12, Vulkan)
- macOS (Metal)
- Linux (Vulkan)
- Android (Vulkan)
- iOS (Metal)
- Web (WebGPU in browsers via Emscripten)

## Development Notes

1. The backend uses the C API of WebGPU which is common between Dawn and wgpu
2. Platform-specific code is minimized and isolated in the renderer backend
3. The implementation follows MapLibre's existing patterns for graphics backends
4. Shader compilation will require WGSL (WebGPU Shading Language) versions of all shaders

## Contributing

To contribute to the WebGPU backend:

1. Ensure you have either Dawn or wgpu set up in your development environment
2. Build with `-DMLN_WITH_WEBGPU=ON`
3. Run tests with the WebGPU backend enabled
4. Submit PRs with clear descriptions of changes

## References

- [WebGPU Specification](https://www.w3.org/TR/webgpu/)
- [Dawn Project](https://dawn.googlesource.com/dawn)
- [wgpu-native](https://github.com/gfx-rs/wgpu-native)
- [WebGPU Headers](https://github.com/webgpu-native/webgpu-headers)