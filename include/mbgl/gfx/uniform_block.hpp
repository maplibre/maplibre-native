#pragma once

#include <mbgl/util/string_indexer.hpp>
#include <mbgl/util/containers.hpp>

#include <algorithm>
#include <functional>
#include <string>
#include <memory>
#include <unordered_map>

namespace mbgl {
namespace gfx {

class UniformBuffer;

/// @brief This class represents an uniform block
class UniformBlock {
protected:
    /// @brief Constructor. Can only be created by UniformBlockArray implementations
    /// @param index_
    /// @param size_
    UniformBlock(int index_, std::size_t size_)
        : index(index_),
          size(size_) {}
    UniformBlock(const UniformBlock&) = default;
    UniformBlock(UniformBlock&& other)
        : index(other.index),
          size(other.size) {}

public:
    /// @brief Destructor
    virtual ~UniformBlock() = default;

    /// @brief Retrieves the index of this uniform block
    /// @return int
    int getIndex() const { return index; }

    /// @brief Get the size of the uniform block
    /// @return std::size_t
    std::size_t getSize() const { return size; }

    /// @brief Binds the buffer
    /// @param uniformBuffer
    virtual void bindBuffer(const UniformBuffer& uniformBuffer) = 0;

    /// @brief Unbinds the uniform buffer
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
    using UniformBlockMap = mbgl::unordered_map<StringIdentity, std::unique_ptr<UniformBlock>>;

    /// @brief Constructor
    UniformBlockArray() = default;

    /// @brief Move constructor
    UniformBlockArray(UniformBlockArray&&);

    /// @brief Copy constructor. Would need to use the virtual assignment operator
    UniformBlockArray(const UniformBlockArray&) = delete;

    /// @brief  Destructor
    virtual ~UniformBlockArray() = default;

    /// @brief Get map of elements.
    const UniformBlockMap& getMap() const { return uniformBlockMap; }

    /// @brief Number of elements
    std::size_t size() const { return uniformBlockMap.size(); }

    /// @brief Get an uniform block element.
    /// @return Pointer to the element on success, or null if the uniform block doesn't exists.
    const std::unique_ptr<UniformBlock>& get(const StringIdentity id) const;

    /// @brief Add a new uniform block element.
    /// @param name
    /// @param index
    /// @param size
    /// @return Pointer to the new element on success, or null if the uniform block already exists.
    const std::unique_ptr<UniformBlock>& add(const StringIdentity id, int index, std::size_t size);

    /// @brief  Move assignment operator
    UniformBlockArray& operator=(UniformBlockArray&&);

    /// @brief  Copy assignment operator
    UniformBlockArray& operator=(const UniformBlockArray&);

    /// Do something with each block
    template <typename Func /* void(const StringIdentity, const UniformBlock&) */>
    void visit(Func f) {
        std::for_each(uniformBlockMap.begin(), uniformBlockMap.end(), [&](const auto& kv) {
            if (kv.second) {
                f(kv.first, *kv.second);
            }
        });
    }

protected:
    const std::unique_ptr<UniformBlock>& add(const StringIdentity id, std::unique_ptr<UniformBlock>&& uniformBlock) {
        const auto result = uniformBlockMap.insert(std::make_pair(id, std::unique_ptr<UniformBlock>()));
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
