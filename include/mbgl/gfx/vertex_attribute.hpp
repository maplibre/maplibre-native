#pragma once

#include <mbgl/gfx/gfx_types.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace mbgl {

using mat2 = std::array<double, 2*2>;
using mat3 = std::array<double, 3*3>;
using mat4 = std::array<double, 4*4>;

namespace gfx {

class ShaderProgramBase;

class VertexAttribute {
public:
    using float2 = std::array<float, 2>;
    using float3 = std::array<float, 3>;
    using float4 = std::array<float, 4>;
    using matf2 = std::array<float, 2*2>;
    using matf3 = std::array<float, 3*3>;
    using matf4 = std::array<float, 4*4>;
    using int2 = std::array<std::int32_t, 2>;
    using int3 = std::array<std::int32_t, 3>;
    using int4 = std::array<std::int32_t, 4>;

    using ElementType = std::variant<std::int32_t, int2, int3, int4,
                                     float, float2, float3, float4,
                                     matf3, matf4>;

    // Can only be created by VertexAttributeArray implementations
protected:
    VertexAttribute(int index_, AttributeDataType dataType_,
                    int size_, std::size_t count_, std::size_t stride_)
        : index(index_),
          size(size_),
          stride((int)stride_),
          dataType(dataType_),
          items(count_) {
    }
    VertexAttribute(const VertexAttribute &) = default;
    VertexAttribute(VertexAttribute&& other)
        : index(other.index),
          size(other.size),
          dataType(other.dataType),
          items(std::move(other.items)) {
    }

public:
    ~VertexAttribute() = default;

    int getIndex() const { return index; }
    void setIndex(int value) { index = value; }

    std::size_t getSize() const { return size; }

    std::size_t getStride() const { return stride; }

    std::size_t getCount() const { return items.size(); }
    AttributeDataType getDataType() const { return dataType; }

    const ElementType& get(std::size_t i) const { return items[i]; }

    template <typename T>
    const ElementType& set(std::size_t i, T value) {
        dirty = true;   // need to rebuild the raw data next time
        items.resize(std::max(items.size(), i + 1));
        return items[i] = value;
    }

    bool getDirty() const { return dirty; }
    void clearDirty() {
        dirty = false;
        rawData.clear();
    }

protected:
    VertexAttribute& operator=(const VertexAttribute&) = default;
    VertexAttribute& operator=(VertexAttribute&& other) {
        index = other.index;
        size = other.size;
        dataType = other.dataType;
        items = std::move(other.items);
        return *this;
    }

protected:
    int index;
    int size;
    int stride;

    /// indicates that a value has changed and any cached result should be discarded
    mutable bool dirty = true;

    AttributeDataType dataType;
    std::vector<ElementType> items;
    mutable std::vector<std::uint8_t> rawData;
};

/// Stores a collection of vertex attributes by name
class VertexAttributeArray {
public:
    using AttributeMap = std::unordered_map<std::string, std::unique_ptr<VertexAttribute>>;

    VertexAttributeArray(int initCapacity = 10);
    VertexAttributeArray(VertexAttributeArray &&);
    // Would need to use the virtual assignment operator
    VertexAttributeArray(const VertexAttributeArray&) = delete;
    virtual ~VertexAttributeArray() = default;

    /// Number of elements
    std::size_t size() const { return attrs.size(); }

    /// Sum of element strides, and the total size of a vertex in the buffer
    std::size_t getTotalSize() const;

    /// Get the largest count value of the attribute elements
    std::size_t getMaxCount() const;

    /// Add a new attribute element.
    /// Returns a pointer to the new element on success, or null if the attribute already exists.
    /// The result is valid only until the next non-const method call on this class.
    VertexAttribute* get(const std::string& name) const;

    /// Add a new attribute element.
    /// Returns a pointer to the new element on success, or null if the attribute already exists.
    /// The result is valid only until the next non-const method call on this class.
    VertexAttribute* add(std::string name,
                         int index = -1,
                         AttributeDataType = AttributeDataType::Invalid,
                         int size = 1,
                         std::size_t count = 1);

    /// Add a new attribute element if it doesn't already exist.
    /// Returns a pointer to the new element on success, or null if the type or count conflict with an existing entry.
    /// The result is valid only until the next non-const method call on this class.
    VertexAttribute* getOrAdd(std::string name,
                              int index = -1,
                              AttributeDataType = AttributeDataType::Invalid,
                              int size = 1,
                              std::size_t count = 1);

    // Set a value if the element is present
    template <typename T>
    bool set(const std::string& name, std::size_t i, T value) {
        if (auto *item = get(name)) {
            return item->set(i, value);
        }
        return false;
    }

    /// Indicates whether any values have changed
    bool isDirty() const;
    void resetDirty();

    using ResolveDelegate = std::function<void(const std::string&, const VertexAttribute&, const VertexAttribute*)>;
    /// Call the provided delegate with each value, providing the override if one exists.
    void resolve(const VertexAttributeArray& overrides, ResolveDelegate) const;

    /// Apply to the active program
    virtual void applyUniforms(const ShaderProgramBase&) = 0;
    
    VertexAttributeArray& operator=(VertexAttributeArray &&);
    VertexAttributeArray& operator=(const VertexAttributeArray&);

protected:
    VertexAttribute* add(std::string name, std::unique_ptr<VertexAttribute>&& attr) {
        const auto result = attrs.insert(std::make_pair(std::move(name), std::unique_ptr<VertexAttribute>()));
        if (result.second) {
            result.first->second = std::move(attr);
            return result.first->second.get();
        } else {
            return nullptr;
        }
    }

    virtual std::unique_ptr<VertexAttribute> create(int index, AttributeDataType dataType,
                                                    int size, std::size_t count) = 0;
    virtual std::unique_ptr<VertexAttribute> copy(const VertexAttribute& attr) = 0;

protected:
    AttributeMap attrs;
};

} // namespace gfx
} // namespace mbgl
