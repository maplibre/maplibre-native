#include "metal_backend.h"

#include <mbgl/mtl/renderable_resource.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.hpp>

class MetalRenderableResource final: public mbgl::mtl::RenderableResource {
public:
    MetalRenderableResource(MetalBackend &backend):
        rendererBackend(backend),
        commandQueue(NS::TransferPtr(backend.getDevice()->newCommandQueue())),
        swapchain(NS::TransferPtr(CA::MetalLayer::layer()))
    {
        swapchain->setDevice(backend.getDevice().get());
    }
    
    void bind() override {
        auto surface = swapchain->nextDrawable();
        
        commandBuffer = NS::RetainPtr(commandQueue->commandBuffer());
        renderPassDescriptor = NS::TransferPtr(MTL::RenderPassDescriptor::renderPassDescriptor());
        renderPassDescriptor->colorAttachments()->object(0)->setTexture(surface->texture());
    }
    
    const mbgl::mtl::RendererBackend& getBackend() const override {
        return rendererBackend;
    }
    
    const mbgl::mtl::MTLCommandBufferPtr& getCommandBuffer() const override {
        return commandBuffer;
    }
    
    mbgl::mtl::MTLBlitPassDescriptorPtr getUploadPassDescriptor() const override {
        return NS::TransferPtr(MTL::BlitPassDescriptor::alloc()->init());
    }
    
    const mbgl::mtl::MTLRenderPassDescriptorPtr& getRenderPassDescriptor() const override {
        return renderPassDescriptor;
    }
    
    NS::SharedPtr<CA::MetalLayer> getSwapchain() {
        return swapchain;
    }
    
private:
    MetalBackend &rendererBackend;
    NS::SharedPtr<MTL::CommandQueue> commandQueue;
    NS::SharedPtr<MTL::CommandBuffer> commandBuffer;
    NS::SharedPtr<MTL::RenderPassDescriptor> renderPassDescriptor;
    
    NS::SharedPtr<CA::MetalLayer> swapchain;
};

MetalBackend::MetalBackend(NSWindow *window):
  mbgl::mtl::RendererBackend(mbgl::gfx::ContextMode::Unique),
  mbgl::gfx::Renderable(mbgl::Size{ 0, 0 }, std::make_unique<MetalRenderableResource>(*this)) 
{
    window.contentView.layer = (__bridge CALayer *)getDefaultRenderable().getResource<MetalRenderableResource>().getSwapchain().get();
    window.contentView.wantsLayer = YES;
}

mbgl::gfx::Renderable &MetalBackend::getDefaultRenderable() {
    return *this;
}

void MetalBackend::activate() {}
void MetalBackend::deactivate() {}
void MetalBackend::updateAssumedState() {}
