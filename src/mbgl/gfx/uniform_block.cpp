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
    for (size_t index = 0; index < other.uniformBlockVector.size(); index++) {
        uniformBlockVector[index] = copy(*other.uniformBlockVector[index]);
    }
    return *this;
}

const std::unique_ptr<UniformBlock>& UniformBlockArray::get(const size_t index) const {
    const auto& result = (index < uniformBlockVector.size()) ? uniformBlockVector[index] : nullref;
    return (result != nullptr) ? result : nullref;
}

const std::unique_ptr<UniformBlock>& UniformBlockArray::add(const size_t index, std::size_t size) {
    uniformBlockVector[index] = create((int)index, size);
    return uniformBlockVector[index];
}

} // namespace gfx
} // namespace mbgl
