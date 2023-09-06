#include <mbgl/gfx/uniform_block.hpp>

namespace mbgl {
namespace gfx {

std::unique_ptr<UniformBlock> UniformBlockArray::nullref = nullptr;

UniformBlockArray::UniformBlockArray(int initCapacity)
    : uniformBlockMap(initCapacity) {}

UniformBlockArray::UniformBlockArray(UniformBlockArray&& other)
    : uniformBlockMap(std::move(other.uniformBlockMap)) {}

UniformBlockArray& UniformBlockArray::operator=(UniformBlockArray&& other) {
    uniformBlockMap = std::move(other.uniformBlockMap);
    return *this;
}

UniformBlockArray& UniformBlockArray::operator=(const UniformBlockArray& other) {
    uniformBlockMap.clear();
    for (const auto& kv : other.uniformBlockMap) {
        add(kv.first, copy(*kv.second));
    }
    return *this;
}

const std::unique_ptr<UniformBlock>& UniformBlockArray::get(const StringIdentity id) const {
    const auto result = uniformBlockMap.find(id);
    return (result != uniformBlockMap.end()) ? result->second : nullref;
}

const std::unique_ptr<UniformBlock>& UniformBlockArray::add(const StringIdentity id, int index, std::size_t size) {
    const auto result = uniformBlockMap.insert(std::make_pair(id, std::unique_ptr<UniformBlock>()));
    if (result.second) {
        result.first->second = create(index, size);
        return result.first->second;
    } else {
        return nullref;
    }
}

} // namespace gfx
} // namespace mbgl
