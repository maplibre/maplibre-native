#pragma once

#include <mbgl/gfx/gfx_types.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/util/containers.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
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
class VertexAttribute;
class VertexAttributeArray;
class VertexVectorBase;

using UniqueVertexAttribute = std::unique_ptr<VertexAttribute>;
using StringIDSetsPair = std::pair<unordered_set<std::string_view>, unordered_set<size_t>>;

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
          stride(stride_),
          dataType(dataType_),
          items(count_) {}
    VertexAttribute(int index_, AttributeDataType dataType_, std::size_t count_)
        : index(index_),
          stride(getStrideOf(dataType_)),
          dataType(dataType_),
          items(count_) {}
    VertexAttribute(const VertexAttribute& other)
        : index(other.index),
          stride(other.stride),
          dataType(other.dataType),
          items(other.items),
          sharedRawData(other.sharedRawData),
          sharedType(other.sharedType),
          sharedOffset(other.sharedOffset),
          sharedVertexOffset(other.sharedVertexOffset),
          sharedStride(other.sharedStride) {}
    VertexAttribute(VertexAttribute&& other)
        : index(other.index),
          stride(other.stride),
          dataType(other.dataType),
          items(std::move(other.items)),
          sharedRawData(std::move(other.sharedRawData)),
          sharedType(other.sharedType),
          sharedOffset(other.sharedOffset),
          sharedVertexOffset(other.sharedVertexOffset),
          sharedStride(other.sharedStride) {}

public:
    virtual ~VertexAttribute() = default;

    static std::size_t getStrideOf(gfx::AttributeDataType);

    /// @brief Get the index of the vertex attribute
    int getIndex() const { return index; }

    /// @brief Set the index of the vertex attribute
    void setIndex(int value) { index = value; }

    /// @brief Get the stride of the vertex attribute
    virtual std::size_t getStride() const { return stride; }
    void setStride(std::size_t value) { stride = value; }

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

    /// @brief Set dirty state
    void setDirty(bool value = true) { dirty = value; }

    bool isModifiedAfter(std::chrono::duration<double> time) const {
        if (sharedRawData) {
            return sharedRawData->isModifiedAfter(time);
        }
        return dirty;
    }

    template <std::size_t I = 0, typename... Tp>
    inline void set(std::size_t, std::tuple<Tp...>, std::size_t)
        requires(I == sizeof...(Tp))
    {}

    /// Set item value
    template <std::size_t I = 0, typename... Tp>
    inline void set(std::size_t i, std::tuple<Tp...> tuple, std::size_t tupleIndex)
        requires(I < sizeof...(Tp))
    {
        if (tupleIndex == 0) {
            set(i, std::get<I>(tuple).a1);
        } else {
            set<I + 1, Tp...>(i, tuple, tupleIndex - 1);
        }
    }

    /// Get the vertex attribute's raw data
    std::vector<std::uint8_t>& getRawData() { return rawData; }
    const std::vector<std::uint8_t>& getRawData() const { return rawData; }
    void setRawData(std::vector<std::uint8_t> value) { rawData = std::move(value); }

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

    const gfx::UniqueVertexBufferResource& getBuffer() const { return buffer; }
    void setBuffer(gfx::UniqueVertexBufferResource&& value) { buffer = std::move(value); }

    VertexAttribute& operator=(const VertexAttribute&) = delete;

protected:
    VertexAttribute& operator=(VertexAttribute&& other) {
        index = other.index;
        dataType = other.dataType;
        items = std::move(other.items);
        return *this;
    }

protected:
    friend class VertexAttributeArray;
    bool isDirty() const { return dirty; }

    int index;
    std::size_t stride;

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

    gfx::UniqueVertexBufferResource buffer;
};

/// Stores a collection of vertex attributes by name
class VertexAttributeArray {
public:
    using AttributeVector = std::array<std::unique_ptr<VertexAttribute>, shaders::maxVertexAttributeCountPerShader>;
    VertexAttributeArray() = default;
    VertexAttributeArray(VertexAttributeArray&&);
    VertexAttributeArray(const VertexAttributeArray&) = delete; // Would need to use the virtual assignment operator
    virtual ~VertexAttributeArray() = default;

    /// Number of maximum allocated elements
    std::size_t allocatedSize() const { return attrs.size(); }

