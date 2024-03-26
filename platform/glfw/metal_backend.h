#pragma once

#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/gfx/renderable.hpp>

#import <Cocoa/Cocoa.h>

class MetalBackend final : public mbgl::mtl::RendererBackend, public mbgl::gfx::Renderable {
public:
    MetalBackend(NSWindow *window);

    mbgl::gfx::Renderable &getDefaultRenderable() override;
    void activate() override;
    void deactivate() override;
    void updateAssumedState() override;
};
