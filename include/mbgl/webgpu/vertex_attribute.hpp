#pragma once

#include <mbgl/gfx/vertex_attribute.hpp>

namespace mbgl {
namespace webgpu {

class VertexAttribute : public gfx::VertexAttribute {
public:
    using gfx::VertexAttribute::VertexAttribute;
};

} // namespace webgpu
} // namespace mbgl