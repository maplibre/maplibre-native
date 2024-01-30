#include <mbgl/shaders/gl/shader_program_gl.hpp>

#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/types.hpp>
#include <mbgl/gl/vertex_attribute_gl.hpp>
#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/programs/program_parameters.hpp>
#include <mbgl/shaders/shader_manifest.hpp>

#include <mbgl/shaders/background_layer_ubo.hpp>
#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/collision_layer_ubo.hpp>
#include <mbgl/shaders/debug_layer_ubo.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>
#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>
#include <mbgl/shaders/heatmap_layer_ubo.hpp>
#include <mbgl/shaders/heatmap_texture_layer_ubo.hpp>
#include <mbgl/shaders/hillshade_layer_ubo.hpp>
#include <mbgl/shaders/hillshade_prepare_layer_ubo.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>
#include <mbgl/shaders/raster_layer_ubo.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

#include <cstring>
#include <utility>

namespace mbgl {
namespace shaders {

UniformBlockInfo::UniformBlockInfo(std::string_view name_, std::size_t id_)
    : name(name_),
      id(id_),
      binding(id_) {}

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::BackgroundShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"BackgroundDrawableUBO", idBackgroundDrawableUBO},
    UniformBlockInfo{"BackgroundLayerUBO", idBackgroundLayerUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"BackgroundDrawableUBO", idBackgroundDrawableUBO},
        UniformBlockInfo{"BackgroundLayerUBO", idBackgroundLayerUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::CircleShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"CircleDrawableUBO", idCircleDrawableUBO},
    UniformBlockInfo{"CirclePaintParamsUBO", idCirclePaintParamsUBO},
    UniformBlockInfo{"CircleEvaluatedPropsUBO", idCircleEvaluatedPropsUBO},
    UniformBlockInfo{"CircleInterpolateUBO", idCircleInterpolateUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::CollisionBoxShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"CollisionBoxUBO", idCollisionUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::CollisionCircleShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"CollisionCircleUBO", idCollisionUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::DebugShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"DebugUBO", idDebugUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::FillShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"FillDrawableUBO", idFillDrawableUBO},
    UniformBlockInfo{"FillEvaluatedPropsUBO", idFillEvaluatedPropsUBO},
    UniformBlockInfo{"FillInterpolateUBO", idFillInterpolateUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::FillOutlineShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"FillOutlineDrawableUBO", idFillOutlineDrawableUBO},
        UniformBlockInfo{"FillOutlineEvaluatedPropsUBO", idFillOutlineEvaluatedPropsUBO},
        UniformBlockInfo{"FillOutlineInterpolateUBO", idFillOutlineInterpolateUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::FillPatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"FillPatternDrawableUBO", idFillPatternDrawableUBO},
        UniformBlockInfo{"FillPatternTilePropsUBO", idFillPatternTilePropsUBO},
        UniformBlockInfo{"FillPatternEvaluatedPropsUBO", idFillPatternEvaluatedPropsUBO},
        UniformBlockInfo{"FillPatternInterpolateUBO", idFillPatternInterpolateUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"FillOutlinePatternDrawableUBO", idFillOutlinePatternDrawableUBO},
        UniformBlockInfo{"FillOutlinePatternTilePropsUBO", idFillOutlinePatternTilePropsUBO},
        UniformBlockInfo{"FillOutlinePatternEvaluatedPropsUBO", idFillOutlinePatternEvaluatedPropsUBO},
        UniformBlockInfo{"FillOutlinePatternInterpolateUBO", idFillOutlinePatternInterpolateUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::FillExtrusionShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"FillExtrusionDrawableUBO", idFillExtrusionDrawableUBO},
        UniformBlockInfo{"FillExtrusionDrawablePropsUBO", idFillExtrusionDrawablePropsUBO},
        UniformBlockInfo{"FillExtrusionDrawableTilePropsUBO", idFillExtrusionDrawableTilePropsUBO},
        UniformBlockInfo{"FillExtrusionInterpolateUBO", idFillExtrusionInterpolateUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"FillExtrusionDrawableUBO", idFillExtrusionDrawableUBO},
        UniformBlockInfo{"FillExtrusionDrawablePropsUBO", idFillExtrusionDrawablePropsUBO},
        UniformBlockInfo{"FillExtrusionDrawableTilePropsUBO", idFillExtrusionDrawableTilePropsUBO},
        UniformBlockInfo{"FillExtrusionInterpolateUBO", idFillExtrusionInterpolateUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::HeatmapShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"HeatmapDrawableUBO", idHeatmapDrawableUBO},
    UniformBlockInfo{"HeatmapEvaluatedPropsUBO", idHeatmapEvaluatedPropsUBO},
    UniformBlockInfo{"HeatmapInterpolateUBO", idHeatmapInterpolateUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"HeatmapTextureDrawableUBO", idHeatmapTextureDrawableUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"HillshadePrepareDrawableUBO", idHillshadePrepareDrawableUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::HillshadeShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"HillshadeDrawableUBO", idHillshadeDrawableUBO},
    UniformBlockInfo{"HillshadeEvaluatedPropsUBO", idHillshadeEvaluatedPropsUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineGradientShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO},
        UniformBlockInfo{"LineGradientUBO", idLineGradientUBO},
        UniformBlockInfo{"LineGradientPropertiesUBO", idLineGradientPropertiesUBO},
        UniformBlockInfo{"LineGradientInterpolationUBO", idLineGradientInterpolationUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LinePatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO},
        UniformBlockInfo{"LinePatternUBO", idLinePatternUBO},
        UniformBlockInfo{"LinePatternPropertiesUBO", idLinePatternPropertiesUBO},
        UniformBlockInfo{"LinePatternInterpolationUBO", idLinePatternInterpolationUBO},
        UniformBlockInfo{"LinePatternTilePropertiesUBO", idLinePatternTilePropertiesUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineSDFShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO},
    UniformBlockInfo{"LineSDFUBO", idLineSDFUBO},
    UniformBlockInfo{"LineSDFPropertiesUBO", idLineSDFPropertiesUBO},
    UniformBlockInfo{"LineSDFInterpolationUBO", idLineSDFInterpolationUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO},
    UniformBlockInfo{"LineUBO", idLineUBO},
    UniformBlockInfo{"LinePropertiesUBO", idLinePropertiesUBO},
    UniformBlockInfo{"LineInterpolationUBO", idLineInterpolationUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineBasicShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"LineBasicUBO", idLineBasicUBO},
    UniformBlockInfo{"LineBasicPropertiesUBO", idLineBasicPropertiesUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::RasterShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"RasterDrawableUBO", idRasterDrawableUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::SymbolIconShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO},
    UniformBlockInfo{"SymbolDynamicUBO", idSymbolDynamicUBO},
    UniformBlockInfo{"SymbolDrawablePaintUBO", idSymbolDrawablePaintUBO},
    UniformBlockInfo{"SymbolDrawableTilePropsUBO", idSymbolDrawableTilePropsUBO},
    UniformBlockInfo{"SymbolDrawableInterpolateUBO", idSymbolDrawableInterpolateUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO},
        UniformBlockInfo{"SymbolDynamicUBO", idSymbolDynamicUBO},
        UniformBlockInfo{"SymbolDrawablePaintUBO", idSymbolDrawablePaintUBO},
        UniformBlockInfo{"SymbolDrawableTilePropsUBO", idSymbolDrawableTilePropsUBO},
        UniformBlockInfo{"SymbolDrawableInterpolateUBO", idSymbolDrawableInterpolateUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO},
        UniformBlockInfo{"SymbolDynamicUBO", idSymbolDynamicUBO},
        UniformBlockInfo{"SymbolDrawablePaintUBO", idSymbolDrawablePaintUBO},
        UniformBlockInfo{"SymbolDrawableTilePropsUBO", idSymbolDrawableTilePropsUBO},
        UniformBlockInfo{"SymbolDrawableInterpolateUBO", idSymbolDrawableInterpolateUBO},
};

} // namespace shaders

