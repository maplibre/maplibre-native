#include <mbgl/gfx/uniform_buffer.hpp>

#include <mbgl/gfx/context.hpp>

namespace mbgl {
namespace gfx {

std::shared_ptr<UniformBuffer> UniformBufferArray::nullref = nullptr;

UniformBufferArray::UniformBufferArray(UniformBufferArray&& other)
    : uniformBufferVector(std::move(other.uniformBufferVector)) {}

UniformBufferArray& UniformBufferArray::operator=(UniformBufferArray&& other) {
    uniformBufferVector = std::move(other.uniformBufferVector);
    return *this;
}

UniformBufferArray& UniformBufferArray::operator=(const UniformBufferArray& other) {
    for (size_t index = 0; index < other.uniformBufferVector.size(); index++) {
        uniformBufferVector[index] = other.uniformBufferVector[index];
    }
    return *this;
}

const std::shared_ptr<UniformBuffer>& UniformBufferArray::get(const size_t index) const {
    const auto& result = (index < uniformBufferVector.size()) ? uniformBufferVector[index] : nullref;
    return (result != nullptr) ? result : nullref;
}

const std::shared_ptr<UniformBuffer>& UniformBufferArray::addOrReplace(const size_t index,
                                                                       std::shared_ptr<UniformBuffer> uniformBuffer) {
    uniformBufferVector[index] = std::move(uniformBuffer);
    return uniformBufferVector[index];
}

void UniformBufferArray::createOrUpdate(const size_t index,
                                        const std::vector<uint8_t>& data,
                                        gfx::Context& context,
                                        bool persistent) {
    createOrUpdate(index, data.data(), data.size(), context, persistent);
}

void UniformBufferArray::createOrUpdate(
    const size_t index, const void* data, const std::size_t size, gfx::Context& context, bool persistent) {
    if (auto& ubo = get(index); ubo && ubo->getSize() == size) {
        ubo->update(data, size);
    } else {
        uniformBufferVector[index] = context.createUniformBuffer(data, size, persistent);
    }
}

} // namespace gfx
} // namespace mbgl
