#include <mbgl/sprite/sprite_parser.hpp>
#include <mbgl/style/image.hpp>
#include <mbgl/style/image_impl.hpp>

#include <mbgl/util/exception.hpp>
#include <mbgl/util/logging.hpp>

#include <mbgl/util/image.hpp>
#include <mbgl/util/rapidjson.hpp>
#include <mbgl/util/string.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

namespace mbgl {

std::unique_ptr<style::Image> createStyleImage(const std::string& id,
                                               const PremultipliedImage& image,
                                               const int32_t srcX,
                                               const int32_t srcY,
                                               const int32_t width,
                                               const int32_t height,
                                               const double ratio,
                                               const bool sdf,
                                               style::ImageStretches&& stretchX,
                                               style::ImageStretches&& stretchY,
                                               const std::optional<style::ImageContent>& content,
                                               const std::optional<style::TextFit>& textFitWidth,
                                               const std::optional<style::TextFit>& textFitHeight) {
    // Disallow invalid parameter configurations.
    if (width <= 0 || height <= 0 || width > 1024 || height > 1024 || ratio <= 0 || ratio > 10 || srcX < 0 ||
        srcY < 0 || srcX >= static_cast<int32_t>(image.size.width) || srcY >= static_cast<int32_t>(image.size.height) ||
        srcX + width > static_cast<int32_t>(image.size.width) ||
        srcY + height > static_cast<int32_t>(image.size.height)) {
        std::ostringstream ss;
        ss << "Can't create image with invalid metrics: " << width << "x" << height << "@" << srcX << "," << srcY
           << " in " << image.size.width << "x" << image.size.height << "@" << util::toString(ratio) << "x"
           << " sprite";
        Log::Error(Event::Sprite, ss.str());
        return nullptr;
    }

    const Size size(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    PremultipliedImage dstImage(size);

    // Copy from the source image into our individual sprite image
    PremultipliedImage::copy(image, dstImage, {static_cast<uint32_t>(srcX), static_cast<uint32_t>(srcY)}, {0, 0}, size);

    try {
        return std::make_unique<style::Image>(id,
                                              std::move(dstImage),
                                              static_cast<float>(ratio),
                                              sdf,
                                              std::move(stretchX),
                                              std::move(stretchY),
                                              content,
                                              textFitWidth,
                                              textFitHeight);
    } catch (const util::StyleImageException& ex) {
        Log::Error(Event::Sprite, std::string("Can't create image with invalid metadata: ") + ex.what());
        return nullptr;
    }
}

namespace {

uint16_t getUInt16(const JSValue& value, const char* property, const char* name, const uint16_t def = 0) {
    if (value.HasMember(property)) {
        auto& v = value[property];
        if (v.IsUint() && v.GetUint() <= std::numeric_limits<uint16_t>::max()) {
            return v.GetUint();
        } else {
            Log::Warning(Event::Sprite,
                         std::string("Invalid sprite image '") + name + "': value of '" + property +
                             "' must be an integer between 0 and 65535");
        }
    }

    return def;
}

double getDouble(const JSValue& value, const char* property, const char* name, const double def = 0) {
    if (value.HasMember(property)) {
        auto& v = value[property];
        if (v.IsNumber()) {
            return v.GetDouble();
        } else {
            Log::Warning(
                Event::Sprite,
                std::string("Invalid sprite image '") + name + "': value of '" + property + "' must be a number");
        }
    }

    return def;
}

bool getBoolean(const JSValue& value, const char* property, const char* name, const bool def = false) {
    if (value.HasMember(property)) {
        auto& v = value[property];
        if (v.IsBool()) {
            return v.GetBool();
        } else {
            Log::Warning(
                Event::Sprite,
                std::string("Invalid sprite image '") + name + "': value of '" + property + "' must be a boolean");
        }
    }

    return def;
}

style::ImageStretches getStretches(const JSValue& value, const char* property, const char* name) {
    style::ImageStretches stretches;

    if (value.HasMember(property)) {
        auto& v = value[property];
        if (v.IsArray()) {
            for (rapidjson::SizeType i = 0; i < v.Size(); ++i) {
                const JSValue& stretch = v[i];
                if (stretch.IsArray() && stretch.Size() == 2 && stretch[rapidjson::SizeType(0)].IsNumber() &&
                    stretch[rapidjson::SizeType(1)].IsNumber()) {
                    stretches.emplace_back(style::ImageStretch{stretch[rapidjson::SizeType(0)].GetFloat(),
                                                               stretch[rapidjson::SizeType(1)].GetFloat()});
                } else {
                    Log::Warning(Event::Sprite,
                                 "Invalid sprite image '" + std::string(name) + "': members of '" + property +
                                     "' must be an array of two numbers");
                }
            }
        } else {
            Log::Warning(
                Event::Sprite,
                "Invalid sprite image '" + std::string(name) + "': value of '" + property + "' must be an array");
        }
    }

    return stretches;
}

std::optional<style::ImageContent> getContent(const JSValue& value, const char* property, const char* name) {
    if (value.HasMember(property)) {
        auto& content = value[property];
        if (content.IsArray() && content.Size() == 4 && content[rapidjson::SizeType(0)].IsNumber() &&
            content[rapidjson::SizeType(1)].IsNumber() && content[rapidjson::SizeType(2)].IsNumber() &&
            content[rapidjson::SizeType(3)].IsNumber()) {
            return style::ImageContent{content[rapidjson::SizeType(0)].GetFloat(),
                                       content[rapidjson::SizeType(1)].GetFloat(),
                                       content[rapidjson::SizeType(2)].GetFloat(),
                                       content[rapidjson::SizeType(3)].GetFloat()};
        } else {
            Log::Warning(Event::Sprite,
                         "Invalid sprite image '" + std::string(name) + "': value of '" + property +
                             "' must be an array of four numbers");
        }
    }

    return std::nullopt;
}

std::optional<style::TextFit> parseTextFit(const std::string_view& value) {
    if (value == "stretchOrShrink") {
        return style::TextFit::stretchOrShrink;
    } else if (value == "stretchOnly") {
        return style::TextFit::stretchOnly;
    } else if (value == "proportional") {
        return style::TextFit::proportional;
    } else {
        return std::nullopt;
    }
}

std::optional<style::TextFit> getTextFit(const JSValue& value, const char* property, const char* name) {
    if (value.HasMember(property)) {
        auto& v = value[property];
        if (v.IsString()) {
            return parseTextFit(std::string_view(v.GetString(), v.GetStringLength()));
        } else {
            Log::Warning(
                Event::Sprite,
                std::string("Invalid sprite image '") + name + "': value of '" + property + "' must be a string");
        }
    }

    return std::nullopt;
}

} // namespace

std::vector<Immutable<style::Image::Impl>> parseSprite(const std::string& id,
                                                       const std::string& encodedImage,
                                                       const std::string& json) {
    const PremultipliedImage raster = decodeImage(encodedImage);

    JSDocument doc;
    doc.Parse<0>(json.c_str());
    if (doc.HasParseError()) {
        throw std::runtime_error("Failed to parse JSON: " + formatJSONParseError(doc));
    }

    if (!doc.IsObject()) {
        throw std::runtime_error("Sprite JSON root must be an object");
    }

    const auto& properties = doc.GetObject();
    std::vector<Immutable<style::Image::Impl>> images;
    images.reserve(properties.MemberCount());
    for (const auto& property : properties) {
        const std::string name = {property.name.GetString(), property.name.GetStringLength()};
        std::string completeName = name;
        if (id != "default") {
            completeName = id + ":";
            completeName += name;
        }
        const JSValue& value = property.value;

        if (value.IsObject()) {
            const uint16_t x = getUInt16(value, "x", name.c_str(), 0);
            const uint16_t y = getUInt16(value, "y", name.c_str(), 0);
            const uint16_t width = getUInt16(value, "width", name.c_str(), 0);
            const uint16_t height = getUInt16(value, "height", name.c_str(), 0);
            const double pixelRatio = getDouble(value, "pixelRatio", name.c_str(), 1);
            const bool sdf = getBoolean(value, "sdf", name.c_str(), false);
            style::ImageStretches stretchX = getStretches(value, "stretchX", name.c_str());
            style::ImageStretches stretchY = getStretches(value, "stretchY", name.c_str());
            std::optional<style::ImageContent> content = getContent(value, "content", name.c_str());
            std::optional<style::TextFit> textFitWidth = getTextFit(value, "textFitWidth", name.c_str());
            std::optional<style::TextFit> textFitHeight = getTextFit(value, "textFitHeight", name.c_str());

            auto image = createStyleImage(completeName,
                                          raster,
                                          x,
                                          y,
                                          width,
                                          height,
                                          pixelRatio,
                                          sdf,
                                          std::move(stretchX),
                                          std::move(stretchY),
                                          content,
                                          textFitWidth,
                                          textFitHeight);
            if (image) {
                images.push_back(std::move(image->baseImpl));
            }
        }
    }

    assert([&images] {
        std::sort(images.begin(), images.end());
        return std::unique(images.begin(), images.end()) == images.end();
    }());
    return images;
}

} // namespace mbgl
