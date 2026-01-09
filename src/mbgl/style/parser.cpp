#include <mbgl/style/parser.hpp>
#include <mbgl/style/layer_impl.hpp>
#include <mbgl/style/rapidjson_conversion.hpp>
#include <mbgl/style/conversion/coordinate.hpp>
#include <mbgl/style/conversion/source.hpp>
#include <mbgl/style/conversion/layer.hpp>
#include <mbgl/style/conversion/light.hpp>
#include <mbgl/style/conversion/sprite.hpp>
#include <mbgl/style/conversion/transition_options.hpp>
#include <mbgl/style/conversion_impl.hpp>

#include <mbgl/util/logging.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/convert.hpp>

#include <mapbox/geojsonvt.hpp>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <algorithm>
#include <memory>
#include <set>
#include <unordered_set>

namespace mbgl {
namespace style {

Parser::~Parser() = default;

StyleParseResult Parser::parse(const std::string& json) {
    rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> document;
    document.Parse<0>(json.c_str());

    if (document.HasParseError()) {
        return std::make_exception_ptr(std::runtime_error(formatJSONParseError(document)));
    }

    if (!document.IsObject()) {
        return std::make_exception_ptr(std::runtime_error("style must be an object"));
    }

    if (document.HasMember("version")) {
        const JSValue& versionValue = document["version"];
        const int version = versionValue.IsNumber() ? versionValue.GetInt() : 0;
        if (version != 8) {
            Log::Warning(Event::ParseStyle,
                         "current renderer implementation only supports style spec "
                         "version 8; using an outdated style "
                         "will cause rendering errors");
        }
    }

    if (document.HasMember("name")) {
        const JSValue& value = document["name"];
        if (value.IsString()) {
            name = {value.GetString(), value.GetStringLength()};
        }
    }

    if (document.HasMember("center")) {
        const JSValue& value = document["center"];
        conversion::Error error;
        auto convertedLatLng = conversion::convert<LatLng>(value, error);
        if (convertedLatLng) {
            latLng = *convertedLatLng;
        } else {
            Log::Warning(Event::ParseStyle, "center coordinate must be a longitude, latitude pair");
        }
    }

    if (document.HasMember("zoom")) {
        const JSValue& value = document["zoom"];
        if (value.IsNumber()) {
            zoom = value.GetDouble();
        }
    }

    if (document.HasMember("bearing")) {
        const JSValue& value = document["bearing"];
        if (value.IsNumber()) {
            bearing = value.GetDouble();
        }
    }

    if (document.HasMember("pitch")) {
        const JSValue& value = document["pitch"];
        if (value.IsNumber()) {
            pitch = value.GetDouble();
        }
    }

    if (document.HasMember("transition")) {
        parseTransition(document["transition"]);
    }

    if (document.HasMember("light")) {
        parseLight(document["light"]);
    }

    if (document.HasMember("sources")) {
        parseSources(document["sources"]);
    }

    if (document.HasMember("layers")) {
        parseLayers(document["layers"]);
    }

    if (document.HasMember("sprite")) {
        parseSprites(document["sprite"]);
    }

    if (document.HasMember("glyphs")) {
        const JSValue& glyphs = document["glyphs"];
        if (glyphs.IsString()) {
            glyphURL = {glyphs.GetString(), glyphs.GetStringLength()};
        }
    }

#ifdef MLN_TEXT_SHAPING_HARFBUZZ
    // Ignore font-faces if no harfbuzz
    if (document.HasMember("font-faces")) {
        const JSValue& faces = document["font-faces"];
        if (faces.IsObject()) {
            fontFaces = std::make_shared<FontFaces>();
            for (auto it = faces.MemberBegin(); it != faces.MemberEnd(); ++it) {
                const std::string& faceName = it->name.GetString();
                const JSValue& faceValue = it->value;

                if (faceValue.IsArray()) {
                    // If the face is an array, we assume it is a list of font file objects.
                    for (const auto& fontFile : faceValue.GetArray()) {
                        if (fontFile.IsObject()) {
                            FontFace fontFace(faceName, "", {{0, 0x10FFFF}});

                            // Parse url
                            if (fontFile.HasMember("url")) {
                                const JSValue& url = fontFile["url"];
                                if (url.IsString()) fontFace.url = url.GetString();
                            }

                            // Parse unicode-range
                            if (fontFile.HasMember("unicode-range")) {
                                const JSValue& unicodeRange = fontFile["unicode-range"];
                                fontFace.ranges.clear();
                                if (unicodeRange.IsArray()) {
                                    for (auto& range : unicodeRange.GetArray()) {
                                        if (range.IsString()) {
                                            std::string rangeString = range.GetString();
                                            if (rangeString.length() > 2) {
                                                rangeString = rangeString.substr(2);
                                                std::string::size_type pos = rangeString.find('-');
                                                if (pos != std::string::npos) {
                                                    std::string start = rangeString.substr(0, pos);
                                                    std::string end = rangeString.substr(pos + 1);
                                                    if (!start.empty() && !end.empty()) {
                                                        auto startInt = util::parse<int>(start, 16);
                                                        auto endInt = util::parse<int>(end, 16);
                                                        if (startInt && endInt) {
                                                            fontFace.ranges.emplace_back(*startInt, *endInt);
                                                        } else {
                                                            Log::Warning(
                                                                Event::ParseStyle,
                                                                "Invalid unicode-range in font-face: " + rangeString);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            // If valid, generate a unique glyph ID type for this font face
                            // and add it to the font faces list.
                            if (fontFace.valid()) {
                                fontFace.type = genNewGlyphIDType(
                                    fontFace.url, FontStack{fontFace.name}, fontFace.ranges);
                                fontFaces->emplace_back(std::move(fontFace));
                            } else {
                                Log::Warning(Event::ParseStyle, "Invalid font-face definition for: " + faceName);
                            }
                        }
                    }
                } else if (faceValue.IsString()) {
                    FontFace fontFace(faceName, faceValue.GetString(), {{0, 0x10FFFF}});
                    fontFace.type = genNewGlyphIDType(fontFace.url, FontStack{fontFace.name}, fontFace.ranges);
                    fontFaces->emplace_back(std::move(fontFace));
                } else {
                    Log::Warning(Event::ParseStyle, "font-face must be an object or array");
                }
            }
        };
    }
#endif

    // Call for side effect of logging warnings for invalid values.
    fontStacks();

    return nullptr;
}

void Parser::parseTransition(const JSValue& value) {
    conversion::Error error;
    std::optional<TransitionOptions> converted = conversion::convert<TransitionOptions>(value, error);
    if (!converted) {
        Log::Warning(Event::ParseStyle, error.message);
        return;
    }

    transition = std::move(*converted);
}

void Parser::parseLight(const JSValue& value) {
    conversion::Error error;
    std::optional<Light> converted = conversion::convert<Light>(value, error);
    if (!converted) {
        Log::Warning(Event::ParseStyle, error.message);
        return;
    }

    light = *converted;
}

void Parser::parseSources(const JSValue& value) {
    if (!value.IsObject()) {
        Log::Warning(Event::ParseStyle, "sources must be an object");
        return;
    }

    for (const auto& property : value.GetObject()) {
        std::string id{property.name.GetString(), property.name.GetStringLength()};

        conversion::Error error;
        std::optional<std::unique_ptr<Source>> source = conversion::convert<std::unique_ptr<Source>>(
            property.value, error, id);
        if (!source) {
            Log::Warning(Event::ParseStyle, error.message);
            continue;
        }

        sources.emplace_back(std::move(*source));
    }
}

void Parser::parseSprites(const JSValue& value) {
    if (value.IsString()) {
        std::string url = {value.GetString(), value.GetStringLength()};
        auto sprite = Sprite("default", url);
        sprites.emplace_back(sprite);
    } else if (value.IsArray()) {
        std::unordered_set<std::string> spriteIds;
        for (auto& spriteValue : value.GetArray()) {
            if (!spriteValue.IsObject()) {
                Log::Warning(Event::ParseStyle, "sprite child must be an object");
                continue;
            }

            conversion::Error error;
            std::optional<Sprite> sprite = conversion::convert<Sprite>(spriteValue, error);
            if (!sprite) {
                Log::Warning(Event::ParseStyle, error.message);
                continue;
            }

            if (spriteIds.find(sprite->id) != spriteIds.end()) {
                Log::Warning(Event::ParseStyle, "sprite ids must be unique");
                continue;
            }

            spriteIds.insert(sprite->id);
            sprites.emplace_back(*sprite);
        }
    } else {
        Log::Warning(Event::ParseStyle, "sprite must be an object or string");
        return;
    }
}

void Parser::parseLayers(const JSValue& value) {
    std::vector<std::string> ids;

    if (!value.IsArray()) {
        Log::Warning(Event::ParseStyle, "layers must be an array");
        return;
    }

    for (auto& layerValue : value.GetArray()) {
        if (!layerValue.IsObject()) {
            Log::Warning(Event::ParseStyle, "layer must be an object");
            continue;
        }

        if (!layerValue.HasMember("id")) {
            Log::Warning(Event::ParseStyle, "layer must have an id");
            continue;
        }

        const JSValue& id = layerValue.FindMember("id")->value;
        if (!id.IsString()) {
            Log::Warning(Event::ParseStyle, "layer id must be a string");
            continue;
        }

        const std::string layerID = {id.GetString(), id.GetStringLength()};
        if (layersMap.find(layerID) != layersMap.end()) {
            Log::Warning(Event::ParseStyle, "duplicate layer id " + layerID);
            continue;
        }

        layersMap.emplace(layerID, std::pair<const JSValue&, std::unique_ptr<Layer>>{layerValue, nullptr});
        ids.push_back(layerID);
    }

    for (const auto& id : ids) {
        auto it = layersMap.find(id);

        parseLayer(it->first, it->second.first, it->second.second);
    }

    for (const auto& id : ids) {
        auto it = layersMap.find(id);

        if (it->second.second) {
            layers.emplace_back(std::move(it->second.second));
        }
    }
}

void Parser::parseLayer(const std::string& id, const JSValue& value, std::unique_ptr<Layer>& layer) {
    if (layer) {
        // Skip parsing this again. We already have a valid layer definition.
        return;
    }

    // Make sure we have not previously attempted to parse this layer.
    if (std::find(stack.begin(), stack.end(), id) != stack.end()) {
        Log::Warning(Event::ParseStyle, "layer reference of '" + id + "' is circular");
        return;
    }

    if (value.HasMember("ref")) {
        // This layer is referencing another layer. Recursively parse that layer.
        const JSValue& refVal = value["ref"];
        if (!refVal.IsString()) {
            Log::Warning(Event::ParseStyle, "layer ref of '" + id + "' must be a string");
            return;
        }

        const std::string ref{refVal.GetString(), refVal.GetStringLength()};
        auto it = layersMap.find(ref);
        if (it == layersMap.end()) {
            Log::Warning(Event::ParseStyle, "layer '" + id + "' references unknown layer " + ref);
            return;
        }

        // Recursively parse the referenced layer.
        stack.push_front(id);
        parseLayer(it->first, it->second.first, it->second.second);
        stack.pop_front();

        Layer* reference = it->second.second.get();
        if (!reference) {
            return;
        }

        layer = reference->cloneRef(id);
        conversion::setPaintProperties(*layer, conversion::Convertible(&value));
    } else {
        conversion::Error error;
        std::optional<std::unique_ptr<Layer>> converted = conversion::convert<std::unique_ptr<Layer>>(value, error);
        if (!converted) {
            Log::Warning(Event::ParseStyle, error.message);
            return;
        }
        layer = std::move(*converted);
    }
}

std::set<FontStack> Parser::fontStacks() const {
    std::vector<Immutable<Layer::Impl>> impls;
    impls.reserve(layers.size());
    for (const auto& layer : layers) {
        impls.emplace_back(layer->baseImpl);
    }
    return mbgl::fontStacks(impls);
}

} // namespace style
} // namespace mbgl
