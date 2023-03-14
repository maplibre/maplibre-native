#pragma once

#include <string>

namespace mbgl {

class ProgramParameters {
public:
    ProgramParameters(float pixelRatio, bool overdraw);

    const std::string& getDefines() const;

private:
    std::string defines;
};

} // namespace mbgl
