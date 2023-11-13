#pragma once

#include <string>

namespace mbgl {
namespace style {

class Sprite {
public:
    Sprite(std::string id, std::string spriteURL);
    ~Sprite();

    std::string id;
    std::string spriteURL;
};

} // namespace style
} // namespace mbgl
