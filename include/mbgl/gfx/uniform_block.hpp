#pragma once

#include <mbgl/shaders/ubo_max_count.hpp>

namespace mbgl {
namespace gfx {

class UniformBuffer;
class UniformBlock;

using UniqueUniformBlock = std::unique_ptr<UniformBlock>;

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

/// Stores a collection of uniform blocks by id
class UniformBlockArray {
public:
    /// @brief Constructor
    UniformBlockArray() = default;

    /// @brief Move constructor
    UniformBlockArray(UniformBlockArray&&);

    /// @brief Copy constructor. Would need to use the virtual assignment operator
    UniformBlockArray(const UniformBlockArray&) = delete;

    /// @brief  Destructor
    virtual ~UniformBlockArray() = default;

    /// @brief Number of elements
    std::size_t size() const { return uniformBlockVector.size(); }

    /// @brief Get an uniform block element.
    /// @return Pointer to the element on success, or null if the uniform block doesn't exists.
    const std::unique_ptr<UniformBlock>& get(const size_t id) const;

    /// @brief Add a new uniform block element.
    /// @param id
    /// @param index
    /// @param size
    /// @return Pointer to the new element on success, or null if the uniform block already exists.
    const std::unique_ptr<UniformBlock>& add(const size_t id, const size_t index, std::size_t size);

    /// @brief  Move assignment operator
    UniformBlockArray& operator=(UniformBlockArray&&);

    /// @brief  Copy assignment operator
    UniformBlockArray& operator=(const UniformBlockArray&);

    /// Do something with each block
    template <typename Func /* void(const size_t, const UniformBlock&) */>
    void visit(Func f) {
        std::for_each(uniformBlockVector.begin(), uniformBlockVector.end(), [&](const auto& block) {
            if (block) {
                f(0, *block);
            }
        });
    }

protected:
    virtual std::unique_ptr<UniformBlock> create(int index, std::size_t size) = 0;
    virtual std::unique_ptr<UniformBlock> copy(const UniformBlock& uniformBlock) = 0;

protected:
    std::array<UniqueUniformBlock, shaders::maxUBOCountPerShader> uniformBlockVector;
    static std::unique_ptr<UniformBlock> nullref;
};

} // namespace gfx
} // namespace mbgl
