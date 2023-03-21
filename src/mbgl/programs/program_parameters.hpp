#pragma once

#include <string>

namespace mbgl {

class ProgramParameters {
public:
    ProgramParameters(float pixelRatio, bool overdraw);

    ProgramParameters withShaderSource(std::string_view vertexSource,
        std::string_view fragmentSource) const noexcept;

    const std::string& getDefines() const;

    std::string_view vertexSource() const noexcept;
    std::string_view fragmentSource() const noexcept;

private:
    std::string defines;
    std::string vertexSource_{""};
    std::string fragmentSource_{""};
};

} // namespace mbgl
