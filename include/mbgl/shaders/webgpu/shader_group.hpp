#pragma once

#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/webgpu/common.hpp>
#include <mbgl/shaders/program_parameters.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/hash.hpp>
#include <mbgl/util/containers.hpp>

#include <cctype>
#include <numeric>
#include <sstream>
#include <stack>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace mbgl {
namespace webgpu {

namespace detail {

inline bool isDirective(const std::string& line, const std::string& directive) {
    std::size_t pos = 0;
    while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos]))) {
        ++pos;
    }
    if (pos >= line.size() || line[pos] != '#') {
        return false;
    }
    ++pos;
    while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos]))) {
        ++pos;
    }
    if (line.compare(pos, directive.size(), directive) != 0) {
        return false;
    }
    pos += directive.size();
    return pos == line.size() || std::isspace(static_cast<unsigned char>(line[pos]));
}

inline std::string getDirectiveArgument(const std::string& line) {
    auto hash = line.find('#');
    if (hash == std::string::npos) {
        return {};
    }
    auto pos = line.find_first_not_of(" \t", hash + 1);
    pos = line.find_first_of(" \t", pos);
    if (pos == std::string::npos) {
        return {};
    }
    pos = line.find_first_not_of(" \t", pos);
    if (pos == std::string::npos) {
        return {};
    }
    const auto end = line.find_first_of(" \t\r\n", pos);
    return line.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
}

inline std::string preprocessWGSL(const std::string& source,
                                  const std::unordered_map<std::string, bool>& defines) {
    std::stringstream input(source);
    std::string line;
    std::string output;
    output.reserve(source.size());

    struct ConditionalFrame {
        bool parentActive;
        bool condition;
        bool elseSeen;
    };

    std::stack<ConditionalFrame> stack;
    bool currentActive = true;

    while (std::getline(input, line)) {
        if (isDirective(line, "ifdef")) {
            const auto symbol = getDirectiveArgument(line);
            const bool condition = defines.contains(symbol);
            stack.push(ConditionalFrame{currentActive, condition, false});
            currentActive = currentActive && condition;
            continue;
        }

        if (isDirective(line, "ifndef")) {
            const auto symbol = getDirectiveArgument(line);
            const bool condition = !defines.contains(symbol);
            stack.push(ConditionalFrame{currentActive, condition, false});
            currentActive = currentActive && condition;
            continue;
        }

        if (isDirective(line, "else")) {
            if (stack.empty()) {
                continue;
            }
            auto& frame = stack.top();
            if (!frame.elseSeen) {
                frame.elseSeen = true;
                currentActive = frame.parentActive && !frame.condition;
            } else {
                currentActive = false;
            }
            continue;
        }

        if (isDirective(line, "endif")) {
            if (stack.empty()) {
                continue;
            }
            currentActive = stack.top().parentActive;
            stack.pop();
            continue;
        }

        if (currentActive) {
            output.append(line);
            output.push_back('\n');
        }
    }

    return output;
}

} // namespace detail

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
            DefinesMap additionalDefines;
            addAdditionalDefines(propertiesAsUniforms, additionalDefines);

            std::string vertexSource;
            std::string fragmentSource;

            if constexpr (requires { ShaderSource::prelude; }) {
                vertexSource = std::string(ShaderSource::prelude) + vert;
                fragmentSource = std::string(ShaderSource::prelude) + frag;
            } else {
                vertexSource = vert;
                fragmentSource = frag;
            }

            std::unordered_map<std::string, bool> defineSet;
            const auto& programDefines = programParameters.getDefines();
            for (const auto& entry : programDefines) {
                defineSet.emplace(entry.first, true);
            }
            for (const auto& entry : additionalDefines) {
                defineSet.emplace(entry.first, true);
            }

            vertexSource = detail::preprocessWGSL(vertexSource, defineSet);
            fragmentSource = detail::preprocessWGSL(fragmentSource, defineSet);

            auto& context = static_cast<Context&>(gfxContext);
            shader = std::make_shared<ShaderProgram>(context,
                                                     vertexSource,
                                                     fragmentSource,
                                                     ShaderSource::attributes,
                                                     ShaderSource::textures,
                                                     std::string(name));

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
