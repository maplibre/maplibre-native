#include <mbgl/gfx/uniform_buffer.hpp>

namespace mbgl {
namespace gfx {

std::shared_ptr<UniformBuffer> UniformBufferArray::nullref = nullptr;

UniformBufferArray::UniformBufferArray(int initCapacity)
    : uniformBufferMap(initCapacity) {}

UniformBufferArray::UniformBufferArray(UniformBufferArray&& other)
    : uniformBufferMap(std::move(other.uniformBufferMap)) {}

UniformBufferArray& UniformBufferArray::operator=(UniformBufferArray&& other) {
    uniformBufferMap = std::move(other.uniformBufferMap);
    return *this;
}

UniformBufferArray& UniformBufferArray::operator=(const UniformBufferArray& other) {
    uniformBufferMap.clear();
    for (const auto& kv : other.uniformBufferMap) {
        add(kv.first, copy(*kv.second));
    }
    return *this;
}

const std::shared_ptr<UniformBuffer>& UniformBufferArray::get(const std::string& name) const {
    const auto result = uniformBufferMap.find(name);
    return (result != uniformBufferMap.end()) ? result->second : nullref;
}

const std::shared_ptr<UniformBuffer>& UniformBufferArray::addOrReplace(
    std::string name, const std::shared_ptr<UniformBuffer>& uniformBuffer) {
    const auto result = uniformBufferMap.insert(std::make_pair(std::move(name), std::shared_ptr<UniformBuffer>()));
    result.first->second = std::move(uniformBuffer);
    return result.first->second;
}

} // namespace gfx
} // namespace mbgl
