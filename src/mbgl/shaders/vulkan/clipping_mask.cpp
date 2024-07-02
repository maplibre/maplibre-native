#include <mbgl/shaders/vulkan/clipping_mask.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 1> ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Vulkan>::attributes =
    {AttributeInfo{0, gfx::AttributeDataType::Short2, idClippingMaskPosVertexAttribute}};

} // namespace shaders
} // namespace mbgl
