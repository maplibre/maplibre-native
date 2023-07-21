#include <mbgl/mtl/uniform_block.hpp>

#include <mbgl/mtl/uniform_buffer.hpp>

#include <cassert>

namespace mbgl {
namespace mtl {

void UniformBlock::bindBuffer(const gfx::UniformBuffer& uniformBuffer) {
    assert(size == uniformBuffer.getSize());
}

void UniformBlock::unbindBuffer() {
}

} // namespace mtl
} // namespace mbgl
