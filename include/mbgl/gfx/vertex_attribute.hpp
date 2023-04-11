#pragma once

#include <mbgl/gfx/gfx_types.hpp>

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace mbgl {

using mat2 = std::array<double, 4>;
using mat3 = std::array<double, 9>;
using mat4 = std::array<double, 16>;

using matf2 = std::array<float, 4>;
using matf3 = std::array<float, 9>;
using matf4 = std::array<float, 16>;

namespace gfx {

//    static_assert(sizeof(Type) < 256, "vertex type must be smaller than 256 bytes");
//    static_assert(std::is_standard_layout_v<Type>, "vertex type must use standard layout");
//    static_assert(I < Descriptor::data.count, "attribute index must be in range");

class VertexAttributeArray;

class VertexAttribute final {
public:
    using ElementType = std::variant<int8_t, int16_t, int32_t, float, matf2, matf4>;

    // Can only be created by VertexAttributeArray
private:
    friend VertexAttributeArray;
    VertexAttribute(AttributeDataType dataType_, std::size_t count_)
        : dataType(dataType_),
          items(count_) {
    }

public:
    AttributeDataType getDataType() const { return dataType; }

    std::size_t getCount() const { return items.size(); }

private:
    AttributeDataType dataType;
    std::vector<ElementType> items;
};

class VertexAttributeArray final {
public:
    VertexAttributeArray() = default;
    ~VertexAttributeArray() = default;
    
    VertexAttribute& add(AttributeDataType dataType, std::size_t count) {
        //attrs.emplace_back(std::make_unique<VertexAttribute>(dataType, count));
        attrs.emplace_back(std::unique_ptr<VertexAttribute>(new VertexAttribute(dataType, count)));
        return *attrs.back();
    }
private:
    
    std::vector<std::unique_ptr<VertexAttribute>> attrs;
};

} // namespace gfx
} // namespace mbgl
