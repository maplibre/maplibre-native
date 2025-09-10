#pragma once

#include <array>
#include <cstdint>

namespace mbgl {
namespace shaders {
namespace webgpu {

// Common constants used across WebGPU shaders
constexpr uint32_t kMaxAttributes = 16;
constexpr uint32_t kMaxTextures = 8;
constexpr uint32_t kMaxUniformBlocks = 4;

// Binding indices
constexpr uint32_t kGlobalUniformBlockBinding = 0;
constexpr uint32_t kLayerUniformBlockBinding = 1;
constexpr uint32_t kDrawableUniformBlockBinding = 2;

// Texture binding start index
constexpr uint32_t kTextureBindingStart = 4;

// Vertex buffer binding indices
constexpr uint32_t kVertexBufferBinding = 0;
constexpr uint32_t kInstanceBufferBinding = 1;

// Common utility functions
inline uint32_t getTextureBinding(uint32_t textureIndex) {
    return kTextureBindingStart + textureIndex;
}

} // namespace webgpu
} // namespace shaders
} // namespace mbgl