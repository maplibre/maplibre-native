#pragma once

#include <mbgl/util/feature.hpp>

namespace mbgl {

// A set of four numbers representing padding around a box.
// Create instances from bare arrays or numeric values using
// the static method `Padding.parse`. //BUGBUG verify
//
// BUGBUG add: A single number is accepted for backwards-compatibility, and treated the
// same as a one-element array — padding applied to all sides.
// Padding values are in CSS order: top, right, bottom, left.
class Padding {
public:
    Padding() = default;
    Padding(float value)
        : top(value), right(value), bottom(value), left(value) {
    };

    Padding(float t_, float r_, float b_, float l_)
        : top(t_), right(r_), bottom(b_), left(l_) {
    }

    float top = 0;
    float right = 0;
    float bottom = 0;
    float left = 0;

    explicit operator bool() const {
        return top != 0 || right != 0 || bottom != 0 || left != 0;
    }

    static std::optional<Padding> parse(const std::string&);
    std::string toString() const;

    // BUGBUG wip
//    std::array<double, 4> toArray() const;
//    operator std::array<float, 4>() const { return {r, g, b, a}; }
//    mbgl::Value toObject() const;
    mbgl::Value serialize() const;
};

inline bool operator==(const Padding& paddingA, const Padding& paddingB) {
    return
        paddingA.top == paddingB.top &&
        paddingA.right == paddingB.right &&
        paddingA.bottom == paddingB.bottom &&
        paddingA.left == paddingB.left;
}

inline bool operator!=(const Padding& paddingA, const Padding& paddingB) {
    return !(paddingA == paddingB);
}

inline Padding operator*(const Padding& padding, float scale) {
    return { padding.top * scale, padding.right * scale, padding.bottom * scale, padding.left * scale };
}

} // namespace mbgl
