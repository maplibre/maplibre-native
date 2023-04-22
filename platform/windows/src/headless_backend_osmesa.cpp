#include <mbgl/util/logging.hpp>

#include <cassert>

#include <mbgl/gl/headless_backend.hpp>

#include <GL/osmesa.h>

namespace mbgl {
namespace gl {

// This class provides a singleton that contains information about the configuration used for
// instantiating new headless rendering contexts.

class OSMesaBackendImpl final : public HeadlessBackend::Impl {
public:
    OSMesaBackendImpl() {
        context = OSMesaCreateContextAttribs(std::initializer_list<int>({
            OSMESA_FORMAT, OSMESA_RGBA,
            OSMESA_DEPTH_BITS, 24,
            OSMESA_STENCIL_BITS, 8,
            OSMESA_PROFILE, OSMESA_COMPAT_PROFILE,
            NULL
        }).begin(), NULL);
    }

    ~OSMesaBackendImpl() final {
        OSMesaDestroyContext(context);
    }

    gl::ProcAddress getExtensionFunctionPointer(const char* name) final {
        return (ProcAddress)::OSMesaGetProcAddress(name);
    }

    void activateContext() final {
        OSMesaMakeCurrent(context, buffer.get(), GL_UNSIGNED_BYTE, 2048, 2048);
    }

    void deactivateContext() final {
    }

private:
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(2048 * 2048 * 4);
    OSMesaContext context = NULL;
};

void HeadlessBackend::createImpl() {
    assert(!impl);
    impl = std::make_unique<OSMesaBackendImpl>();
}

} // namespace gl
} // namespace mbgl
