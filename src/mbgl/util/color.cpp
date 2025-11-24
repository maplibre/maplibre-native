#include <mbgl/util/color.hpp>
#include <mbgl/util/string.hpp>

#include <csscolorparser/csscolorparser.hpp>

namespace mbgl {

std::optional<Color> Color::parse(const std::string& s) {
    std::string colorString = s;

    // --- START: FIX to support #RGBA (4-digit hex) ---
    // Check for the 4-digit hex format: #RGBA (length 5, including '#')
    if (colorString.length() == 5 && colorString[0] == '#') {
        // Convert #RGBA to rgba() format since CSSColorParser doesn't support 8-digit hex
        auto hexToInt = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return 0;
        };
        
        int r = hexToInt(colorString[1]);
        int g = hexToInt(colorString[2]);
        int b = hexToInt(colorString[3]);
        int a = hexToInt(colorString[4]);
        
        // Expand from 0-15 range to 0-255 range
        r = (r << 4) | r;
        g = (g << 4) | g;
        b = (b << 4) | b;
        a = (a << 4) | a;
        
        // Convert to rgba() format with alpha in 0-1 range
        colorString = "rgba(" + util::toString(r) + "," + 
                      util::toString(g) + "," + 
                      util::toString(b) + "," + 
                      util::toString(a / 255.0) + ")";
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
