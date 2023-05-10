#pragma once

#include <string>

namespace mbgl {
namespace gfx {

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
    UniformBuffer* get(const std::string& name) const;

    /// Add a new uniform buffer element or replace the existing one.
    UniformBuffer* addOrReplace(std::string name,
                                const std::shared_ptr<UniformBuffer>& uniformBuffer);

    UniformBufferArray& operator=(UniformBufferArray&&);
    UniformBufferArray& operator=(const UniformBufferArray&);

protected:
    UniformBuffer* add(std::string name, std::shared_ptr<UniformBuffer>&& uniformBuffer) {
        const auto result = uniformBufferMap.insert(std::make_pair(std::move(name), std::shared_ptr<UniformBuffer>()));
        if (result.second) {
            result.first->second = std::move(uniformBuffer);
            return result.first->second.get();
        } else {
            return nullptr;
        }
    }

    virtual std::unique_ptr<UniformBuffer> copy(const UniformBuffer& uniformBuffer) = 0;

protected:
    UniformBufferMap uniformBufferMap;
};

} // namespace gfx
} // namespace mbgl
