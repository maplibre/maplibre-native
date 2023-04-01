#include <mbgl/programs/program_parameters.hpp>
#include <mbgl/util/string.hpp>
#include <string_view>

namespace mbgl {

ProgramParameters::ProgramParameters(const float pixelRatio,
                                     const bool overdraw)
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
      }())
{}

ProgramParameters ProgramParameters::withShaderSource(std::string_view vertexSource,
    std::string_view fragmentSource) const noexcept
{
    ProgramParameters params = *this;
    params.vertexSource_ = vertexSource;
    params.fragmentSource_ = fragmentSource;
    return params;
}

const std::string& ProgramParameters::getDefines() const {
    return defines;
}

const std::string& ProgramParameters::vertexSource() const noexcept {
    return vertexSource_;
}

const std::string& ProgramParameters::fragmentSource() const noexcept {
    return fragmentSource_;
}

} // namespace mbgl
