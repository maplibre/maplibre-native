#pragma once

#include <mbgl/gfx/gfx_types.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/util/string_indexer.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace mbgl {

/// A 2 by 2 matrix
using mat2 = std::array<double, 2 * 2>;

/// A 3 by 3 matrix
using mat3 = std::array<double, 3 * 3>;

/// A 4 by 4 matrix
using mat4 = std::array<double, 4 * 4>;

namespace gfx {

class ShaderProgramBase;
class VertexVectorBase;

class VertexAttribute {
public:
    /// type for vector of 2 floats
    using float2 = std::array<float, 2>;

    /// type for vector of 3 floats
    using float3 = std::array<float, 3>;

    /// type for vector of 3 floats
    using float4 = std::array<float, 4>;

    /// type for 2 by 2 matrix of floats
    using matf2 = std::array<float, 2 * 2>;

    /// type for 3 by 3 matrix of floats
    using matf3 = std::array<float, 3 * 3>;

    /// type for 4 by 4 matrix of floats
    using matf4 = std::array<float, 4 * 4>;

    /// type for vector of 2 integers
    using int2 = std::array<std::int32_t, 2>;

    /// type for vector of 3 integers
    using int3 = std::array<std::int32_t, 3>;

    /// type for vector of 4 integers
    using int4 = std::array<std::int32_t, 4>;

    /// type for vector of 8 unsigned short integers
    using ushort8 = std::array<std::uint16_t, 8>;

    using ElementType =
        std::variant<std::int32_t, ushort8, int2, int3, int4, float, float2, float3, float4, matf3, matf4>;

    /// @brief  Constructor for vertex attribute
    /// @param index_ the index of the vertex attribute
    /// @param dataType_ the vertex attribute data type
    /// @param count_ element count
    /// @param stride_ the vertex attribute stride
    VertexAttribute(int index_, AttributeDataType dataType_, std::size_t count_, std::size_t stride_)
        : index(index_),
          stride((int)stride_),
          dataType(dataType_),
          items(count_) {}
    VertexAttribute(const VertexAttribute& other)
        : index(other.index),
          dataType(other.dataType),
          items(other.items),
          sharedRawData(other.sharedRawData),
          sharedType(other.sharedType),
          sharedOffset(other.sharedOffset),
          sharedVertexOffset(other.sharedVertexOffset),
          sharedStride(other.sharedStride) {}
    VertexAttribute(VertexAttribute&& other)
        : index(other.index),
          dataType(other.dataType),
          items(std::move(other.items)),
          sharedRawData(std::move(other.sharedRawData)),
          sharedType(other.sharedType),
          sharedOffset(other.sharedOffset),
          sharedVertexOffset(other.sharedVertexOffset),
          sharedStride(other.sharedStride) {}

public:
    virtual ~VertexAttribute() = default;

    /// @brief Get the index of the vertex attribute
    int getIndex() const { return index; }

    /// @brief Set the index of the vertex attribute
    void setIndex(int value) { index = value; }

    /// @brief Get the stride of the vertex attribute
    std::size_t getStride() const { return stride; }

    /// @brief Get the count of vertex attribute items
    std::size_t getCount() const;

    /// @brief Get the data type of the vertex attribute
    AttributeDataType getDataType() const { return dataType; }

    /// @brief Get a vertex attribute item
    /// @param index index of the item
    /// @return ElementType reference to the item
    const ElementType& get(std::size_t i) const { return items[i]; }

    /// @brief Reserves space for a number of items
    /// @param count Number of items to reserve memory for
    void reserve(std::size_t count) { items.reserve(count); }

    /// @brief Set the value of an item
    /// @tparam T item data type
    /// @param i index of the item
    /// @param value value to set
    /// @return ElementType reference to item
    template <typename T>
    const ElementType& set(std::size_t i, T value) {
        if (items.size() < i + 1) {
            items.resize(std::max(items.size(), i + 1));
            setDirty(); // need to rebuild the raw data next time
        }
        if (!isDirty()) {
            if (std::holds_alternative<T>(items[i])) {
                // TODO: epsilon for floats?
                if (std::get<T>(items[i]) != value) {
                    setDirty();
                }
            } else {
                // different types
                setDirty();
            }
        }
        return items[i] = value;
    }

    /// @brief Set the value of an item
    /// @param i index of the item
    /// @param value value to set
    /// @return ElementType reference to item
    const ElementType& setVariant(std::size_t i, const ElementType& value) {
        if (items.size() < i + 1) {
            items.resize(std::max(items.size(), i + 1));
        }
        setDirty();
        return items[i] = value;
    }

    /// @brief Clear the items
    void clear() {
        if (!items.empty()) {
            setDirty();
        }
        items.clear();
    }

    /// @brief Get dirty state
    bool isDirty() const { return dirty; }

    /// @brief Set dirty state
    void setDirty(bool value = true) { dirty = value; }

