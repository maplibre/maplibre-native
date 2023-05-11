#pragma once

#include <string>
#include <memory>
#include <unordered_map>

namespace mbgl {
namespace gfx {

class UniformBuffer;

class UniformBlock {
    // Can only be created by UniformBlockArray implementations
protected:
    UniformBlock(int index_, std::size_t size_)
        : index(index_),
          size(size_) {}
    UniformBlock(const UniformBlock&) = default;
    UniformBlock(UniformBlock&& other)
        : index(other.index),
          size(other.size) {}

public:
    virtual ~UniformBlock() = default;

    int getIndex() const { return index; }

    std::size_t getSize() const { return size; }

    virtual void bindBuffer(const UniformBuffer& uniformBuffer) = 0;
    virtual void unbindBuffer() = 0;

protected:
    UniformBlock& operator=(const UniformBlock&) = default;
    UniformBlock& operator=(UniformBlock&& other) {
        index = other.index;
        size = other.size;
        return *this;
    }

protected:
    int index;
    std::size_t size;
};

/// Stores a collection of uniform blocks by name
class UniformBlockArray {
public:
    using UniformBlockMap = std::unordered_map<std::string, std::unique_ptr<UniformBlock>>;

    UniformBlockArray(int initCapacity = 10);
    UniformBlockArray(UniformBlockArray&&);
    // Would need to use the virtual assignment operator
    UniformBlockArray(const UniformBlockArray&) = delete;
    virtual ~UniformBlockArray() = default;

    /// Get map of elements.
    const UniformBlockMap& getMap() const { return uniformBlockMap; }

    /// Number of elements
    std::size_t size() const { return uniformBlockMap.size(); }

    /// Get an uniform block element.
    /// Returns a pointer to the element on success, or null if the uniform block doesn't exists.
    const std::unique_ptr<UniformBlock>& get(const std::string& name) const;

    /// Add a new uniform block element.
    /// Returns a pointer to the new element on success, or null if the uniform block already exists.
    const std::unique_ptr<UniformBlock>& add(std::string name, int index, std::size_t size);

    UniformBlockArray& operator=(UniformBlockArray&&);
    UniformBlockArray& operator=(const UniformBlockArray&);

protected:
    const std::unique_ptr<UniformBlock>& add(std::string name, std::unique_ptr<UniformBlock>&& uniformBlock) {
        const auto result = uniformBlockMap.insert(std::make_pair(std::move(name), std::unique_ptr<UniformBlock>()));
        if (result.second) {
            result.first->second = std::move(uniformBlock);
            return result.first->second;
        } else {
            return nullref;
        }
    }

    virtual std::unique_ptr<UniformBlock> create(int index, std::size_t size) = 0;
    virtual std::unique_ptr<UniformBlock> copy(const UniformBlock& uniformBlock) = 0;

protected:
    UniformBlockMap uniformBlockMap;
    static std::unique_ptr<UniformBlock> nullref;
};

} // namespace gfx
} // namespace mbgl
