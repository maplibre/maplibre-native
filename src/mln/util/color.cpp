#include <mln/util/color.hpp>
#include <mln/util/interpolate.hpp>
#include <mln/util/string.hpp>
#include <csscolorparser/csscolorparser.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <vector>

namespace mln {

namespace {

// The D50 Lab conversions GL JS uses (color_spaces.ts, after https://observablehq.com/@mbostock/lab-and-rgb).
constexpr double Xn = 0.96422;
constexpr double Yn = 1.0;
constexpr double Zn = 0.82521;
constexpr double t0 = 4.0 / 29.0;
constexpr double t1 = 6.0 / 29.0;
constexpr double t2 = 3.0 * t1 * t1;
constexpr double t3 = t1 * t1 * t1;
constexpr double nan = std::numeric_limits<double>::quiet_NaN();

struct Rgb {
    double r, g, b, a;
};

struct Lab {
    double l, a, b, alpha;
};

struct Hcl {
    double h, c, l, alpha;
};

double rgb2xyz(double x) {
    return x <= 0.04045 ? x / 12.92 : std::pow((x + 0.055) / 1.055, 2.4);
}

double xyz2lab(double t) {
    return t > t3 ? std::pow(t, 1.0 / 3.0) : t / t2 + t0;
}

double lab2xyz(double t) {
    return t > t1 ? t * t * t : t2 * (t - t0);
}

double xyz2rgb(double x) {
    x = x <= 0.00304 ? 12.92 * x : 1.055 * std::pow(x, 1.0 / 2.4) - 0.055;
    return std::clamp(x, 0.0, 1.0);
}

double constrainAngle(double angle) {
    angle = std::fmod(angle, 360.0);
    return angle < 0 ? angle + 360.0 : angle;
}

Lab rgbToLab(const Rgb& c) {
    const double r = rgb2xyz(c.r);
    const double g = rgb2xyz(c.g);
    const double b = rgb2xyz(c.b);
    const double y = xyz2lab((0.2225045 * r + 0.7168786 * g + 0.0606169 * b) / Yn);
    double x = y;
    double z = y;
    if (r != g || g != b) {
        x = xyz2lab((0.4360747 * r + 0.3850649 * g + 0.1430804 * b) / Xn);
        z = xyz2lab((0.0139322 * r + 0.0971045 * g + 0.7141733 * b) / Zn);
    }
    const double l = 116.0 * y - 16.0;
    return {.l = l < 0 ? 0 : l, .a = 500.0 * (x - y), .b = 200.0 * (y - z), .alpha = c.a};
}

Rgb labToRgb(const Lab& c) {
    double y = (c.l + 16.0) / 116.0;
    double x = std::isnan(c.a) ? y : y + c.a / 500.0;
    double z = std::isnan(c.b) ? y : y - c.b / 200.0;
    y = Yn * lab2xyz(y);
    x = Xn * lab2xyz(x);
    z = Zn * lab2xyz(z);
    return {.r = xyz2rgb(3.1338561 * x - 1.6168667 * y - 0.4906146 * z),
            .g = xyz2rgb(-0.9787684 * x + 1.9161415 * y + 0.033454 * z),
            .b = xyz2rgb(0.0719453 * x - 0.2289914 * y + 1.4052427 * z),
            .a = c.alpha};
}

Hcl rgbToHcl(const Rgb& rgb) {
    const Lab lab = rgbToLab(rgb);
    const double c = std::sqrt(lab.a * lab.a + lab.b * lab.b);
    const double h = std::round(c * 10000.0) != 0 ? constrainAngle(std::atan2(lab.b, lab.a) * 180.0 / std::numbers::pi)
                                                  : nan;
    return {.h = h, .c = c, .l = lab.l, .alpha = lab.alpha};
}

Rgb hclToRgb(const Hcl& c) {
    const double h = std::isnan(c.h) ? 0.0 : c.h * std::numbers::pi / 180.0;
    return labToRgb({.l = c.l, .a = std::cos(h) * c.c, .b = std::sin(h) * c.c, .alpha = c.alpha});
}

Rgb straight(const Color& color) {
    if (color.a == 0) {
        return {.r = 0, .g = 0, .b = 0, .a = 0};
    }
    return {.r = color.r / color.a, .g = color.g / color.a, .b = color.b / color.a, .a = color.a};
}

Color premultiplied(const Rgb& c) {
    return {static_cast<float>(c.r * c.a),
            static_cast<float>(c.g * c.a),
            static_cast<float>(c.b * c.a),
            static_cast<float>(c.a)};
}

double lerp(double a, double b, double t) {
    return a * (1.0 - t) + b * t;
}

} // namespace

Color Color::interpolate(const Color& from, const Color& to, const double t, const ColorSpace space) {
    switch (space) {
        case ColorSpace::RGB:
            return util::interpolate(from, to, t);
        case ColorSpace::LAB: {
            const Lab a = rgbToLab(straight(from));
            const Lab b = rgbToLab(straight(to));
            return premultiplied(labToRgb({.l = lerp(a.l, b.l, t),
                                           .a = lerp(a.a, b.a, t),
                                           .b = lerp(a.b, b.b, t),
                                           .alpha = lerp(a.alpha, b.alpha, t)}));
        }
        case ColorSpace::HCL: {
            const Hcl a = rgbToHcl(straight(from));
            const Hcl b = rgbToHcl(straight(to));
            double hue = nan;
            double chroma = nan;
            if (!std::isnan(a.h) && !std::isnan(b.h)) {
                double dh = b.h - a.h;
                if (b.h > a.h && dh > 180.0) {
                    dh -= 360.0;
                } else if (b.h < a.h && a.h - b.h > 180.0) {
                    dh += 360.0;
                }
                hue = a.h + t * dh;
            } else if (!std::isnan(a.h)) {
                hue = a.h;
                if (b.l == 1.0 || b.l == 0.0) chroma = a.c;
            } else if (!std::isnan(b.h)) {
                hue = b.h;
                if (a.l == 1.0 || a.l == 0.0) chroma = b.c;
            }
            return premultiplied(hclToRgb({.h = hue,
                                           .c = std::isnan(chroma) ? lerp(a.c, b.c, t) : chroma,
                                           .l = lerp(a.l, b.l, t),
                                           .alpha = lerp(a.alpha, b.alpha, t)}));
        }
    }
    assert(false);
    return from;
}

std::optional<Color> Color::parse(const std::string& s) {
    const auto css_color = CSSColorParser::parse(s);

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

mln::Value Color::serialize() const {
    // Emit as an rgba expression array for expression serialization to avoid
    // "Bare objects invalid" parse errors in expression roundtrips.
    const auto array = toArray();
    return std::vector<mln::Value>{
        std::string("rgba"),
        array[0],
        array[1],
        array[2],
        array[3],
    };
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

mln::Value Color::toObject() const {
    // Return object format for evaluation output
    return mapbox::base::ValueObject{{"r", static_cast<double>(r)},
                                     {"g", static_cast<double>(g)},
                                     {"b", static_cast<double>(b)},
                                     {"a", static_cast<double>(a)}};
}

} // namespace mln
