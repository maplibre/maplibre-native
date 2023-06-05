#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mbgl {
namespace gfx {

class Context;

class UniformBuffer {
    // Can only be created by platform specific implementations
protected:
    UniformBuffer(std::size_t size_)
        : size(size_) {}
    UniformBuffer(const UniformBuffer&) = default;
    UniformBuffer(UniformBuffer&& other)
        : size(other.size) {}

public:
    virtual ~UniformBuffer() = default;
    virtual void update(const void* data, std::size_t size_) = 0;

    std::size_t getSize() const { return size; }

protected:
    UniformBuffer& operator=(const UniformBuffer&) = default;
    UniformBuffer& operator=(UniformBuffer&& other) {
        size = other.size;
        return *this;
    }

protected:
    std::size_t size;
};

/// Stores a collection of uniform buffers by name
class UniformBufferArray {
public:
    using UniformBufferMap = std::unordered_map<std::string, std::shared_ptr<UniformBuffer>>;

    UniformBufferArray(int initCapacity = 10);
    UniformBufferArray(UniformBufferArray&&);
    // Would need to use the virtual assignment operator
    UniformBufferArray(const UniformBufferArray&) = delete;
    virtual ~UniformBufferArray() = default;

    /// Number of elements
    std::size_t size() const { return uniformBufferMap.size(); }

    /// Get an uniform buffer element.
    /// Returns a pointer to the element on success, or null if the uniform buffer doesn't exists.
    const std::shared_ptr<UniformBuffer>& get(std::string_view name) const;

    /// Add a new uniform buffer element or replace the existing one.
    const std::shared_ptr<UniformBuffer>& addOrReplace(std::string_view name,
                                                       std::shared_ptr<UniformBuffer> uniformBuffer);

    /// Create and add a new buffer or update an existing one
    void createOrUpdate(std::string_view name, const std::vector<uint8_t>& data, gfx::Context&);
    void createOrUpdate(std::string_view name, const void* data, std::size_t size, gfx::Context&);
    template <typename T>
    void createOrUpdate(std::string_view name, const T* data, gfx::Context& context) {
        createOrUpdate(name, data, sizeof(T), context);
    }

    UniformBufferArray& operator=(UniformBufferArray&&);
    UniformBufferArray& operator=(const UniformBufferArray&);

protected:
    const std::shared_ptr<UniformBuffer>& add(const std::string_view name, std::shared_ptr<UniformBuffer>&&);

    virtual std::unique_ptr<UniformBuffer> copy(const UniformBuffer& uniformBuffer) = 0;

protected:
    UniformBufferMap uniformBufferMap;
    static std::shared_ptr<UniformBuffer> nullref;
};

} // namespace gfx
} // namespace mbgl
