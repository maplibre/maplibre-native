#include <mbgl/gfx/uniform_block.hpp>

namespace mbgl {
namespace gfx {

std::unique_ptr<UniformBlock> UniformBlockArray::nullref = nullptr;

UniformBlockArray::UniformBlockArray(UniformBlockArray&& other)
    : uniformBlockVector(std::move(other.uniformBlockVector)) {}

UniformBlockArray& UniformBlockArray::operator=(UniformBlockArray&& other) {
    uniformBlockVector = std::move(other.uniformBlockVector);
    return *this;
}

UniformBlockArray& UniformBlockArray::operator=(const UniformBlockArray& other) {
    for (size_t id = 0; id < other.uniformBlockVector.size(); id++) {
        uniformBlockVector[id] = copy(*other.uniformBlockVector[id]);
    }
    return *this;
}

const std::unique_ptr<UniformBlock>& UniformBlockArray::get(const size_t id) const {
    const auto& result = (id < uniformBlockVector.size()) ? uniformBlockVector[id] : nullref;
    return (result != nullptr) ? result : nullref;
}

const std::unique_ptr<UniformBlock>& UniformBlockArray::add(const size_t id, const size_t index, std::size_t size) {
    uniformBlockVector[id] = create((int)index, size);
    return uniformBlockVector[id];
}

} // namespace gfx
} // namespace mbgl