    template <std::size_t I = 0, typename... Tp>
    inline typename std::enable_if<I == sizeof...(Tp), void>::type set(std::size_t, std::tuple<Tp...>, std::size_t) {}

    /// Set item value
    template <std::size_t I = 0, typename... Tp>
        inline typename std::enable_if <
        I<sizeof...(Tp), void>::type set(std::size_t i, std::tuple<Tp...> tuple, std::size_t tupleIndex) {
        if (tupleIndex == 0) {
            set(i, std::get<I>(tuple).a1);
        } else {
            set<I + 1, Tp...>(i, tuple, tupleIndex - 1);
        }
    }

    /// Get the vertex attribute's raw data
    std::vector<std::uint8_t>& getRawData() { return rawData; }
    const std::vector<std::uint8_t>& getRawData() const { return rawData; }

    /// Get the vertex attribute's shared raw data
    const std::shared_ptr<VertexVectorBase>& getSharedRawData() const { return sharedRawData; }

    /// Get the vertex attribute's shared data type
    AttributeDataType getSharedType() const { return sharedType; }

    /// Get the vertex attribute's shared data offset
    uint32_t getSharedOffset() const { return sharedOffset; }

    /// Get the vertex attribute's shared vertex offset
    uint32_t getSharedVertexOffset() const { return sharedVertexOffset; }

    /// Get the vertex attribute's shared data stride
    uint32_t getSharedStride() const { return sharedStride; }

    /// @brief Set the vertex attribute's shared data
    /// @param data shared pointer to vertex vector
    /// @param offset Offset into the source vertex layout structure
    /// @param vertexOffset Offset into the source vertex array
    /// @param stride Stride of the source vertex structure
    /// @param type Type of the attribute
    void setSharedRawData(std::shared_ptr<VertexVectorBase> data,
                          uint32_t offset,
                          uint32_t vertexOffset,
                          uint32_t stride_,
                          AttributeDataType type) {
        sharedRawData = std::move(data);
        sharedType = type;
        sharedOffset = offset;
        sharedVertexOffset = vertexOffset;
        sharedStride = stride_;
    }

    /// @brief Clears the shared data
    void resetSharedRawData() { sharedRawData.reset(); }

protected:
    VertexAttribute& operator=(const VertexAttribute&) = default;
    VertexAttribute& operator=(VertexAttribute&& other) {
        index = other.index;
        dataType = other.dataType;
        items = std::move(other.items);
        return *this;
    }

protected:
    int index;
    int stride;

    /// indicates that a value has changed and any cached result should be discarded
    mutable bool dirty = true;

    AttributeDataType dataType;
    std::vector<ElementType> items;

    // Cache of attribute data
    mutable std::vector<std::uint8_t> rawData;

    std::shared_ptr<VertexVectorBase> sharedRawData;
    AttributeDataType sharedType = AttributeDataType::Invalid;
    uint32_t sharedOffset = 0;
    uint32_t sharedVertexOffset = 0;
    uint32_t sharedStride = 0;
};

/// Stores a collection of vertex attributes by name
class VertexAttributeArray {
public:
    using AttributeMap = std::unordered_map<StringIdentity, std::unique_ptr<VertexAttribute>>;

    VertexAttributeArray() = default;
    VertexAttributeArray(VertexAttributeArray&&);
    VertexAttributeArray(const VertexAttributeArray&) = delete; // Would need to use the virtual assignment operator
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
    const std::unique_ptr<VertexAttribute>& get(const StringIdentity id) const;

    /// Add a new attribute element.
    /// Returns a pointer to the new element on success, or null if the attribute already exists.
    /// The result is valid only until the next non-const method call on this class.
    const std::unique_ptr<VertexAttribute>& add(const StringIdentity id,
                                                int index = -1,
                                                AttributeDataType = AttributeDataType::Invalid,
                                                std::size_t count = 1);

    /// Add a new attribute element if it doesn't already exist.
    /// Returns a pointer to the new element on success, or null if the type or count conflict with an existing entry.
    /// The result is valid only until the next non-const method call on this class.
    /// @param index index to match, or -1 for any
    /// @param type type to match, or `Invalid` for any
    /// @param count type to match, or 0 for any
    const std::unique_ptr<VertexAttribute>& getOrAdd(const StringIdentity id,
                                                     int index = -1,
                                                     AttributeDataType type = AttributeDataType::Invalid,
                                                     std::size_t count = 0);

    // Set a value if the element is present
    template <typename T>
    bool set(const StringIdentity id, std::size_t i, T value) {
        if (const auto& item = get(id)) {
            return item->set(i, value);
        }
        return false;
    }

    /// Indicates whether any values have changed
    virtual bool isDirty() const {
        return std::any_of(
            attrs.begin(), attrs.end(), [](const auto& kv) { return kv.second && kv.second->isDirty(); });
    }

    /// Clear the collection
    void clear();

