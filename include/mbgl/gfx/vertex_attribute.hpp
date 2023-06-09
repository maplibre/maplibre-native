#pragma once

#include <mbgl/gfx/gfx_types.hpp>

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

using mat2 = std::array<double, 2 * 2>;
using mat3 = std::array<double, 3 * 3>;
using mat4 = std::array<double, 4 * 4>;

namespace gfx {

class ShaderProgramBase;

class VertexAttribute {
public:
    using float2 = std::array<float, 2>;
    using float3 = std::array<float, 3>;
    using float4 = std::array<float, 4>;
    using matf2 = std::array<float, 2 * 2>;
    using matf3 = std::array<float, 3 * 3>;
    using matf4 = std::array<float, 4 * 4>;
    using int2 = std::array<std::int32_t, 2>;
    using int3 = std::array<std::int32_t, 3>;
    using int4 = std::array<std::int32_t, 4>;
    using ushort8 = std::array<std::uint16_t, 8>;

    using ElementType = std::variant<std::int32_t, ushort8, int2, int3, int4, float, float2, float3, float4, matf3, matf4>;

    VertexAttribute(int index_, AttributeDataType dataType_, int size_, std::size_t count_, std::size_t stride_)
        : index(index_),
          size(size_),
          stride((int)stride_),
          dataType(dataType_),
          items(count_) {}
    VertexAttribute(const VertexAttribute&) = default;
    VertexAttribute(VertexAttribute&& other)
        : index(other.index),
          size(other.size),
          dataType(other.dataType),
          items(std::move(other.items)) {}

public:
    virtual ~VertexAttribute() = default;

    int getIndex() const { return index; }
    void setIndex(int value) { index = value; }

    std::size_t getSize() const { return size; }

    std::size_t getStride() const { return stride; }

    std::size_t getCount() const { return items.size(); }
    AttributeDataType getDataType() const { return dataType; }

    const ElementType& get(std::size_t i) const { return items[i]; }

    void reserve(std::size_t count) { items.reserve(count); }

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
    const ElementType& setVariant(std::size_t i, const ElementType& value) {
        if (items.size() < i + 1) {
            items.resize(std::max(items.size(), i + 1));
        }
        setDirty();
        return items[i] = value;
    }

    void clear() {
        if (!items.empty()) {
            setDirty();
        }
        items.clear();
    }

    bool isDirty() const { return dirty; }
    void setDirty() {
        dirty = true;
        rawData.clear();
    }

    template<std::size_t I = 0, typename... Tp>
    inline typename std::enable_if<I == sizeof...(Tp), void>::type
    set(std::size_t, std::tuple<Tp...>, std::size_t) {}

