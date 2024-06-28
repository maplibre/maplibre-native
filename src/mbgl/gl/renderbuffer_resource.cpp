
#include <mbgl/gl/renderbuffer_resource.hpp>
#include <mbgl/util/instrumentation.hpp>

namespace mbgl {
namespace gl {

RenderbufferResource::~RenderbufferResource() noexcept {
    MLN_TRACE_FREE_RT(renderbuffer.get());
}

} // namespace gl
} // namespace mbgl
