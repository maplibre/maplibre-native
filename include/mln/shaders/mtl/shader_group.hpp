#pragma once

#include <mln/gfx/shader_group.hpp>
#include <mln/shaders/mtl/common.hpp>
#include <mln/shaders/mtl/shader_program.hpp>
#include <mln/shaders/program_parameters.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/util/hash.hpp>
#include <mln/util/containers.hpp>

#include <numeric>
#include <span>
#include <string>
#include <type_traits>

namespace mln {
namespace mtl {

class ShaderGroupBase : public gfx::ShaderGroup {
protected:
    ShaderGroupBase(const ProgramParameters& parameters_)
        : programParameters(parameters_) {}

    using DefinesMap = mln::unordered_map<std::string, std::string>;
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

// Views refer to static reflection arrays; accessors lazily load source. Each shader uses
// the same group implementation; only this metadata varies between built-ins.
struct ShaderInfo {
    shaders::BuiltIn id;
    const char* name;
    std::string_view (*prelude)();
    std::string_view (*source)();
    const char* vertexMain;
    const char* fragmentMain;
    std::span<const shaders::AttributeInfo> attributes;
    std::span<const shaders::AttributeInfo> instanceAttributes;
    std::span<const shaders::TextureInfo> textures;
};

class ShaderGroup final : public ShaderGroupBase {
public:
    ShaderGroup(const ProgramParameters& programParameters_, const ShaderInfo& info_)
        : ShaderGroupBase(programParameters_),
          info(info_) {}
    ~ShaderGroup() noexcept override = default;

    gfx::ShaderPtr getOrCreateShader(gfx::Context&,
                                     const StringIDSetsPair& propertiesAsUniforms,
                                     std::string_view firstAttribName) override;

private:
    const ShaderInfo info;
};

} // namespace mtl
} // namespace mln
