#pragma once

#include <string>

namespace mbgl {
namespace style {

class Sprite {
public:
    Sprite(std::string, std::string);
    ~Sprite();

    std::string id;
    std::string spriteURL;
};

} // namespace style
} // namespace mbgl
