#pragma once

#include <mbgl/gfx/draw_scope.hpp>

namespace mbgl {
namespace webgpu {

class Context;

class DrawScopeResource : public gfx::DrawScopeResource {
public:
    explicit DrawScopeResource(Context& context);
    ~DrawScopeResource() override = default;

private:
    Context& context;
};

} // namespace webgpu
} // namespace mbgl
