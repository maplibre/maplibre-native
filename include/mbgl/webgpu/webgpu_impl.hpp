#pragma once

// Forward declarations for WebGPU implementation classes

namespace mbgl {
namespace webgpu {

class Context;
class CommandEncoder;
class OffscreenTexture;
class VertexAttributeArray;
class Texture2D;
class RenderTarget;
class TileLayerGroup;
class LayerGroup;
class UniformBufferArray;
class RenderbufferResource;
class DrawScopeResource;

} // namespace webgpu
} // namespace mbgl

// Include headers to ensure they exist
#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/offscreen_texture.hpp>
#include <mbgl/webgpu/renderbuffer.hpp>
#include <mbgl/webgpu/vertex_attribute_array.hpp>
#include <mbgl/webgpu/texture2d.hpp>
#include <mbgl/webgpu/render_target.hpp>
#include <mbgl/webgpu/tile_layer_group.hpp>
#include <mbgl/webgpu/layer_group.hpp>
#include <mbgl/webgpu/uniform_buffer_array.hpp>