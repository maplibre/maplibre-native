#include <mln/util/color.hpp>
#include <mln/util/interpolate.hpp>
#include <mln/util/string.hpp>
#include <csscolorparser/csscolorparser.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <numbers>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
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

using Vec3 = std::array<double, 3>;
using Mat3 = std::array<Vec3, 3>;

// The matrices of https://drafts.csswg.org/css-color-4/#color-conversion-code
constexpr Mat3 linearSrgbToXyz = {{{506752.0 / 1228815.0, 87881.0 / 245763.0, 12673.0 / 70218.0},
                                   {87098.0 / 409605.0, 175762.0 / 245763.0, 12673.0 / 175545.0},
                                   {7918.0 / 409605.0, 87881.0 / 737289.0, 1001167.0 / 1053270.0}}};
constexpr Mat3 xyzToLinearSrgb = {{{12831.0 / 3959.0, -329.0 / 214.0, -1974.0 / 3959.0},
                                   {-851781.0 / 878810.0, 1648619.0 / 878810.0, 36519.0 / 878810.0},
                                   {705.0 / 12673.0, -2585.0 / 12673.0, 705.0 / 667.0}}};
constexpr Mat3 xyzToLms = {{{0.8190224379967030, 0.3619062600528904, -0.1288737815209879},
                            {0.0329836539323885, 0.9292868615863434, 0.0361446663506424},
                            {0.0481771893596242, 0.2642395317527308, 0.6335478284694309}}};
constexpr Mat3 lmsToOklab = {{{0.2104542683093140, 0.7936177747023054, -0.0040720430116193},
                              {1.9779985324311684, -2.4285922420485799, 0.4505937096174110},
                              {0.0259040424655478, 0.7827717124575296, -0.8086757549230774}}};
constexpr Mat3 oklabToLms = {{{1.0, 0.3963377773761749, 0.2158037573099136},
                              {1.0, -0.1055613458156586, -0.0638541728258133},
                              {1.0, -0.0894841775298119, -1.2914855480194092}}};
constexpr Mat3 lmsToXyz = {{{1.2268798758459243, -0.5578149944602171, 0.2813910456659647},
                            {-0.0405757452148008, 1.1122868032803170, -0.0717110580655164},
                            {-0.0763729366746601, -0.4214933324022432, 1.5869240198367816}}};

constexpr double degToRad = std::numbers::pi / 180.0;
constexpr double radToDeg = 180.0 / std::numbers::pi;

// https://drafts.csswg.org/css-color-4/#powerless
constexpr double powerlessHueEpsilon = 0.000004;
// The chroma of 100% in oklch(): https://drafts.csswg.org/css-color-4/#specifying-oklab-oklch
constexpr double chromaPercentReference = 0.4;
// Keeps the conversion finite: past this chroma, the clipped color stops changing.
constexpr double maxChroma = 1e6;

struct Oklch {
    double l, c, h, alpha;
};

Vec3 multiply(const Mat3& m, const Vec3& v) {
    return {m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2],
            m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2],
            m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2]};
}

double linearToSrgb(double x) {
    const double magnitude = std::abs(x);
    return magnitude > 0.0031308 ? std::copysign(1.055 * std::pow(magnitude, 1.0 / 2.4) - 0.055, x) : 12.92 * x;
}

Vec3 srgbToOklab(const Vec3& rgb) {
    const Vec3 lms = multiply(xyzToLms, multiply(linearSrgbToXyz, {rgb2xyz(rgb[0]), rgb2xyz(rgb[1]), rgb2xyz(rgb[2])}));
    return multiply(lmsToOklab, {std::cbrt(lms[0]), std::cbrt(lms[1]), std::cbrt(lms[2])});
}

Vec3 oklabToSrgb(const Vec3& lab) {
    const Vec3 lms = multiply(oklabToLms, lab);
    const Vec3 rgb = multiply(
        xyzToLinearSrgb,
        multiply(lmsToXyz, {lms[0] * lms[0] * lms[0], lms[1] * lms[1] * lms[1], lms[2] * lms[2] * lms[2]}));
    return {linearToSrgb(rgb[0]), linearToSrgb(rgb[1]), linearToSrgb(rgb[2])};
}

Oklch rgbToOklch(const Rgb& rgb) {
    const Vec3 lab = srgbToOklab({rgb.r, rgb.g, rgb.b});
    const double c = std::sqrt(lab[1] * lab[1] + lab[2] * lab[2]);
    if (c <= powerlessHueEpsilon) {
        return {.l = lab[0], .c = 0.0, .h = nan, .alpha = rgb.a};
    }
    return {.l = lab[0], .c = c, .h = constrainAngle(std::atan2(lab[2], lab[1]) * radToDeg), .alpha = rgb.a};
}

// Clips a color outside sRGB channel by channel, as browsers draw one.
Rgb oklchToRgb(const Oklch& color) {
    const double hue = std::isnan(color.h) ? 0.0 : color.h * degToRad;
    const double chroma = std::min(color.c, maxChroma);
    const Vec3 rgb = oklabToSrgb({color.l, chroma * std::cos(hue), chroma * std::sin(hue)});
    return {.r = std::clamp(rgb[0], 0.0, 1.0),
            .g = std::clamp(rgb[1], 0.0, 1.0),
            .b = std::clamp(rgb[2], 0.0, 1.0),
            .a = color.alpha};
}

