#include <mbgl/style/sprite.hpp>

namespace mbgl {
namespace style {

Sprite::~Sprite() = default;

Sprite::Sprite(std::string id, std::string spriteURL) {
    this->id = id;
    this->spriteURL = spriteURL;
}

} // namespace style
} // namespace mbgl
