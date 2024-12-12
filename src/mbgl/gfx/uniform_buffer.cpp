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
    for (size_t id = 0; id < other.uniformBufferVector.size(); id++) {
        uniformBufferVector[id] = other.uniformBufferVector[id];
    }
    return *this;
}

const std::shared_ptr<UniformBuffer>& UniformBufferArray::get(const size_t id) const {
    return (id < uniformBufferVector.size()) ? uniformBufferVector[id] : nullref;
}

const std::shared_ptr<UniformBuffer>& UniformBufferArray::set(const size_t id,
                                                              std::shared_ptr<UniformBuffer> uniformBuffer,
                                                              bool bindVertex,
                                                              bool bindFragment) {
    assert(id < uniformBufferVector.size());
    if (id >= uniformBufferVector.size()) {
        return nullref;
    }
    uniformBufferVector[id] = std::move(uniformBuffer);
    if (uniformBufferVector[id]) {
        uniformBufferVector[id]->setBindVertex(bindVertex);
        uniformBufferVector[id]->setBindFragment(bindFragment);
    }
    return uniformBufferVector[id];
}

void UniformBufferArray::createOrUpdate(const size_t id,
                                        const std::vector<uint8_t>& data,
                                        gfx::Context& context,
                                        bool bindVertex,
                                        bool bindFragment) {
    createOrUpdate(id, data.data(), data.size(), context, bindVertex, bindFragment);
}

void UniformBufferArray::createOrUpdate(
    const size_t id, const void* data, const std::size_t size, gfx::Context& context, bool bindVertex, bool bindFragment) {
    if (auto& ubo = get(id); ubo && ubo->getSize() == size) {
        ubo->update(data, size);
    } else {
        uniformBufferVector[id] = context.createUniformBuffer(data, size, false);
        uniformBufferVector[id]->setBindVertex(bindVertex);
        uniformBufferVector[id]->setBindFragment(bindFragment);
    }
}

} // namespace gfx
} // namespace mbgl
