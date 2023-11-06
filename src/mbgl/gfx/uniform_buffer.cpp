#include <mbgl/gfx/uniform_buffer.hpp>

#include <mbgl/gfx/context.hpp>

namespace mbgl {
namespace gfx {

std::shared_ptr<UniformBuffer> UniformBufferArray::nullref = nullptr;

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

const std::shared_ptr<UniformBuffer>& UniformBufferArray::get(const StringIdentity id) const {
    const auto result = uniformBufferMap.find(id);
    return (result != uniformBufferMap.end()) ? result->second : nullref;
}

const std::shared_ptr<UniformBuffer>& UniformBufferArray::addOrReplace(const StringIdentity id,
                                                                       std::shared_ptr<UniformBuffer> uniformBuffer) {
    const auto result = uniformBufferMap.insert(std::make_pair(id, std::shared_ptr<UniformBuffer>()));
    result.first->second = std::move(uniformBuffer);
    return result.first->second;
}

void UniformBufferArray::createOrUpdate(const StringIdentity id,
                                        const std::vector<uint8_t>& data,
                                        gfx::Context& context,
                                        bool persistent) {
    createOrUpdate(id, data.data(), data.size(), context, persistent);
}

void UniformBufferArray::createOrUpdate(
    const StringIdentity id, const void* data, const std::size_t size, gfx::Context& context, bool persistent) {
    if (auto& ubo = get(id); ubo && ubo->getSize() == size) {
        ubo->update(data, size);
    } else {
        add(id, context.createUniformBuffer(data, size, persistent));
    }
}

const std::shared_ptr<UniformBuffer>& UniformBufferArray::add(const StringIdentity id,
                                                              std::shared_ptr<UniformBuffer>&& uniformBuffer) {
    const auto result = uniformBufferMap.insert(std::make_pair(id, std::shared_ptr<UniformBuffer>()));
    if (result.second) {
        result.first->second = std::move(uniformBuffer);
        return result.first->second;
    } else {
        return nullref;
    }
}

} // namespace gfx
} // namespace mbgl