    /// Sum of element strides, and the total size of a vertex in the buffer
    std::size_t getTotalSize() const;

    /// Get the largest count value of the attribute elements
    std::size_t getMaxCount() const;

    /// Get a attribute element.
    /// Returns a pointer to the element on success, or null if the attribute doesn't exists.
    const std::unique_ptr<VertexAttribute>& get(const size_t id) const;

    /// Set a new attribute element or replace the existing one.
    /// Returns a pointer to the new element on success, or null if the attribute already exists.
    /// The result is valid only until the next non-const method call on this class.
    /// @param index index to match, or -1 for any
    /// @param type type to match, or `Invalid` for any
    /// @param count Number of items, zero for shared data
    const std::unique_ptr<VertexAttribute>& set(const size_t id,
                                                int index = -1,
                                                AttributeDataType type = AttributeDataType::Invalid,
                                                std::size_t count = 0);

    /// Indicates whether any values have changed
    bool isModifiedAfter(std::chrono::duration<double> time) const {
        return std::ranges::any_of(attrs, [&](const auto& attr) { return attr && attr->isModifiedAfter(time); });
    }

    /// Clear the collection
    void clear();

    /// Do something with each attribute
    template <typename Func /* void(VertexAttribute&) */>
    void visitAttributes(Func f) const {
        std::for_each(attrs.begin(), attrs.end(), [&](const auto& attr) {
            if (attr) {
                f(*attr);
            }
        });
    }

    /// Call the provided delegate with each value, providing the override if one exists.
    /// @param foundDelegate Called for each attribute in `default`, with the corresponding override, if present
    /// @param missingDelegate Called for any overrides with no corresponding default
    template <typename ResolveFunc /* void(const size_t, VertexAttribute&, const std::unique_ptr<VertexAttribute>&) */,
              typename MissingFunc /* void(const size_t, const std::unique_ptr<VertexAttribute>&) */>
    void resolve(const VertexAttributeArray& overrides, ResolveFunc foundDelegate, MissingFunc missingDelegate) const {
        for (size_t id = 0; id < attrs.size(); id++) {
            const auto& override = overrides.get(id);
            if (const auto& attr = attrs[id]) {
                foundDelegate(id, *attr, override);
            } else if (override) {
                missingDelegate(id, override);
            }
        }
    }

    VertexAttributeArray& operator=(VertexAttributeArray&&);
    VertexAttributeArray& operator=(const VertexAttributeArray&) = delete;

    /// Specialized DataDrivenPaintProperty reader
    /// @param binders Property binders for the target shader
    /// @param evaluated Evaluated properties
    /// @param propertiesAsUniforms [out] A set of string identities for the properties which will be constant, not
    /// attributes.
    /// @details The property name IDs refer to the "a\_" prefixed values to match the shader definitions.
    template <typename... DataDrivenPaintProperty, typename Binders, typename Evaluated>
    void readDataDrivenPaintProperties(const Binders& binders,
                                       const Evaluated& evaluated,
                                       StringIDSetsPair* propertiesAsUniforms,
                                       const size_t firstDataDrivenAttrId) {
        // Read each property in the type pack
        if (propertiesAsUniforms) {
            propertiesAsUniforms->first.reserve(sizeof...(DataDrivenPaintProperty));
            propertiesAsUniforms->second.reserve(sizeof...(DataDrivenPaintProperty));
        }
        size_t dataDrivenAttrId = firstDataDrivenAttrId;
        (readDataDrivenPaintProperty<DataDrivenPaintProperty>(binders.template get<DataDrivenPaintProperty>(),
                                                              isConstant<DataDrivenPaintProperty>(evaluated),
                                                              propertiesAsUniforms,
                                                              dataDrivenAttrId),
         ...);
    }

