#include <mbgl/style/sprite.hpp>

namespace mbgl {
namespace style {

Sprite::~Sprite() = default;

Sprite::Sprite(std::string id_, std::string spriteURL_) {
    this->id = id_;
    this->spriteURL = spriteURL_;
}

} // namespace style
} // namespace mbgl
