#include <mbgl/programs/program_parameters.hpp>
#include <mbgl/util/string.hpp>
#include <string_view>
#include <stdexcept>

namespace mbgl {

ProgramParameters::ProgramParameters(const float pixelRatio, const bool overdraw)
    : defines([&] {
          std::string result;
          result.reserve(32);
          result += "#define DEVICE_PIXEL_RATIO ";
          result += util::toString(pixelRatio, true);
          result += '\n';
          if (overdraw) {
              result += "#define OVERDRAW_INSPECTOR\n";
          }
          return result;
      }()) {}

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

const std::string& ProgramParameters::getDefines() const {
    return defines;
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
