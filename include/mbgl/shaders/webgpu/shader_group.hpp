#pragma once

#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/webgpu/common.hpp>
#include <mbgl/shaders/program_parameters.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/hash.hpp>
#include <mbgl/util/containers.hpp>

#include <numeric>
#include <string>
#include <type_traits>
#include <unordered_set>

namespace mbgl {
namespace webgpu {

class ShaderGroupBase : public gfx::ShaderGroup {
protected:
    ShaderGroupBase(const ProgramParameters& parameters_)
        : programParameters(parameters_) {}

    using DefinesMap = mbgl::unordered_map<std::string, std::string>;
    void addAdditionalDefines(const StringIDSetsPair& propertiesAsUniforms, DefinesMap& additionalDefines) {
        additionalDefines.reserve(propertiesAsUniforms.first.size());
        for (const auto name : propertiesAsUniforms.first) {
            // We expect the names to be prefixed by "a_", but we need just the base here.
            const auto* base = (name[0] == 'a' && name[1] == '_') ? &name[2] : name.data();
            additionalDefines.insert(std::make_pair(std::string(uniformPrefix) + base, std::string()));
        }
    }

    ProgramParameters programParameters;

private:
    static constexpr auto uniformPrefix = "HAS_UNIFORM_u_";
};

template <shaders::BuiltIn ShaderID>
class ShaderGroup final : public ShaderGroupBase {
public:
    ShaderGroup(const ProgramParameters& programParameters_)
        : ShaderGroupBase(programParameters_) {}
    ~ShaderGroup() noexcept override = default;

