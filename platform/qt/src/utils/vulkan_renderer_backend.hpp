#pragma once

#if defined(MLN_RENDER_BACKEND_VULKAN)

#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/util/size.hpp>

#include <QtGui/QWindow>
#include <QtGui/qvulkaninstance.h>

namespace QMapLibre {

class VulkanRendererBackend;

// Renderable resource that integrates Qt's Vulkan surface with MapLibre Native
class QtVulkanRenderableResource final : public mbgl::vulkan::SurfaceRenderableResource {
public:
    explicit QtVulkanRenderableResource(VulkanRendererBackend& backend_);

    std::vector<const char*> getDeviceExtensions() override;
    void createPlatformSurface() override;
    void bind() override {}
};

class VulkanRendererBackend : public mbgl::vulkan::RendererBackend, public mbgl::vulkan::Renderable {
public:
    explicit VulkanRendererBackend(QWindow* window);
    ~VulkanRendererBackend() override;

    // mbgl::gfx::RendererBackend implementation
    mbgl::gfx::Renderable& getDefaultRenderable() override { return *this; }

    // Size helpers
    mbgl::Size getSize() const { return size; }
    void setSize(const mbgl::Size newSize);

    // Expose the Vk instance to the renderable resource
    vk::Instance const& getInstance() const { return mbgl::vulkan::RendererBackend::getInstance().get(); }
    QWindow* getWindow() const { return window; }
    QVulkanInstance* getQtVulkanInstance() const { return vulkanInstance; }

    // Instance extensions
    std::vector<const char*> getInstanceExtensions() override;

private:
    QWindow* window{nullptr};
    QVulkanInstance* vulkanInstance{nullptr};
};

} // namespace QMapLibre

#endif // MLN_RENDER_BACKEND_VULKAN 