#pragma once

#include <mbgl/gfx/renderbuffer.hpp>

namespace mbgl {
namespace webgpu {

class RenderbufferResource : public gfx::RenderbufferResource {
public:
    RenderbufferResource() = default;
    ~RenderbufferResource() override = default;
};

} // namespace webgpu
} // namespace mbgl