    template<std::size_t I = 0, typename... Tp>
    inline typename std::enable_if<I < sizeof...(Tp), void>::type
    set(std::size_t i, std::tuple<Tp...> tuple, std::size_t tupleIndex) {
        if (tupleIndex == 0) {
            set(i, std::get<I>(tuple).a1);
        } else {
            set<I + 1, Tp...>(i, tuple, tupleIndex - 1);
        }
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
    VertexAttributeArray(VertexAttributeArray&&);
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
    const std::unique_ptr<VertexAttribute>& get(const std::string& name) const;

    /// Add a new attribute element.
    /// Returns a pointer to the new element on success, or null if the attribute already exists.
    /// The result is valid only until the next non-const method call on this class.
    const std::unique_ptr<VertexAttribute>& add(std::string name,
                                                int index = -1,
                                                AttributeDataType = AttributeDataType::Invalid,
                                                int size = 1,
                                                std::size_t count = 1);

    /// Add a new attribute element if it doesn't already exist.
    /// Returns a pointer to the new element on success, or null if the type or count conflict with an existing entry.
    /// The result is valid only until the next non-const method call on this class.
    const std::unique_ptr<VertexAttribute>& getOrAdd(std::string name,
                                                     int index = -1,
                                                     AttributeDataType = AttributeDataType::Invalid,
                                                     int size = 1,
                                                     std::size_t count = 1);

    // Set a value if the element is present
    template <typename T>
    bool set(const std::string& name, std::size_t i, T value) {
        if (const auto& item = get(name)) {
            return item->set(i, value);
        }
        return false;
    }

    /// Indicates whether any values have changed
    bool isDirty() const {
        return std::any_of(
            attrs.begin(), attrs.end(), [](const auto& kv) { return kv.second && kv.second->isDirty(); });
    }

    void clear();

    /// Do something with each attribute
    void observeAttributes(const std::function<void(const std::string&, VertexAttribute&)>& f) {
        std::for_each(attrs.begin(), attrs.end(), [&](const auto& kv) {
            if (kv.second) {
                f(kv.first, *kv.second);
            }
        });
    }
    void observeAttributes(const std::function<void(const std::string&, const VertexAttribute&)>& f) const {
        std::for_each(attrs.begin(), attrs.end(), [&](const auto& kv) {
            if (kv.second) {
                f(kv.first, *kv.second);
            }
        });
    }

    using ResolveDelegate =
        std::function<void(const std::string&, const VertexAttribute&, const std::unique_ptr<VertexAttribute>&)>;
    /// Call the provided delegate with each value, providing the override if one exists.
    void resolve(const VertexAttributeArray& overrides, ResolveDelegate) const;

    VertexAttributeArray& operator=(VertexAttributeArray&&);
    VertexAttributeArray& operator=(const VertexAttributeArray&);

    virtual std::unique_ptr<VertexAttributeArray> clone() const {
        auto newAttrs = std::make_unique<VertexAttributeArray>();
        newAttrs->copy(*this);
        return newAttrs;
    }

    template <class... DataDrivenPaintProperty, class Binders, class Evaluated>
    std::vector<std::string> readDataDrivenPaintProperties(const Binders& binders, const Evaluated& evaluated) {
        std::vector<std::string> propertiesAsUniforms;
        (
            [&](const auto& attributeNames) {
                for(std::size_t attrIndex = 0; attrIndex < attributeNames.size(); ++attrIndex) {
                    const auto& attributeName = std::string(attributeNames[attrIndex]);
                    if (auto& binder = binders.template get<DataDrivenPaintProperty>()) {
                        const auto vertexCount = binder->getVertexCount();
                        const auto isConstant = evaluated.template get<DataDrivenPaintProperty>().isConstant();
                        if (vertexCount > 0 && !isConstant) {
                            if (auto& attr = getOrAdd("a_" + attributeName)) {
                                for (std::size_t i = 0; i < vertexCount; ++i) {
                                    attr->set(i, binder->getVertexValue(i), attrIndex);
                                }
                            }
                            propertiesAsUniforms.emplace_back("");
                        } else {
                            propertiesAsUniforms.emplace_back(attributeName);
                        }
                    }
                }
            }(DataDrivenPaintProperty::AttributeNames),
            ...);
        return propertiesAsUniforms;
    }

protected:
    const std::unique_ptr<VertexAttribute>& add(std::string name, std::unique_ptr<VertexAttribute>&& attr) {
        const auto result = attrs.insert(std::make_pair(std::move(name), std::unique_ptr<VertexAttribute>()));
        if (result.second) {
            result.first->second = std::move(attr);
            return result.first->second;
        } else {
            return nullref;
        }
    }

    virtual std::unique_ptr<VertexAttribute> create(int index,
                                                    AttributeDataType dataType,
                                                    int size,
                                                    std::size_t count) const {
        return std::make_unique<VertexAttribute>(index, dataType, size, count, size * count);
    }

    virtual void copy(const VertexAttributeArray& other) {
        for (const auto& kv : other.attrs) {
            if (kv.second) {
                attrs.insert(std::make_pair(kv.first, copy(*kv.second)));
            }
        }
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