    /// Specialized DataDrivenPaintProperty reader
    /// @param binders Property binders for the target shader
    /// @param evaluated Evaluated properties
    /// @param propertiesAsUniforms [out] A set of string identities for the properties which will be constant, not
    /// attributes.
    /// @details The property name IDs refer to the "a\_" prefixed values to match the shader definitions.
    template <typename... DataDrivenPaintProperty, typename Binders, typename Evaluated>
    void readDataDrivenPaintProperties(const Binders& binders,
                                       const Evaluated& evaluated,
                                       StringIDSetsPair& propertiesAsUniforms,
                                       const size_t firstDataDrivenAttrId) {
        // Read each property in the type pack
        propertiesAsUniforms.first.reserve(sizeof...(DataDrivenPaintProperty));
        propertiesAsUniforms.second.reserve(sizeof...(DataDrivenPaintProperty));

        size_t dataDrivenAttrId = firstDataDrivenAttrId;
        (readDataDrivenPaintProperty<DataDrivenPaintProperty>(binders.template get<DataDrivenPaintProperty>(),
                                                              isConstant<DataDrivenPaintProperty>(evaluated),
                                                              &propertiesAsUniforms,
                                                              dataDrivenAttrId),
         ...);
    }

protected:
    template <typename DataDrivenPaintProperty, typename Evaluated>
    static bool isConstant(const Evaluated& evaluated) noexcept {
        using PropType = decltype(evaluated.template get<DataDrivenPaintProperty>());
        using MethodType = decltype(&std::remove_reference_t<PropType>::isConstant);
        static_assert(std::is_nothrow_invocable_v<MethodType, PropType>, "isConstant() must be noexcept");

        return evaluated.template get<DataDrivenPaintProperty>().isConstant();
    }

    /// Place one property from a type pack into an attribute in this collection, replacing if it already exists.
    template <typename DataDrivenPaintProperty, typename Binder>
    void readDataDrivenPaintProperty(const Binder& binder,
                                     const bool isConstant,
                                     StringIDSetsPair* propertiesAsUniforms,
                                     size_t& dataDrivenAttrId) {
        if (!binder) {
            return;
        }

        // Consider each attribute name in the attribute (e.g., pattern_from, pattern_to)
        for (std::size_t attrIndex = 0; attrIndex < DataDrivenPaintProperty::AttributeNames.size(); ++attrIndex) {
            const auto& attributeName = DataDrivenPaintProperty::AttributeNames[attrIndex];

            // Apply the property, or add it to the uniforms collection if it's constant.
            if (!isConstant && binder->getVertexCount() > 0) {
                using Attribute = typename DataDrivenPaintProperty::Attribute;
                if (const auto& attr = set(dataDrivenAttrId)) {
                    applyPaintProperty<Attribute>(attrIndex, attr, binder);
                }
            } else if (propertiesAsUniforms) {
                propertiesAsUniforms->first.emplace(attributeName);
                propertiesAsUniforms->second.emplace(dataDrivenAttrId);
            }
            dataDrivenAttrId++;
        }
    }

    /// Copy or share the attribute data from a paint property
    template <typename TAttribute, typename TBinder>
    static void applyPaintProperty(const std::size_t attrIndex, const UniqueVertexAttribute& attrib, TBinder& binder) {
        using Type = typename TAttribute::Type; // ::mbgl::gfx::AttributeType<type_, n_>
        using InterpType = ZoomInterpolatedAttributeType<Type>;

        if (!attrib) {
            assert(!"Failed to add attribute");
            return;
        }

        if (const auto& sharedVector = binder->getSharedVertexVector()) {
            const auto rawSize = static_cast<uint32_t>(sharedVector->getRawSize());
            const bool isInterpolated = binder->isInterpolated();
            const auto dataType = isInterpolated ? InterpType::DataType : Type::DataType;
            assert(rawSize == static_cast<uint32_t>(isInterpolated ? sizeof(typename InterpType::Value)
                                                                   : sizeof(typename Type::Value)));
            assert(sharedVector->getRawCount() == binder->getVertexCount());
            attrib->setSharedRawData(std::move(sharedVector), 0, 0, rawSize, dataType);
        } else {
            const auto vertexCount = binder->getVertexCount();
            for (std::size_t i = 0; i < vertexCount; ++i) {
                attrib->set(i, binder->getVertexValue(i), attrIndex);
            }
        }
    }

    virtual UniqueVertexAttribute create(int index, AttributeDataType dataType, std::size_t count) const {
        return std::make_unique<VertexAttribute>(index, dataType, count);
    }

    virtual UniqueVertexAttribute copy(const gfx::VertexAttribute& attr) const {
        return std::make_unique<VertexAttribute>(attr);
    }

protected:
    AttributeVector attrs;
    static const std::unique_ptr<VertexAttribute> nullref;
    static const std::string attributePrefix;
};

} // namespace gfx
} // namespace mbgl
