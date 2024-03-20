#include <mbgl/util/logging.hpp>

#include <cassert>

#include <mbgl/gl/headless_backend.hpp>

#include <GL/osmesa.h>

namespace mbgl {
namespace gl {

// This class provides a singleton that contains information about the
// configuration used for instantiating new headless rendering contexts.

class OSMesaBackendImpl final : public HeadlessBackend::Impl {
public:
    OSMesaBackendImpl()
        : buffer(std::make_unique<uint8_t[]>(2048 * 2048 * 4)),
          context(OSMesaCreateContextAttribs(std::initializer_list<int>({OSMESA_FORMAT,
                                                                         OSMESA_RGBA,
                                                                         OSMESA_DEPTH_BITS,
                                                                         24,
                                                                         OSMESA_STENCIL_BITS,
                                                                         8,
                                                                         OSMESA_PROFILE,
                                                                         OSMESA_COMPAT_PROFILE,
                                                                         OSMESA_CONTEXT_MAJOR_VERSION,
                                                                         3,
                                                                         OSMESA_CONTEXT_MINOR_VERSION,
                                                                         0,
                                                                         NULL})
                                                 .begin(),
                                             nullptr)) {}

    ~OSMesaBackendImpl() final { OSMesaDestroyContext(context); }

    gl::ProcAddress getExtensionFunctionPointer(const char* name) final {
        return (ProcAddress)::OSMesaGetProcAddress(name);
    }

    void activateContext() final { OSMesaMakeCurrent(context, buffer.get(), GL_UNSIGNED_BYTE, 2048, 2048); }

    void deactivateContext() final { OSMesaMakeCurrent(nullptr, nullptr, GL_UNSIGNED_BYTE, 0, 0); }

private:
    std::unique_ptr<uint8_t[]> buffer;
    OSMesaContext context;
};

void HeadlessBackend::createImpl() {
    assert(!impl);
    impl = std::make_unique<OSMesaBackendImpl>();
}

} // namespace gl
} // namespace mbgl
