#include <mbgl/mtl/vertex_attribute.hpp>

#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/mtl/buffer_resource.hpp>
#include <mbgl/mtl/upload_pass.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/convert.hpp>

#include <cstring>
#include <sstream>

namespace mbgl {
namespace mtl {

namespace {

class Converter {
public:
    Converter(gfx::AttributeDataType dataType_, std::uint8_t* buffer_, std::size_t stride_)
    : dataType(dataType_), buffer(buffer_), stride(stride_)
    {}
    
    void operator()(std::int32_t value) {
        switch (dataType) {
            case gfx::AttributeDataType::Byte:
                get<int8_t>(value);
                break;
            case gfx::AttributeDataType::UByte:
                get<uint8_t>(value);
                break;
            case gfx::AttributeDataType::Short:
                get<int16_t>(value);
                break;
            case gfx::AttributeDataType::UShort:
                get<uint16_t>(value);
                break;
            case gfx::AttributeDataType::Int:
                get<int32_t>(value);
                break;
            case gfx::AttributeDataType::UInt:
                get<uint32_t>(value);
                break;
            case gfx::AttributeDataType::Float:
                get<float>(value);
                break;
            default:
                assert(false);
                buffer += stride;
                break;
        }
    }
    
    void operator()(VertexAttribute::int2 value) {
        switch (dataType) {
            case gfx::AttributeDataType::Byte2:
                get<int8_t>(value);
                break;
            case gfx::AttributeDataType::UByte2:
                get<uint8_t>(value);
                break;
            case gfx::AttributeDataType::Short2:
                get<int16_t>(value);
                break;
            case gfx::AttributeDataType::UShort2:
                get<uint16_t>(value);
                break;
            case gfx::AttributeDataType::Int2:
                get<int32_t>(value);
                break;
            case gfx::AttributeDataType::UInt2:
                get<uint32_t>(value);
                break;
            case gfx::AttributeDataType::Float2:
                get<float>(value);
                break;
            default:
                assert(false);
                buffer += stride;
                break;
        }
    }
    
    void operator()(VertexAttribute::int3 value) {
        switch (dataType) {
            case gfx::AttributeDataType::Byte3:
                get<int8_t>(value);
                break;
            case gfx::AttributeDataType::UByte3:
                get<uint8_t>(value);
                break;
            case gfx::AttributeDataType::Short3:
                get<int16_t>(value);
                break;
            case gfx::AttributeDataType::UShort3:
                get<uint16_t>(value);
                break;
            case gfx::AttributeDataType::Int3:
                get<int32_t>(value);
                break;
            case gfx::AttributeDataType::UInt3:
                get<uint32_t>(value);
                break;
            case gfx::AttributeDataType::Float3:
                get<float>(value);
                break;
            default:
                assert(false);
                buffer += stride;
                break;
        }
    }
    
    void operator()(VertexAttribute::int4 value) {
        switch (dataType) {
            case gfx::AttributeDataType::Byte4:
                get<int8_t>(value);
                break;
            case gfx::AttributeDataType::UByte4:
                get<uint8_t>(value);
                break;
            case gfx::AttributeDataType::Short4:
                get<int16_t>(value);
                break;
            case gfx::AttributeDataType::UShort4:
                get<uint16_t>(value);
                break;
            case gfx::AttributeDataType::Int4:
                get<int32_t>(value);
                break;
            case gfx::AttributeDataType::UInt4:
                get<uint32_t>(value);
                break;
            case gfx::AttributeDataType::Float4:
                get<float>(value);
                break;
            default:
                assert(false);
                buffer += stride;
                break;
        }
    }
    
    void operator()(float value) {
        switch (dataType) {
            case gfx::AttributeDataType::Byte:
                get<int8_t>(value);
                break;
            case gfx::AttributeDataType::UByte:
                get<uint8_t>(value);
                break;
            case gfx::AttributeDataType::Short:
                get<int16_t>(value);
                break;
            case gfx::AttributeDataType::UShort:
                get<uint16_t>(value);
                break;
            case gfx::AttributeDataType::Int:
                get<int32_t>(value);
                break;
            case gfx::AttributeDataType::UInt:
                get<uint32_t>(value);
                break;
            case gfx::AttributeDataType::Float:
                get<float>(value);
                break;
            default:
                assert(false);
                buffer += stride;
                break;
        }
    }
    
