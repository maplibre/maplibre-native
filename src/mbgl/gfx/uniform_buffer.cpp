#include <mbgl/gfx/uniform_buffer.hpp>

#include <mbgl/gfx/context.hpp>

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

const std::shared_ptr<UniformBuffer>& UniformBufferArray::get(const std::string_view name) const {
    const auto result = uniformBufferMap.find(std::string(name));
    return (result != uniformBufferMap.end()) ? result->second : nullref;
}

const std::shared_ptr<UniformBuffer>& UniformBufferArray::addOrReplace(const std::string_view name,
                                                                       std::shared_ptr<UniformBuffer> uniformBuffer) {
    const auto result = uniformBufferMap.insert(std::make_pair(name, std::shared_ptr<UniformBuffer>()));
    result.first->second = std::move(uniformBuffer);
    return result.first->second;
}

void UniformBufferArray::createOrUpdate(std::string_view name,
                                        const std::vector<uint8_t>& data,
                                        gfx::Context& context) {
    createOrUpdate(name, data.data(), data.size(), context);
}

void UniformBufferArray::createOrUpdate(const std::string_view name,
                                        const void* data,
                                        const std::size_t size,
                                        gfx::Context& context) {
    if (auto& ubo = get(name); ubo && ubo->getSize() == size) {
        ubo->update(data, size);
    } else {
        add(name, context.createUniformBuffer(data, size));
    }
}

const std::shared_ptr<UniformBuffer>& UniformBufferArray::add(const std::string_view name,
                                                              std::shared_ptr<UniformBuffer>&& uniformBuffer) {
    const auto result = uniformBufferMap.insert(std::make_pair(name, std::shared_ptr<UniformBuffer>()));
    if (result.second) {
        result.first->second = std::move(uniformBuffer);
        return result.first->second;
    } else {
        return nullref;
    }
}

} // namespace gfx
} // namespace mbgl
