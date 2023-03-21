#include <mbgl/programs/program_parameters.hpp>
#include <mbgl/util/string.hpp>

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

ProgramParameters ProgramParameters::withShaderSource(std::string_view vertexSource_,
    std::string_view fragmentSource_) const noexcept
{
    ProgramParameters params = *this;
    params.vertexSource_ = vertexSource_;
    params.fragmentSource_ = fragmentSource_;
    return std::move(params);
}

const std::string& ProgramParameters::getDefines() const {
    return defines;
}

std::string_view ProgramParameters::vertexSource() const noexcept {
    return vertexSource_;
}

std::string_view ProgramParameters::fragmentSource() const noexcept {
    return fragmentSource_;
}

} // namespace mbgl
