// This is an interface-compatible file analogous to color.cpp
// which is conditionally compiled when the optional Rust build flag is enabled.
#include <cmath>

#include <mbgl/util/color.hpp>
#include <mbgl/util/string.hpp>

#include <vector>

#include <rustutils/color.hpp>

namespace mbgl {

std::optional<Color> Color::parse(const std::string& s) {
    const auto css_color = rustutils::parse_css_color(s);
    if (css_color.success) {
        return {{css_color.r * css_color.a, css_color.g * css_color.a, css_color.b * css_color.a, css_color.a}};
    } else {
        return {};
    }
}

std::string Color::stringify() const {
    std::array<double, 4> array = toArray();
    return "rgba(" + util::toString(array[0]) + "," + util::toString(array[1]) + "," + util::toString(array[2]) + "," +
           util::toString(array[3]) + ")";
}

std::array<double, 4> Color::toArray() const {
    if (a == 0) {
        return {{0, 0, 0, 0}};
    } else {
        return {{
            r * 255 / a,
            g * 255 / a,
            b * 255 / a,
            floor(a * 100 + .5) / 100 // round to 2 decimal places
        }};
    }
}

mbgl::Value Color::toObject() const {
    const auto array = toArray();
    return std::vector<mbgl::Value>{
        std::string("rgba"),
        array[0],
        array[1],
        array[2],
        array[3],
    };
}

mbgl::Value Color::serialize() const {
    std::array<double, 4> array = toArray();
    return std::vector<mbgl::Value>{
        std::string("rgba"),
        array[0],
        array[1],
        array[2],
        array[3],
    };
}

} // namespace mbgl
