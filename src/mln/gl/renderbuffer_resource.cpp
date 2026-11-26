
#include <mln/gl/renderbuffer_resource.hpp>
#include <mln/util/instrumentation.hpp>

namespace mln {
namespace gl {

RenderbufferResource::~RenderbufferResource() noexcept {
    MLN_TRACE_FREE_RT(renderbuffer.get());
}

} // namespace gl
} // namespace mln