namespace gl {

namespace {

gfx::AttributeDataType mapType(platform::GLenum attrType) {
    using T = gfx::AttributeDataType;
    switch (attrType) {
        case GL_FLOAT:
            return T::Float;
        case GL_FLOAT_VEC2:
            return T::Float2;
        case GL_FLOAT_VEC3:
            return T::Float3;
        case GL_FLOAT_VEC4:
            return T::Float4;
        case GL_FLOAT_MAT2:
            return T::Float4; // does that work ?
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT4:
            return T::Float4;
        case GL_INT:
            return T::Int;
        case GL_INT_VEC2:
            return T::Int2;
        case GL_INT_VEC3:
            return T::Int3;
        case GL_INT_VEC4:
            return T::Int4;
        case GL_UNSIGNED_INT:
            return T::UInt;
        // ES3 stuff that isn't defined yet:
        // case GL_FLOAT_MAT2x3:
        // case GL_FLOAT_MAT2x4:
        // case GL_FLOAT_MAT3x2:
        // case GL_FLOAT_MAT3x4:
        // case GL_FLOAT_MAT4x2:
        // case GL_FLOAT_MAT4x3:       return T::Invalid;
        // case GL_UNSIGNED_INT_VEC2:  return T::UInt2;
        // case GL_UNSIGNED_INT_VEC3:  return T::UInt3;
        // case GL_UNSIGNED_INT_VEC4:  return T::UInt4;
        // case GL_DOUBLE:             return T::Float;
        // case GL_DOUBLE_VEC2:        return T::Float2;
        // case GL_DOUBLE_VEC3:        return T::Float3;
        // case GL_DOUBLE_VEC4:        return T::Float4;
        // case GL_DOUBLE_MAT2:        return T::Float4;
        // case GL_DOUBLE_MAT3:
        // case GL_DOUBLE_MAT4:
        // case GL_DOUBLE_MAT2x3:
        // case GL_DOUBLE_MAT2x4:
        // case GL_DOUBLE_MAT3x2:
        // case GL_DOUBLE_MAT3x4:
        // case GL_DOUBLE_MAT4x2:
        // case GL_DOUBLE_MAT4x3:
        default:
            return T::Invalid;
    }
}

using namespace platform;

void addAttr(
    VertexAttributeArrayGL& attrs, const StringIdentity id, GLint index, GLsizei length, GLint count, GLenum glType) {
    const auto elementType = mapType(glType);
    if (elementType != gfx::AttributeDataType::Invalid && length > 0) {
        if (const auto& newAttr = attrs.add(id, index, elementType, count)) {
            const auto& glAttr = static_cast<VertexAttributeGL*>(newAttr.get());
            glAttr->setGLType(glType);
        }
    }
}

} // namespace

ShaderProgramGL::ShaderProgramGL(UniqueProgram&& glProgram_)
    : ShaderProgramBase(),
      glProgram(std::move(glProgram_)) {}

ShaderProgramGL::ShaderProgramGL(UniqueProgram&& program,
                                 UniformBlockArrayGL&& uniformBlocks_,
                                 VertexAttributeArrayGL&& attributes_,
                                 SamplerLocationMap&& samplerLocations_)
    : ShaderProgramBase(),
      glProgram(std::move(program)),
      uniformBlocks(std::move(uniformBlocks_)),
      vertexAttributes(std::move(attributes_)),
      samplerLocations(std::move(samplerLocations_)) {}

ShaderProgramGL::ShaderProgramGL(ShaderProgramGL&& other)
    : ShaderProgramBase(std::forward<ShaderProgramBase&&>(other)),
      glProgram(std::move(other.glProgram)),
      uniformBlocks(std::move(other.uniformBlocks)),
      vertexAttributes(std::move(other.vertexAttributes)),
      samplerLocations(std::move(other.samplerLocations)) {}

std::optional<uint32_t> ShaderProgramGL::getSamplerLocation(const StringIdentity id) const {
    std::optional<uint32_t> result{};
    if (auto it = samplerLocations.find(id); it != samplerLocations.end()) {
        result = it->second;
    }
    return result;
}

std::shared_ptr<ShaderProgramGL> ShaderProgramGL::create(
    Context& context,
    const ProgramParameters& programParameters,
    const std::string& /*name*/,
    const std::string_view firstAttribName,
    const std::vector<shaders::UniformBlockInfo>& uniformBlocksInfo,
    const std::string& vertexSource,
    const std::string& fragmentSource,
    const std::string& additionalDefines) noexcept(false) {
    // throws on compile error
    auto vertProg = context.createShader(
        ShaderType::Vertex,
        std::initializer_list<const char*>{
            "#version 300 es\n",
            programParameters.getDefinesString().c_str(),
            additionalDefines.c_str(),
            shaders::ShaderSource<shaders::BuiltIn::Prelude, gfx::Backend::Type::OpenGL>::vertex,
            vertexSource.c_str()});
    auto fragProg = context.createShader(
        ShaderType::Fragment,
        {"#version 300 es\n",
         programParameters.getDefinesString().c_str(),
         additionalDefines.c_str(),
         shaders::ShaderSource<shaders::BuiltIn::Prelude, gfx::Backend::Type::OpenGL>::fragment,
         fragmentSource.c_str()});
    auto program = context.createProgram(vertProg, fragProg, firstAttribName.data());

    // GLES3.1
    // GLint numAttribs;
    // glGetProgramInterfaceiv(program, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numAttribs);

    UniformBlockArrayGL uniformBlocks;

    GLint count = 0;
    GLint maxLength = 0;
    MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &count));
    MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &maxLength));

    for (const auto& blockInfo : uniformBlocksInfo) {
        GLint index = MBGL_CHECK_ERROR(glGetUniformBlockIndex(program, blockInfo.name.data()));
        GLint size = 0;
        MBGL_CHECK_ERROR(glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_DATA_SIZE, &size));
        assert(size > 0);
        GLint binding = GLint(blockInfo.binding);
        MBGL_CHECK_ERROR(glUniformBlockBinding(program, index, binding));
        uniformBlocks.add(blockInfo.id, binding, size);
    }

    SamplerLocationMap samplerLocations;
    GLint numActiveUniforms = 0;
    MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numActiveUniforms));
    MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength));
    auto name = std::vector<GLchar>(maxLength);
    for (GLint index = 0; index < numActiveUniforms; ++index) {
        GLsizei actualLength = 0;
        GLint size = 0;
        GLenum type = GL_ZERO;

        MBGL_CHECK_ERROR(glGetActiveUniform(program, index, maxLength, &actualLength, &size, &type, name.data()));

        if (type == GL_SAMPLER_2D) {
            // This uniform is a texture sampler
            GLint location = MBGL_CHECK_ERROR(glGetUniformLocation(program, name.data()));
            assert(location != -1);
            if (location != -1) {
                samplerLocations[stringIndexer().get(name.data())] = location;
            }
        }
    }

    VertexAttributeArrayGL attrs;

    count = 0;
    MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count));
    MBGL_CHECK_ERROR(glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength));
    name.resize(maxLength);
    for (GLint index = 0; index < count; ++index) {
        GLsizei length = 0; // "number of characters actually written in name (excluding the null terminator)"
        GLint size = 0;     // "size of the attribute variable, in units of the type returned in type"
        GLenum glType = 0;
        MBGL_CHECK_ERROR(glGetActiveAttrib(program, index, maxLength, &length, &size, &glType, name.data()));
        if (!strncmp(name.data(), "gl_", 3)) { // Is there a better way to detect built-in attributes?
            continue;
        }
        const GLint location = MBGL_CHECK_ERROR(glGetAttribLocation(program, name.data()));
        addAttr(attrs, stringIndexer().get(name.data()), location, length, size, glType);
    }

    return std::make_shared<ShaderProgramGL>(
        std::move(program), std::move(uniformBlocks), std::move(attrs), std::move(samplerLocations));
}

} // namespace gl
} // namespace mbgl
