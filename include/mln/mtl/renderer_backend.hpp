#pragma once

#include <mln/gfx/renderer_backend.hpp>
#include <mln/mtl/mtl_fwd.hpp>
#include <mln/util/image.hpp>
#include <mln/util/size.hpp>
#include <mln/util/util.hpp>

#include <Foundation/NSSharedPtr.hpp>
#include <Metal/MTLDevice.hpp>
#include <Metal/MTLCommandQueue.hpp>

namespace mln {

class ProgramParameters;

namespace mtl {

using ProcAddress = void (*)();

class RendererBackend : public gfx::RendererBackend {
public:
    RendererBackend(gfx::ContextMode);
    ~RendererBackend() override;

    /// One-time shader initialization
    void initShaders(gfx::ShaderRegistry&, const ProgramParameters& programParameters) override;

    const MTLDevicePtr& getDevice() const { return device; }
    const MTLCommandQueuePtr& getCommandQueue() const { return commandQueue; }
    bool isBaseVertexInstanceDrawingSupported() const { return baseVertexInstanceDrawingSupported; }

protected:
    std::unique_ptr<gfx::Context> createContext() override;

    /// Reads the color pixel data from the currently bound framebuffer.
    PremultipliedImage readFramebuffer(const Size&);

    MTLDevicePtr device;
    MTLCommandQueuePtr commandQueue;
    bool baseVertexInstanceDrawingSupported = false;
};

} // namespace mtl
} // namespace mln
