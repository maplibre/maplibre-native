#include <mbgl/shaders/program_parameters.hpp>

#include <mbgl/util/hash.hpp>
#include <mbgl/util/string.hpp>

#include <string_view>
#include <stdexcept>

namespace mbgl {

ProgramParameters::ProgramParameters(const float pixelRatio, const bool overdraw)
    : defines(2),
      overdrawInspector(overdraw) {
    defines["DEVICE_PIXEL_RATIO"] = util::toString(pixelRatio, true);
    if (overdraw) {
        defines["OVERDRAW_INSPECTOR"] = std::string();
    }
    definesHash = util::hash(getDefinesString());
}

ProgramParameters ProgramParameters::withShaderSource(const ProgramSource& source) const noexcept {
    assert(gfx::Backend::Type::TYPE_MAX != source.backend);

    ProgramParameters params = *this;
    params.userSources[static_cast<size_t>(source.backend)] = source;
    return params;
}

ProgramParameters ProgramParameters::withDefaultSource(const ProgramSource& source) const noexcept {
    assert(gfx::Backend::Type::TYPE_MAX != source.backend);

    ProgramParameters params = *this;
    params.defaultSources[static_cast<size_t>(source.backend)] = source;
    return params;
}

ProgramParameters ProgramParameters::withProgramType(shaders::BuiltIn type) const noexcept {
    ProgramParameters params = *this;
    params.programType = type;
    return params;
}

const std::string& ProgramParameters::getDefinesString() const {
    if (definesStr.empty() && !defines.empty()) {
        definesStr.assign(defines.size() * 32, '\0');
        definesStr.clear();
        for (const auto& pair : defines) {
            definesStr.append("#define ").append(pair.first);
            if (!pair.second.empty()) {
                definesStr.append(" ").append(pair.second);
            }
            definesStr.append("\n");
        }
    }
    return definesStr;
}

const std::string& ProgramParameters::vertexSource(gfx::Backend::Type backend) const {
    assert(gfx::Backend::Type::TYPE_MAX != backend);

    if (userSources[static_cast<size_t>(backend)].vertex.length() > 0) {
        return userSources[static_cast<size_t>(backend)].vertex;
    } else if (defaultSources[static_cast<size_t>(backend)].vertex.length() > 0) {
        return defaultSources[static_cast<size_t>(backend)].vertex;
    } else {
        throw std::runtime_error("No vertex shader source provided for selected backend!");
    }
}

const std::string& ProgramParameters::fragmentSource(gfx::Backend::Type backend) const {
    assert(gfx::Backend::Type::TYPE_MAX != backend);

    if (userSources[static_cast<size_t>(backend)].fragment.length() > 0) {
        return userSources[static_cast<size_t>(backend)].fragment;
    } else if (defaultSources[static_cast<size_t>(backend)].fragment.length() > 0) {
        return defaultSources[static_cast<size_t>(backend)].fragment;
    } else {
        throw std::runtime_error("No fragment shader source provided for selected backend!");
    }
}

} // namespace mbgl
