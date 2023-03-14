#pragma once

#include <mbgl/style/conversion.hpp>
#include <mbgl/util/color.hpp>

#include <string>
#include <vector>
#include <optional>

namespace mbgl {
namespace style {
namespace expression {

class Image {
public:
    Image() = default;
    Image(const char* imageID);
    Image(std::string imageID);
    explicit Image(std::string imageID, bool available);
    bool operator==(const Image&) const;
    mbgl::Value toValue() const;
    const std::string& id() const;
    bool isAvailable() const;
    bool empty() const;

private:
    std::string imageID;
    bool available;
};

} // namespace expression

namespace conversion {

template <>
struct Converter<expression::Image> {
public:
    std::optional<expression::Image> operator()(const Convertible& value, Error& error) const;
};

template <>
struct ValueFactory<expression::Image> {
    static Value make(const expression::Image& image) { return image.toValue(); }
};

} // namespace conversion

} // namespace style
} // namespace mbgl
