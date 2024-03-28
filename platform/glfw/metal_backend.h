#pragma once

#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/mtl/texture2d.hpp>
#include <mbgl/gfx/context.hpp>

#import <Cocoa/Cocoa.h>

class MetalBackend final : public mbgl::mtl::RendererBackend, public mbgl::gfx::Renderable {
public:
    MetalBackend(NSWindow *window);

    mbgl::gfx::Renderable &getDefaultRenderable() override;
    void activate() override;
    void deactivate() override;
    void updateAssumedState() override;
    void setSize(mbgl::Size size_);
    mbgl::Size getSize() const;
};