    /// Do something with each attribute
    void visitAttributes(const std::function<void(const StringIdentity, VertexAttribute&)>& f) {
        std::for_each(attrs.begin(), attrs.end(), [&](const auto& kv) {
            if (kv.second) {
                f(kv.first, *kv.second);
            }
        });
    }

    /// Do something with each attribute
    void visitAttributes(const std::function<void(const StringIdentity, const VertexAttribute&)>& f) const {
        std::for_each(attrs.begin(), attrs.end(), [&](const auto& kv) {
            if (kv.second) {
                f(kv.first, *kv.second);
            }
        });
    }

    using ResolveDelegate =
        std::function<void(const StringIdentity, VertexAttribute&, const std::unique_ptr<VertexAttribute>&)>;
    /// Call the provided delegate with each value, providing the override if one exists.
    void resolve(const VertexAttributeArray& overrides, ResolveDelegate) const;

    VertexAttributeArray& operator=(VertexAttributeArray&&);
    VertexAttributeArray& operator=(const VertexAttributeArray&);

    /// Clone the collection
    virtual std::unique_ptr<VertexAttributeArray> clone() const {
        auto newAttrs = std::make_unique<VertexAttributeArray>();
        newAttrs->copy(*this);
        return newAttrs;
    }

    /// Specialized DataDrivenPaintProperty reader
    template <class... DataDrivenPaintProperty, class Binders, class Evaluated>
    std::vector<std::string> readDataDrivenPaintProperties(const Binders& binders, const Evaluated& evaluated) {
        std::vector<std::string> propertiesAsUniforms;
        propertiesAsUniforms.reserve(sizeof...(DataDrivenPaintProperty));
        (
            [&](const auto& attributeNames) {
                for (std::size_t attrIndex = 0; attrIndex < attributeNames.size(); ++attrIndex) {
                    const auto& attributeName = attributeNames[attrIndex];
                    if (auto& binder = binders.template get<DataDrivenPaintProperty>()) {
                        using Attribute = typename DataDrivenPaintProperty::Attribute;
                        using Type = typename Attribute::Type; // ::mbgl::gfx::AttributeType<type_, n_>
                        using InterpType = ZoomInterpolatedAttributeType<Type>;

                        const auto vertexCount = binder->getVertexCount();
                        const auto isConstant = evaluated.template get<DataDrivenPaintProperty>().isConstant();
                        if (vertexCount > 0 && !isConstant) {
                            auto& attributeNameID = DataDrivenPaintProperty::AttributeNameIDs[attrIndex];
                            if (!attributeNameID) {
                                static const std::string attributePrefix = "a_";
                                attributeNameID = StringIndexer::get(attributePrefix + attributeName.data());
                            }

                            if (auto& attr = getOrAdd(*attributeNameID)) {
                                if (const auto& sharedVector = binder->getSharedVertexVector()) {
                                    const auto rawSize = static_cast<uint32_t>(sharedVector->getRawSize());
                                    const bool isInterpolated = binder->isInterpolated();
                                    const auto dataType = isInterpolated ? InterpType::DataType : Type::DataType;
                                    assert(rawSize == static_cast<uint32_t>(isInterpolated
                                                                                ? sizeof(typename InterpType::Value)
                                                                                : sizeof(typename Type::Value)));
                                    assert(sharedVector->getRawCount() == vertexCount);
                                    attr->setSharedRawData(std::move(sharedVector), 0, 0, rawSize, dataType);
                                } else {
                                    for (std::size_t i = 0; i < vertexCount; ++i) {
                                        attr->set(i, binder->getVertexValue(i), attrIndex);
                                    }
                                }
                            }
                            propertiesAsUniforms.emplace_back(std::string());
                        } else {
                            propertiesAsUniforms.emplace_back(attributeName);
                        }
                    }
                }
            }(DataDrivenPaintProperty::AttributeNames),
            ...);
        return propertiesAsUniforms;
    }

    /// Copy another collection into this one
    virtual void copy(const VertexAttributeArray& other) {
        for (const auto& kv : other.attrs) {
            if (kv.second) {
                attrs.insert(std::make_pair(kv.first, copy(*kv.second)));
            }
        }
    }

protected:
    const std::unique_ptr<VertexAttribute>& add(StringIdentity id, std::unique_ptr<VertexAttribute>&& attr) {
        const auto result = attrs.insert(std::make_pair(id, std::unique_ptr<VertexAttribute>()));
        if (result.second) {
            result.first->second = std::move(attr);
            return result.first->second;
        } else {
            return nullref;
        }
    }

    virtual std::unique_ptr<VertexAttribute> create(int index, AttributeDataType dataType, std::size_t count) const {
        return std::make_unique<VertexAttribute>(index, dataType, count, count);
    }

    virtual std::unique_ptr<VertexAttribute> copy(const gfx::VertexAttribute& attr) const {
        return std::make_unique<VertexAttribute>(attr);
    }

protected:
    AttributeMap attrs;
    static std::unique_ptr<VertexAttribute> nullref;
};

} // namespace gfx
} // namespace mbgl
