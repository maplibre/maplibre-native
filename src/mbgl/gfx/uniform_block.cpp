#include <mbgl/gfx/uniform_block.hpp>

namespace mbgl {
namespace gfx {

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

UniformBlock* UniformBlockArray::get(const std::string& name) const {
    const auto result = uniformBlockMap.find(name);
    return (result != uniformBlockMap.end()) ? result->second.get() : nullptr;
}

UniformBlock* UniformBlockArray::add(std::string name, int index, std::size_t size) {
    const auto result = uniformBlockMap.insert(std::make_pair(std::move(name), std::unique_ptr<UniformBlock>()));
    if (result.second) {
        result.first->second = create(index, size);
        return result.first->second.get();
    } else {
        return nullptr;
    }
}

} // namespace gfx
} // namespace mbgl
