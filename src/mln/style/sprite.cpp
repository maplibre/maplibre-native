#include <mln/style/sprite.hpp>

namespace mln {
namespace style {

Sprite::~Sprite() = default;

Sprite::Sprite(std::string id_, std::string spriteURL_) {
    this->id = id_;
    this->spriteURL = spriteURL_;
}

} // namespace style
} // namespace mln
