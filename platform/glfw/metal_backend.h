#pragma once

#include <mln/mtl/renderer_backend.hpp>
#include <mln/gfx/renderable.hpp>
#include <mln/mtl/texture2d.hpp>
#include <mln/gfx/context.hpp>

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
