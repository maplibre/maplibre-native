#pragma once

#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/mtl/texture2d.hpp>
#include <mbgl/gfx/context.hpp>

#import <Cocoa/Cocoa.h>

class MetalBackend final : public mln::mtl::RendererBackend, public mln::gfx::Renderable {
public:
    MetalBackend(NSWindow *window);

    mln::gfx::Renderable &getDefaultRenderable() override;
    void activate() override;
    void deactivate() override;
    void updateAssumedState() override;
    void setSize(mln::Size size_);
    mln::Size getSize() const;
};
