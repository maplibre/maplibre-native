#pragma once

#include "window_backend.hpp"

#include <mln/util/size.hpp>
#include <mln/vulkan/renderable_resource.hpp>
#include <mln/vulkan/renderer_backend.hpp>

#include <cstdint>
#include <memory>
#include <string>

namespace mln {
namespace ohos {

class VulkanWindowBackend final : public WindowBackend, public vulkan::RendererBackend, public vulkan::Renderable {
public:
    VulkanWindowBackend(OHNativeWindow*, Size);
    ~VulkanWindowBackend() override;

    VulkanWindowBackend(const VulkanWindowBackend&) = delete;
    VulkanWindowBackend& operator=(const VulkanWindowBackend&) = delete;

    gfx::RendererBackend& getRendererBackend() override { return *this; }
    gfx::Renderable& getDefaultRenderable() override { return *this; }

    OHNativeWindow* getNativeWindow() const { return window; }
    void setSize(Size) override;
    const std::string& getRendererDiagnostic() const override { return rendererDiagnostic; }

protected:
    std::vector<const char*> getInstanceExtensions() override;
    void initInstance() override;
    void activate() override {}
    void deactivate() override {}

private:
    OHNativeWindow* window = nullptr;
    std::string rendererDiagnostic;
};

} // namespace ohos
} // namespace mln
