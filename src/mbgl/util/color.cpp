#include <mbgl/util/color.hpp>
#include <mbgl/util/string.hpp>

#include <csscolorparser/csscolorparser.hpp>

namespace mbgl {

std::optional<Color> Color::parse(const std::string& s) {
    std::string colorString = s; // Use a mutable copy of the input string

    // --- START: NEW LOGIC to support #RGBA (4-digit hex) ---
    // Check for the 4-digit hex format: #RGBA (length 5, including '#')
    if (colorString.length() == 5 && colorString[0] == '#') {
        std::string expandedColor = "#";
        // R (Red): colorString[1] is doubled
        expandedColor += colorString[1];
        expandedColor += colorString[1];
        // G (Green): colorString[2] is doubled
        expandedColor += colorString[2];
        expandedColor += colorString[2];
        // B (Blue): colorString[3] is doubled
        expandedColor += colorString[3];
        expandedColor += colorString[3];
        // A (Alpha): colorString[4] is doubled
        expandedColor += colorString[4];
        expandedColor += colorString[4];

        // Replace the original string with the expanded string (e.g., "#F00C" becomes "#FF0000CC")
        colorString = std::move(expandedColor);
    }
    // --- END: NEW LOGIC to support #RGBA (4-digit hex) ---

    // Pass the potentially expanded string to the existing parser
    const auto css_color = CSSColorParser::parse(colorString);

    // Premultiply the color.
    if (css_color) {
        const float factor = css_color->a / 255;
        return {{css_color->r * factor, css_color->g * factor, css_color->b * factor, css_color->a}};
    } else {
        return {};
    }
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
