#pragma once

#include <mbgl/shaders/ubo_max_count.hpp>
#include <mbgl/util/string_indexer.hpp>

namespace mbgl {
namespace gfx {

class Context;
class UniformBuffer;
class UniformBufferArray;

using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
using UniqueUniformBuffer = std::unique_ptr<UniformBuffer>;
using UniqueUniformBufferArray = std::unique_ptr<UniformBufferArray>;

class UniformBuffer {
    // Can only be created by platform specific implementations
protected:
    UniformBuffer(std::size_t size_)
        : size(size_) {}
    UniformBuffer(const UniformBuffer& other)
        : size(other.size) {}
    UniformBuffer(UniformBuffer&& other)
        : size(other.size) {}

public:
    virtual ~UniformBuffer() = default;
    virtual void update(const void* data, std::size_t size_) = 0;

    std::size_t getSize() const { return size; }

    UniformBuffer& operator=(const UniformBuffer&) = delete;

protected:
    UniformBuffer& operator=(UniformBuffer&& other) {
        size = other.size;
        return *this;
    }

protected:
    std::size_t size;
};

/// Stores a collection of uniform buffers by id
class UniformBufferArray {
public:
    UniformBufferArray() = default;
    UniformBufferArray(UniformBufferArray&&);
    // Would need to use the virtual assignment operator
    UniformBufferArray(const UniformBufferArray&) = delete;
    virtual ~UniformBufferArray() = default;

    /// Number of elements
    std::size_t size() const { return uniformBufferVector.size(); }

    /// Get an uniform buffer element.
    /// Returns a pointer to the element on success, or null if the uniform buffer doesn't exists.
    const std::shared_ptr<UniformBuffer>& get(const size_t id) const;

    /// Add a new uniform buffer element or replace the existing one.
    const std::shared_ptr<UniformBuffer>& addOrReplace(const size_t id, std::shared_ptr<UniformBuffer> uniformBuffer);

    /// Create and add a new buffer or update an existing one
    void createOrUpdate(const size_t id, const std::vector<uint8_t>& data, gfx::Context&, bool persistent = false);
    void createOrUpdate(const size_t id, const void* data, std::size_t size, gfx::Context&, bool persistent = false);
    template <typename T>
    std::enable_if_t<!std::is_pointer_v<T>> createOrUpdate(const size_t id,
                                                           const T* data,
                                                           gfx::Context& context,
                                                           bool persistent = false) {
        createOrUpdate(id, data, sizeof(T), context, persistent);
    }

    UniformBufferArray& operator=(UniformBufferArray&&);
    UniformBufferArray& operator=(const UniformBufferArray&);

protected:
    virtual std::unique_ptr<UniformBuffer> copy(const UniformBuffer&) = 0;

protected:
    std::array<UniformBufferPtr, shaders::maxUBOCountPerShader> uniformBufferVector;
    static std::shared_ptr<UniformBuffer> nullref;
};

} // namespace gfx
} // namespace mbgl