bool isCssWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

// Reads `none` as 0, or a CSS <number> followed by a `%` that makes 100% `percentReference` or, for a hue, by an
// angle unit, in degrees.
std::optional<double> readOklchArgument(std::string_view input, std::size_t& pos, double percentReference, bool hue) {
    if (input.substr(pos, 4) == "none") {
        pos += 4;
        return 0.0;
    }
    const std::size_t start = pos;
    const auto digits = [&] {
        const std::size_t first = pos;
        while (pos < input.size() && isDigit(input[pos])) {
            ++pos;
        }
        return pos > first;
    };
    if (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) {
        ++pos;
    }
    const bool integer = digits();
    if (pos + 1 < input.size() && input[pos] == '.' && isDigit(input[pos + 1])) {
        ++pos;
        digits();
    } else if (!integer) {
        return std::nullopt;
    }
    if (pos < input.size() && input[pos] == 'e') {
        std::size_t exponent = pos + 1;
        if (exponent < input.size() && (input[exponent] == '+' || input[exponent] == '-')) {
            ++exponent;
        }
        if (exponent < input.size() && isDigit(input[exponent])) {
            pos = exponent;
            digits();
        }
    }
    double value = std::strtod(std::string(input.substr(start, pos - start)).c_str(), nullptr);
    if (hue) {
        constexpr std::array<std::pair<std::string_view, double>, 4> units = {
            {{"deg", 1.0}, {"grad", 0.9}, {"rad", radToDeg}, {"turn", 360.0}}};
        for (const auto& [unit, degrees] : units) {
            if (input.substr(pos, unit.size()) == unit) {
                pos += unit.size();
                value *= degrees;
                break;
            }
        }
    } else if (pos < input.size() && input[pos] == '%') {
        ++pos;
        value = value / 100.0 * percentReference;
    }
    if (!std::isfinite(value)) {
        return std::nullopt;
    }
    return value;
}

char toLowerAscii(char c) {
    return c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c;
}

// https://drafts.csswg.org/css-color-4/#specifying-oklab-oklch
std::optional<Oklch> parseOklch(std::string_view text) {
    constexpr std::string_view prefix = "oklch(";
    const auto first = std::ranges::find_if_not(text, isCssWhitespace);
    const auto last = std::ranges::find_if_not(text | std::views::reverse, isCssWhitespace).base();
    if (last - first <= static_cast<std::ptrdiff_t>(prefix.size()) ||
        !std::ranges::equal(std::string_view(first, first + prefix.size()), prefix, {}, toLowerAscii)) {
        return std::nullopt;
    }
    std::string input(first, last);
    std::ranges::transform(input, input.begin(), toLowerAscii);
    if (!input.ends_with(")")) {
        return std::nullopt;
    }
    const std::string_view inside = std::string_view(input).substr(prefix.size(), input.size() - prefix.size() - 1);
    std::size_t pos = 0;
    const auto skipWhitespace = [&] {
        const std::size_t from = pos;
        while (pos < inside.size() && isCssWhitespace(inside[pos])) {
            ++pos;
        }
        return pos > from;
    };

    skipWhitespace();
    const auto l = readOklchArgument(inside, pos, 1.0, false);
    if (!l || !skipWhitespace()) {
        return std::nullopt;
    }
    const auto c = readOklchArgument(inside, pos, chromaPercentReference, false);
    if (!c || !skipWhitespace()) {
        return std::nullopt;
    }
    const auto h = readOklchArgument(inside, pos, 1.0, true);
    if (!h) {
        return std::nullopt;
    }
    skipWhitespace();
    std::optional<double> alpha = 1.0;
    if (pos < inside.size() && inside[pos] == '/') {
        ++pos;
        skipWhitespace();
        alpha = readOklchArgument(inside, pos, 1.0, false);
        skipWhitespace();
    }
    if (!alpha || pos != inside.size()) {
        return std::nullopt;
    }
    return Oklch{.l = std::clamp(*l, 0.0, 1.0),
                 .c = std::max(*c, 0.0),
                 .h = std::fmod(*h, 360.0),
                 .alpha = std::clamp(*alpha, 0.0, 1.0)};
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
        case ColorSpace::OKLCH: {
            // https://drafts.csswg.org/css-color-4/#interpolation
            const Oklch a = rgbToOklch(straight(from));
            const Oklch b = rgbToOklch(straight(to));
            double hueA = std::isnan(a.h) ? b.h : a.h;
            double hueB = std::isnan(b.h) ? a.h : b.h;
            if (hueB - hueA > 180.0) {
                hueA += 360.0;
            } else if (hueB - hueA < -180.0) {
                hueB += 360.0;
            }
            const double alpha = lerp(a.alpha, b.alpha, t);
            const double divisor = alpha == 0.0 ? 1.0 : alpha;
            return premultiplied(oklchToRgb({.l = lerp(a.l * a.alpha, b.l * b.alpha, t) / divisor,
                                             .c = lerp(a.c * a.alpha, b.c * b.alpha, t) / divisor,
                                             .h = lerp(hueA, hueB, t),
                                             .alpha = alpha}));
        }
    }
    assert(false);
    return from;
}

std::optional<Color> Color::parse(const std::string& s) {
    if (const auto oklch = parseOklch(s)) {
        return premultiplied(oklchToRgb(*oklch));
    }

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
