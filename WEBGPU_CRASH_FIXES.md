# WebGPU Crash Fixes - Thread Safety and Memory Management

## Issues Identified and Fixed

### 1. **Use-After-Free in Pipeline Management**
**Files**: `src/mbgl/webgpu/drawable.cpp`, `src/mbgl/shaders/webgpu/shader_program.cpp`

**Problem**: 
- Drawable was storing a raw pointer to the pipeline owned by ShaderProgram
- When ShaderProgram was destroyed, the pipeline was released but Drawable still held the dangling pointer
- Drawable destructor was trying to release a pipeline it didn't own

**Solution**:
- Drawable no longer releases the pipeline in its destructor (it doesn't own it)
- Added proper null checks before accessing pipeline
- Removed unsafe pointer validation that could itself cause crashes

### 2. **Race Conditions in Buffer Management**
**File**: `src/mbgl/webgpu/drawable.cpp`

**Problem**:
- Buffers were being released and nullified in two separate operations
- This created a window where another thread could access a buffer being freed

**Solution**:
- Changed to atomic-like pattern: copy pointer, nullify member, then release
- This prevents use-after-free if another thread accesses the member

### 3. **Missing Bind Group Layout Configuration**
**File**: `src/mbgl/shaders/webgpu/shader_program.cpp`

**Problem**:
- Pipeline was created without proper bind group layout
- Uniforms (MVP matrix) couldn't be bound properly
- Vertex buffer layout was not configured

**Solution**:
- Added proper bind group layout creation for uniform buffer
- Configured vertex buffer layout for int16x2 position data
- Added proper error handling and resource cleanup on failure

### 4. **Missing Error Callbacks and Diagnostics**
**File**: `platform/glfw/glfw_webgpu_backend.mm`

**Problem**:
- No error callbacks were set up for WebGPU device
- Validation errors and device lost events were silent
- Hard to diagnose issues without error messages

**Solution**:
- Added uncaptured error callback to log validation and runtime errors
- Added device lost callback to detect GPU issues
- Added detailed logging for surface texture acquisition failures
- Improved error messages throughout initialization

### 5. **Resource Cleanup Order Issues**
**Files**: `src/mbgl/shaders/webgpu/shader_program.cpp`, `platform/glfw/glfw_webgpu_backend.mm`

**Problem**:
- Resources were not being released in correct dependency order
- Some resources were leaked on error paths

**Solution**:
- Fixed destructor order: pipeline before pipeline layout before shader modules
- Added proper cleanup on all error paths during pipeline creation
- Backend destructor now properly releases resources in reverse order

### 6. **Thread Safety in Shader Registration**
**Note**: The shader registry already has proper mutex protection (std::shared_mutex)

## Testing Recommendations

1. **Stress Testing**:
   - Run with multiple tiles loading simultaneously
   - Rapidly zoom/pan to trigger frequent drawable creation/destruction
   - Monitor for crashes during shader registration phase

2. **Memory Testing**:
   - Run with AddressSanitizer to detect use-after-free
   - Run with ThreadSanitizer to detect race conditions
   - Monitor GPU memory usage for leaks

3. **Error Injection**:
   - Test with invalid shader code to verify error handling
   - Test with limited GPU memory to verify out-of-memory handling
   - Test device lost scenarios (GPU reset)

## Remaining Considerations

1. **Shader Variants**: Current implementation uses a single test shader for all layer types. Production code should generate appropriate shaders for each layer type.

2. **Performance**: Consider implementing:
   - Pipeline caching to avoid recreation
   - Bind group caching for frequently used uniform combinations
   - Buffer suballocation for small vertex/index buffers

3. **Synchronization**: May need additional synchronization for:
   - Multi-threaded tile loading
   - Concurrent drawable creation
   - Resource sharing between frames

## Build and Run

After applying these fixes:
```bash
cmake --build build
./build/bin/mbgl-glfw-app --style https://demotiles.maplibre.org/style.json --backend=webgpu
```

Monitor the console output for any WebGPU errors or warnings that may indicate remaining issues.