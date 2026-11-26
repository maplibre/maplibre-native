#pragma once

#include <mln/util/variant.hpp>
#include <mln/util/tileset.hpp>

#include <string>

#include "../value.hpp"

namespace mln {
namespace android {

variant<std::string, Tileset> convertURLOrTileset(mln::android::Value&& value);

}
} // namespace mln