    gfx::ShaderPtr getOrCreateShader(gfx::Context& gfxContext,
                                     const StringIDSetsPair& propertiesAsUniforms,
                                     std::string_view /*firstAttribName*/) override {
        using ShaderSource = shaders::ShaderSource<ShaderID, gfx::Backend::Type::WebGPU>;
        constexpr auto& name = ShaderSource::name;
        constexpr auto& vert = ShaderSource::vertex;
        constexpr auto& frag = ShaderSource::fragment;

        std::size_t seed = 0;
        mbgl::util::hash_combine(seed, propertyHash(propertiesAsUniforms));
        mbgl::util::hash_combine(seed, programParameters.getDefinesHash());
        const std::string shaderName = getShaderName(name, seed);

        auto shader = get<webgpu::ShaderProgram>(shaderName);
        if (!shader) {
            // For WebGPU, we don't need to add defines to the shader source
            // since WGSL doesn't support preprocessor directives
            // Instead, we'll handle data-driven properties with uniform buffers

        std::string vertexSource;
        std::string fragmentSource;

        if constexpr (requires { ShaderSource::prelude; }) {
            vertexSource = std::string(ShaderSource::prelude) + vert;
            fragmentSource = std::string(ShaderSource::prelude) + frag;
        } else {
            vertexSource = vert;
            fragmentSource = frag;
        }

        auto replaceAll = [](std::string& target, const std::string& from, const std::string& to) {
            if (from.empty()) {
                return;
            }
            std::size_t pos = 0;
            while ((pos = target.find(from, pos)) != std::string::npos) {
                target.replace(pos, from.length(), to);
                pos += to.length();
            }
        };

        std::unordered_set<std::string> uniformBaseNames;
        uniformBaseNames.reserve(propertiesAsUniforms.first.size());
        for (const auto uniformName : propertiesAsUniforms.first) {
            if (!uniformName.empty()) {
                std::string base{uniformName};
                if (base.size() > 2 && base[0] == 'a' && base[1] == '_') {
                    base.erase(0, 2);
                }
                uniformBaseNames.insert(base);
            }
        }

        const auto adjustSymbolShader = [&](const std::string& shaderNameStr,
                                            std::string& vertSrc,
                                            std::string& fragSrc) {
            if (uniformBaseNames.empty()) {
                return;
            }

            struct PropertyTransform {
                const char* baseName;
                const char* vertexInputLine;
                const char* vertexOutputLine;
                const char* vertexAssignLine;
                const char* fragmentInputLine;
                const char* fragmentSelectBlock;
                const char* fragmentUniformLine;
            };

            const auto removeProperty = [&](const PropertyTransform& transform) {
                replaceAll(vertSrc, transform.vertexInputLine, "");
                replaceAll(vertSrc, transform.vertexOutputLine, "");
                replaceAll(vertSrc, transform.vertexAssignLine, "");
                replaceAll(fragSrc, transform.fragmentInputLine, "");
                replaceAll(fragSrc, transform.fragmentSelectBlock, transform.fragmentUniformLine);
            };

            if (shaderNameStr == "SymbolSDFShader") {
                static const PropertyTransform transforms[] = {
                    {"fill_color",
                     "    @location(8) fill_color: vec4<f32>,\n",
                     "    @location(4) fill_color: vec4<f32>,\n",
                     "    out.fill_color = unpack_mix_color(in.fill_color, drawable.fill_color_t);\n",
                     "    @location(4) fill_color: vec4<f32>,\n",
                     "    let fill_color = select(in.fill_color,\n                           select(props.icon_fill_color, props.text_fill_color, tileProps.is_text != 0u),\n                           false);\n",
                     "    let fill_color = select(props.icon_fill_color, props.text_fill_color, tileProps.is_text != 0u);\n"},
                    {"halo_color",
                     "    @location(9) halo_color: vec4<f32>,\n",
                     "    @location(5) halo_color: vec4<f32>,\n",
                     "    out.halo_color = unpack_mix_color(in.halo_color, drawable.halo_color_t);\n",
                     "    @location(5) halo_color: vec4<f32>,\n",
                     "    let halo_color = select(in.halo_color,\n                           select(props.icon_halo_color, props.text_halo_color, tileProps.is_text != 0u),\n                           false);\n",
                     "    let halo_color = select(props.icon_halo_color, props.text_halo_color, tileProps.is_text != 0u);\n"},
                    {"opacity",
                     "    @location(10) opacity: f32,\n",
                     "    @location(6) opacity: f32,\n",
                     "    out.opacity = unpack_mix_float(vec2<f32>(in.opacity, in.opacity), drawable.opacity_t);\n",
                     "    @location(6) opacity: f32,\n",
                     "    let opacity = select(in.opacity,\n                        select(props.icon_opacity, props.text_opacity, tileProps.is_text != 0u),\n                        false);\n",
                     "    let opacity = select(props.icon_opacity, props.text_opacity, tileProps.is_text != 0u);\n"},
                    {"halo_width",
                     "    @location(11) halo_width: f32,\n",
                     "    @location(7) halo_width: f32,\n",
                     "    out.halo_width = unpack_mix_float(vec2<f32>(in.halo_width, in.halo_width), drawable.halo_width_t);\n",
                     "    @location(7) halo_width: f32,\n",
                     "    let halo_width = select(in.halo_width,\n                           select(props.icon_halo_width, props.text_halo_width, tileProps.is_text != 0u),\n                           false);\n",
                     "    let halo_width = select(props.icon_halo_width, props.text_halo_width, tileProps.is_text != 0u);\n"},
                    {"halo_blur",
                     "    @location(12) halo_blur: f32,\n",
                     "    @location(8) halo_blur: f32,\n",
                     "    out.halo_blur = unpack_mix_float(vec2<f32>(in.halo_blur, in.halo_blur), drawable.halo_blur_t);\n",
                     "    @location(8) halo_blur: f32,\n",
                     "    let halo_blur = select(in.halo_blur,\n                          select(props.icon_halo_blur, props.text_halo_blur, tileProps.is_text != 0u),\n                          false);\n",
                     "    let halo_blur = select(props.icon_halo_blur, props.text_halo_blur, tileProps.is_text != 0u);\n"},
                };

                for (const auto& transform : transforms) {
                    if (uniformBaseNames.count(transform.baseName)) {
                        removeProperty(transform);
                    }
                }
            } else if (shaderNameStr == "SymbolTextAndIconShader") {
                static const PropertyTransform transforms[] = {
                    {"fill_color",
                     "    @location(7) fill_color: vec4<f32>,\n",
                     "    @location(5) fill_color: vec4<f32>,\n",
                     "    out.fill_color = unpack_mix_color(in.fill_color, drawable.fill_color_t);\n",
                     "    @location(5) fill_color: vec4<f32>,\n",
                     "    let fill_color = select(in.fill_color,\n                           select(props.icon_fill_color, props.text_fill_color, tileProps.is_text != 0u),\n                           false);\n",
                     "    let fill_color = select(props.icon_fill_color, props.text_fill_color, tileProps.is_text != 0u);\n"},
                    {"halo_color",
                     "    @location(8) halo_color: vec4<f32>,\n",
                     "    @location(6) halo_color: vec4<f32>,\n",
                     "    out.halo_color = unpack_mix_color(in.halo_color, drawable.halo_color_t);\n",
                     "    @location(6) halo_color: vec4<f32>,\n",
                     "    let halo_color = select(in.halo_color,\n                           select(props.icon_halo_color, props.text_halo_color, tileProps.is_text != 0u),\n                           false);\n",
                     "    let halo_color = select(props.icon_halo_color, props.text_halo_color, tileProps.is_text != 0u);\n"},
                    {"opacity",
                     "    @location(9) opacity: f32,\n",
                     "    @location(7) opacity: f32,\n",
                     "    out.opacity = unpack_mix_float(vec2<f32>(in.opacity, in.opacity), drawable.opacity_t);\n",
                     "    @location(7) opacity: f32,\n",
                     "    let opacity = select(in.opacity,\n                        select(props.icon_opacity, props.text_opacity, tileProps.is_text != 0u),\n                        false);\n",
                     "    let opacity = select(props.icon_opacity, props.text_opacity, tileProps.is_text != 0u);\n"},
                    {"halo_width",
                     "    @location(10) halo_width: f32,\n",
                     "    @location(8) halo_width: f32,\n",
                     "    out.halo_width = unpack_mix_float(vec2<f32>(in.halo_width, in.halo_width), drawable.halo_width_t);\n",
                     "    @location(8) halo_width: f32,\n",
                     "    let halo_width = select(in.halo_width,\n                           select(props.icon_halo_width, props.text_halo_width, tileProps.is_text != 0u),\n                           false);\n",
                     "    let halo_width = select(props.icon_halo_width, props.text_halo_width, tileProps.is_text != 0u);\n"},
                    {"halo_blur",
                     "    @location(11) halo_blur: f32,\n",
                     "    @location(9) halo_blur: f32,\n",
                     "    out.halo_blur = unpack_mix_float(vec2<f32>(in.halo_blur, in.halo_blur), drawable.halo_blur_t);\n",
                     "    @location(9) halo_blur: f32,\n",
                     "    let halo_blur = select(in.halo_blur,\n                          select(props.icon_halo_blur, props.text_halo_blur, tileProps.is_text != 0u),\n                          false);\n",
                     "    let halo_blur = select(props.icon_halo_blur, props.text_halo_blur, tileProps.is_text != 0u);\n"},
                };

                for (const auto& transform : transforms) {
                    if (uniformBaseNames.count(transform.baseName)) {
                        removeProperty(transform);
                    }
                }
            }
        };

        adjustSymbolShader(std::string(name), vertexSource, fragmentSource);

        auto& context = static_cast<Context&>(gfxContext);
        shader = std::make_shared<ShaderProgram>(
            context,
            vertexSource,
                fragmentSource,
                ShaderSource::attributes,
                ShaderSource::textures,
                std::string(name)  // Pass the shader name
            );

            assert(shader);
            if (!shader || !registerShader(shader, shaderName)) {
                assert(false);
                throw std::runtime_error("Failed to register " + shaderName + " with shader group!");
            }
        }

        return shader;
    }
};

} // namespace webgpu
} // namespace mbgl
