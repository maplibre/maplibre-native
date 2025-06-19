// Copyright (C) 2024 MapLibre contributors
// SPDX-License-Identifier: BSD-2-Clause

#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_OSX

#include "metal_renderer_backend.hpp"

#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/texture2d.hpp>
#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/context.hpp>

#import <Metal/Metal.hpp>
#import <QuartzCore/CAMetalLayer.hpp>

#include <cassert>
#include <QDebug>

namespace QMapLibre {

using namespace mbgl::mtl;

namespace {
class QtMetalRenderableResource final : public mbgl::mtl::RenderableResource {
public:
    using LayerPtr = CA::MetalLayer*;
    QtMetalRenderableResource(MetalRendererBackend& backend_, LayerPtr layer_)
        : backend(backend_), layer(layer_), commandQueue(NS::TransferPtr(backend.getDevice()->newCommandQueue())) {
        assert(layer);
        layer->setDevice(backend.getDevice().get());
    }

    void setBackendSize(mbgl::Size size_) {
        size = size_;
        layer->setDrawableSize({static_cast<CGFloat>(size.width), static_cast<CGFloat>(size.height)});
        buffersInvalid = true;
    }

    mbgl::Size getSize() const { return size; }

    // — mbgl::mtl::RenderableResource -----------------------------------
    void bind() override {
        // Qt Quick may supply us with the current swap-chain texture via
        // MetalRendererBackend::_q_setCurrentDrawable().  Use that texture if
        // present to avoid contending for a second drawable from the same
        // CAMetalLayer.
        MTL::Texture *externalTex = static_cast<MTL::Texture *>(backend.currentDrawable());

        if (!externalTex) {
            auto *tmpDrawable = layer->nextDrawable();
            if (!tmpDrawable) {
                qWarning() << "MapLibre Metal: nextDrawable() returned nil."
                           << "drawableSize=" << layer->drawableSize().width << "x" << layer->drawableSize().height
                           << ", device=" << (void *)layer->device()
                           << ", pixelFormat=" << static_cast<int>(layer->pixelFormat());

                // Still allocate a command buffer so subsequent frames continue
                commandBuffer = NS::RetainPtr(commandQueue->commandBuffer());
                // Leave renderPassDescriptor null; caller will treat this as
                // an empty frame.
                return;
            }

            surface = NS::RetainPtr(tmpDrawable);
            externalTex = surface->texture();
            backend._q_setCurrentDrawable(externalTex);
        }

        auto texSize = mbgl::Size{ static_cast<uint32_t>(layer->drawableSize().width),
                                   static_cast<uint32_t>(layer->drawableSize().height) };

        commandBuffer = NS::RetainPtr(commandQueue->commandBuffer());
        renderPassDescriptor = NS::RetainPtr(MTL::RenderPassDescriptor::alloc()->init());
        renderPassDescriptor->colorAttachments()->object(0)->setTexture(externalTex);

        if (buffersInvalid || !depthTexture || !stencilTexture) {
            buffersInvalid = false;
            depthTexture = backend.getContext().createTexture2D();
            depthTexture->setSize(texSize);
            depthTexture->setFormat(mbgl::gfx::TexturePixelType::Depth, mbgl::gfx::TextureChannelDataType::Float);
            depthTexture->setSamplerConfiguration({ mbgl::gfx::TextureFilterType::Linear,
                                                    mbgl::gfx::TextureWrapType::Clamp,
                                                    mbgl::gfx::TextureWrapType::Clamp });
            static_cast<Texture2D*>(depthTexture.get())->setUsage(MTL::TextureUsageShaderRead |
                                                                  MTL::TextureUsageShaderWrite |
                                                                  MTL::TextureUsageRenderTarget);

            stencilTexture = backend.getContext().createTexture2D();
            stencilTexture->setSize(texSize);
            stencilTexture->setFormat(mbgl::gfx::TexturePixelType::Stencil, mbgl::gfx::TextureChannelDataType::UnsignedByte);
            stencilTexture->setSamplerConfiguration({ mbgl::gfx::TextureFilterType::Linear,
                                                      mbgl::gfx::TextureWrapType::Clamp,
                                                      mbgl::gfx::TextureWrapType::Clamp });
            static_cast<Texture2D*>(stencilTexture.get())->setUsage(MTL::TextureUsageShaderRead |
                                                                    MTL::TextureUsageShaderWrite |
                                                                    MTL::TextureUsageRenderTarget);
        }

        if (depthTexture) {
            depthTexture->create();
            if (auto* depthTarget = renderPassDescriptor->depthAttachment()) {
                depthTarget->setTexture(static_cast<Texture2D*>(depthTexture.get())->getMetalTexture());
            }
        }
        if (stencilTexture) {
            stencilTexture->create();
            if (auto* stencilTarget = renderPassDescriptor->stencilAttachment()) {
                stencilTarget->setTexture(static_cast<Texture2D*>(stencilTexture.get())->getMetalTexture());
            }
        }
    }

    void swap() override {
        if (surface) {
            commandBuffer->presentDrawable(surface.get());
        }
        commandBuffer->commit();
        commandBuffer.reset();
        renderPassDescriptor.reset();
    }

    const mbgl::mtl::RendererBackend& getBackend() const override { return backend; }

    const MTLCommandBufferPtr& getCommandBuffer() const override { return commandBuffer; }

    MTLBlitPassDescriptorPtr getUploadPassDescriptor() const override { return NS::RetainPtr(MTL::BlitPassDescriptor::alloc()->init()); }

    const MTLRenderPassDescriptorPtr& getRenderPassDescriptor() const override { return renderPassDescriptor; }

private:
    MetalRendererBackend& backend;
    LayerPtr layer;
    MTLCommandQueuePtr commandQueue;
    MTLCommandBufferPtr commandBuffer;
    MTLRenderPassDescriptorPtr renderPassDescriptor;
    CAMetalDrawablePtr surface;
    mbgl::gfx::Texture2DPtr depthTexture;
    mbgl::gfx::Texture2DPtr stencilTexture;
    mbgl::Size size{0, 0};
    bool buffersInvalid = true;
};
} // namespace

MetalRendererBackend::MetalRendererBackend(CA::MetalLayer* layer)
    : mbgl::mtl::RendererBackend(mbgl::gfx::ContextMode::Unique),
      mbgl::gfx::Renderable(mbgl::Size{0, 0}, std::make_unique<QtMetalRenderableResource>(*this, layer)) {
}

// Convenience constructor that allocates its own CAMetalLayer. Used when only a
// ContextMode is available (e.g. MapRenderer's default path).
MetalRendererBackend::MetalRendererBackend(mbgl::gfx::ContextMode)
    : MetalRendererBackend(CA::MetalLayer::layer()) {}

MetalRendererBackend::~MetalRendererBackend() = default;

void MetalRendererBackend::setSize(mbgl::Size size_) {
    this->getResource<QtMetalRenderableResource>().setBackendSize(size_);
}

mbgl::Size MetalRendererBackend::getSize() const {
    return this->getResource<QtMetalRenderableResource>().getSize();
}

} // namespace QMapLibre

#endif // TARGET_OS_OSX
#endif // __APPLE__
