#pragma once

#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/util/size.hpp>

#include <cstdint>
#include <string>

namespace mbgl {
namespace ohos {

class WindowBackend {
public:
    virtual ~WindowBackend() = default;

    virtual gfx::RendererBackend& getRendererBackend() = 0;
    virtual void setSize(Size) = 0;

    virtual std::int32_t getGlesContextClientVersion() const { return 0; }
    virtual const std::string& getRendererDiagnostic() const { return emptyDiagnostic(); }

private:
    static const std::string& emptyDiagnostic() {
        static const std::string empty;
        return empty;
    }
};

} // namespace ohos
} // namespace mbgl
