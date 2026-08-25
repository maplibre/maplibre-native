#pragma once

#include <mln/gfx/renderable.hpp>
#include <mln/gfx/backend.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/util/image.hpp>

#include <memory>

namespace mln {
namespace gfx {

// Common headless backend interface, provides HeadlessBackend backend factory
// and enables extending gfx::Renderable with platform specific implementation
// of readStillImage.
class HeadlessBackend : public gfx::Renderable {
public:
    // Factory.
#if MLN_WEBGPU_EMDAWN
    static std::unique_ptr<HeadlessBackend> Create(const Size size = {256, 256},
                                                   SwapBehaviour swapBehavior = SwapBehaviour::NoFlush,
                                                   const gfx::ContextMode contextMode = gfx::ContextMode::Unique) =
        delete;
#else
    static std::unique_ptr<HeadlessBackend> Create(const Size size = {256, 256},
                                                   SwapBehaviour swapBehavior = SwapBehaviour::NoFlush,
                                                   const gfx::ContextMode contextMode = gfx::ContextMode::Unique) {
        return Backend::Create<HeadlessBackend, Size, SwapBehaviour, gfx::ContextMode>(size, swapBehavior, contextMode);
    }
#endif

    virtual PremultipliedImage readStillImage() = 0;
    virtual RendererBackend* getRendererBackend() = 0;
    void setSize(Size);

protected:
    HeadlessBackend(Size);
};

} // namespace gfx
} // namespace mln
