#include <mln/gfx/headless_backend.hpp>

namespace mln {
namespace gfx {

bool Backend::enableGPUExpressionEval = false;

HeadlessBackend::HeadlessBackend(Size size_)
    : mln::gfx::Renderable(size_, nullptr) {}

void HeadlessBackend::setSize(Size size_) {
    size = size_;
    resource.reset();
}

} // namespace gfx
} // namespace mln
