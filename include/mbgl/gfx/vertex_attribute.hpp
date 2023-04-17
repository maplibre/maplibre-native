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

//    static_assert(sizeof(Type) < 256, "vertex type must be smaller than 256 bytes");
//    static_assert(std::is_standard_layout_v<Type>, "vertex type must use standard layout");
//    static_assert(I < Descriptor::data.count, "attribute index must be in range");

class VertexAttributeArray;

class VertexAttribute {
public:
    using float2 = std::array<float, 2>;
    using float3 = std::array<float, 3>;
    using matf2 = std::array<float, 2*2>;
    using matf3 = std::array<float, 3*3>;
    using matf4 = std::array<float, 4*4>;
    using int2 = std::array<std::int32_t, 2>;
    using int3 = std::array<std::int32_t, 3>;
    using int4 = std::array<std::int32_t, 4>;

    using ElementType = std::variant<std::int32_t, int2, int3, int4, float, float2, float3, matf2, matf4>;

    // Can only be created by VertexAttributeArray implementations
protected:
    VertexAttribute(int index_, AttributeDataType dataType_, std::size_t count_)
        : index(index_),
          dataType(dataType_),
          items(count_) {
    }
    VertexAttribute(const VertexAttribute &) = default;
    VertexAttribute(VertexAttribute&& other)
        : index(other.index),
          dataType(other.dataType),
          items(std::move(other.items)) {
    }

public:
    ~VertexAttribute() = default;

    int getIndex() const { return index; }
    std::size_t getCount() const { return items.size(); }
    AttributeDataType getDataType() const { return dataType; }

    const ElementType& get(std::size_t i) const { return items[i]; }

    template <typename T>
    const ElementType& set(std::size_t i, T value) {
        dirty = true;   // need to rebuild the raw data next time
        return items[i] = value;
    }

protected:
    VertexAttribute& operator=(const VertexAttribute&) = default;
    VertexAttribute& operator=(const VertexAttribute&& other) {
        index = other.index;
        dataType = other.dataType;
        items = std::move(other.items);
        return *this;
    }

protected:
    int index;

    /// indicates that a value has changed and any cached result should be discarded
    mutable bool dirty = true;

    AttributeDataType dataType;
    std::vector<ElementType> items;
};

/// Stores a collection of vertex attributes by name
class VertexAttributeArray {
public:
    using AttributeMap = std::unordered_map<std::string, std::unique_ptr<VertexAttribute>>;

    VertexAttributeArray(int initCapacity = 10);
    VertexAttributeArray(VertexAttributeArray &&);
    VertexAttributeArray(const VertexAttributeArray&);
    virtual ~VertexAttributeArray() = default;

    std::size_t size() const { return attrs.size(); }

    /// Add a new attribute element.
    /// Returns a pointer to the new element on success, or null if the attribute already exists.
    /// The result is valid only until the next non-const method call on this class.
    VertexAttribute* get(const std::string& name) const;

    /// Add a new attribute element.
    /// Returns a pointer to the new element on success, or null if the attribute already exists.
    /// The result is valid only until the next non-const method call on this class.
    VertexAttribute* add(std::string name, int index, AttributeDataType, std::size_t count);

    /// Add a new attribute element if it doesn't already exist.
    /// Returns a pointer to the new element on success, or null if the type or count conflict with an existing entry.
    /// The result is valid only until the next non-const method call on this class.
    VertexAttribute* getOrAdd(std::string name, int index, AttributeDataType, std::size_t count);

    using ResolveDelegate = std::function<void(const std::string&, const VertexAttribute&, const VertexAttribute*)>;
    /// Call the provided delegate with each value, providing the override if one exists.
    void resolve(const VertexAttributeArray& overrides, ResolveDelegate) const;

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

    virtual std::unique_ptr<VertexAttribute> create(int index, AttributeDataType dataType, std::size_t count) = 0;
    virtual std::unique_ptr<VertexAttribute> copy(const VertexAttribute& attr) = 0;

protected:
    AttributeMap attrs;
};

} // namespace gfx
} // namespace mbgl
