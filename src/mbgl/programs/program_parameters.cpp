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

ProgramParameters ProgramParameters::withShaderSource(
    const ProgramSource &source) const noexcept
{
    assert(gfx::Backend::Type::TYPE_MAX != source.backend);

    ProgramParameters params = *this;
    params.vertexSources[static_cast<size_t>(source.backend)]
        = source.vertex;
    params.fragmentSources[static_cast<size_t>(source.backend)]
        = source.fragment;
    return params;
}

const std::string& ProgramParameters::getDefines() const {
    return defines;
}

const std::string&
ProgramParameters::vertexSource(gfx::Backend::Type backend) const noexcept {
    assert(gfx::Backend::Type::TYPE_MAX != backend);
    return vertexSources[static_cast<size_t>(backend)];
}

const std::string&
ProgramParameters::fragmentSource(gfx::Backend::Type backend) const noexcept {
    assert(gfx::Backend::Type::TYPE_MAX != backend);
    return fragmentSources[static_cast<size_t>(backend)];
}

} // namespace mbgl