    void operator()(VertexAttribute::float2 value) {
        switch (dataType) {
            case gfx::AttributeDataType::Byte2:
                get<int8_t>(value);
                break;
            case gfx::AttributeDataType::UByte2:
                get<uint8_t>(value);
                break;
            case gfx::AttributeDataType::Short2:
                get<int16_t>(value);
                break;
            case gfx::AttributeDataType::UShort2:
                get<uint16_t>(value);
                break;
            case gfx::AttributeDataType::Int2:
                get<int32_t>(value);
                break;
            case gfx::AttributeDataType::UInt2:
                get<uint32_t>(value);
                break;
            case gfx::AttributeDataType::Float2:
                get<float>(value);
                break;
            default:
                assert(false);
                buffer += stride;
                break;
        }
    }
    
    void operator()(VertexAttribute::float3 value) {
        switch (dataType) {
            case gfx::AttributeDataType::Byte3:
                get<int8_t>(value);
                break;
            case gfx::AttributeDataType::UByte3:
                get<uint8_t>(value);
                break;
            case gfx::AttributeDataType::Short3:
                get<int16_t>(value);
                break;
            case gfx::AttributeDataType::UShort3:
                get<uint16_t>(value);
                break;
            case gfx::AttributeDataType::Int3:
                get<int32_t>(value);
                break;
            case gfx::AttributeDataType::UInt3:
                get<uint32_t>(value);
                break;
            case gfx::AttributeDataType::Float3:
                get<float>(value);
                break;
            default:
                assert(false);
                buffer += stride;
                break;
        }
    }
    
    void operator()(VertexAttribute::float4 value) {
        switch (dataType) {
            case gfx::AttributeDataType::Byte4:
                get<int8_t>(value);
                break;
            case gfx::AttributeDataType::UByte4:
                get<uint8_t>(value);
                break;
            case gfx::AttributeDataType::Short4:
                get<int16_t>(value);
                break;
            case gfx::AttributeDataType::UShort4:
                get<uint16_t>(value);
                break;
            case gfx::AttributeDataType::Int4:
                get<int32_t>(value);
                break;
            case gfx::AttributeDataType::UInt4:
                get<uint32_t>(value);
                break;
            case gfx::AttributeDataType::Float4:
                get<float>(value);
                break;
            default:
                assert(false);
                buffer += stride;
                break;
        }
    }
    
    void operator()(VertexAttribute::matf3) { assert(false); buffer += stride; }
    void operator()(VertexAttribute::matf4) { assert(false); buffer += stride; }
    void operator()(VertexAttribute::ushort8) { assert(false); buffer += stride; }
    
private:
    gfx::AttributeDataType dataType;
    std::uint8_t* buffer{nullptr};
    std::size_t stride{0};
    
    template <typename To, typename From, typename = std::enable_if_t<std::is_assignable_v<To&, From>>>
    void get(const From value) {
        assert(stride == sizeof(To));
        *reinterpret_cast<To*>(buffer) = static_cast<To>(value);
        buffer += stride;
    }
    
    template <typename To, typename From, std::size_t Size, typename = std::enable_if_t<std::is_assignable_v<To&, From>>>
    void get(const std::array<From, Size>& value) {
        assert(stride == sizeof(std::array<To, Size>));
        std::transform(std::begin(value), std::end(value), std::begin(*reinterpret_cast<std::array<To, Size>*>(buffer)), [](From x) { return static_cast<To>(x); });
        buffer += stride;
    }
};

} // namespace

const gfx::UniqueVertexBufferResource& VertexAttribute::getBuffer(gfx::VertexAttribute& attrib_,
                                                                  UploadPass& uploadPass,
                                                                  const gfx::BufferUsageType usage) {
    if (!attrib_.getBuffer()) {
        auto& attrib = static_cast<VertexAttribute&>(attrib_);
        if (attrib.sharedRawData) {
            return uploadPass.getBuffer(attrib.sharedRawData, usage);
        } else {
            if (attrib.rawData.empty()) {
                // vertex building
                auto& rawData = attrib.getRawData();
                const auto count = attrib.getCount();
                const auto stride = attrib.getStride();
                assert(stride != 0);
                
                rawData.resize(count * stride);
                std::fill(rawData.begin(), rawData.end(), 0);

                Converter converter(attrib.getDataType(), rawData.data(), stride);
                for (std::size_t i = 0; i < count; ++i) {
                    std::visit(converter, attrib.get(i));
                }
            }
            if (!attrib.rawData.empty()) {
                auto buffer = uploadPass.createVertexBufferResource(
                    attrib.rawData.data(), attrib.rawData.size(), usage, false);
                attrib.setBuffer(std::move(buffer));
                attrib.setRawData({});
            } else {
                assert(false);
            }
        }
    }
    return attrib_.getBuffer();
}

} // namespace mtl
} // namespace mbgl
