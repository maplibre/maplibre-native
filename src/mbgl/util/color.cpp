#include <mbgl/util/color.hpp>
#include <mbgl/util/string.hpp>

#include <csscolorparser/csscolorparser.hpp>

namespace mbgl {

std::optional<Color> Color::parse(const std::string& s) {
    std::string colorString = s;

    // --- START: FIX to support #RGBA (4-digit hex) ---
    // Check for the 4-digit hex format: #RGBA (length 5, including '#')
    if (colorString.length() == 5 && colorString[0] == '#') {
        std::string expandedColor = "#";
        // R (Red)
        expandedColor += colorString[1];
        expandedColor += colorString[1];
        // G (Green)
        expandedColor += colorString[2];
        expandedColor += colorString[2];
        // B (Blue)
        expandedColor += colorString[3];
        expandedColor += colorString[3];
        // A (Alpha)
        expandedColor += colorString[4];
        expandedColor += colorString[4];

        colorString = std::move(expandedColor);
    }
    // --- END: FIX to support #RGBA (4-digit hex) ---

    const auto css_color = CSSColorParser::parse(colorString);

    // Premultiply the color.
    if (css_color) {
        const float factor = css_color->a / 255;
        return {{css_color->r * factor, css_color->g * factor, css_color->b * factor, css_color->a}};
    } else {
        return {};
    }
}

std::string Color::stringify() const {
    std::array<double, 4> array = toArray();
    return "rgba(" + util::toString(array[0]) + "," + util::toString(array[1]) + "," + util::toString(array[2]) + "," +
           util::toString(array[3]) + ")";
}

mbgl::Value Color::serialize() const {
    return toObject();
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
    return mapbox::base::ValueObject{{"r", static_cast<double>(r)},
                                     {"g", static_cast<double>(g)},
                                     {"b", static_cast<double>(b)},
                                     {"a", static_cast<double>(a)}};
}

} // namespace mbgl
