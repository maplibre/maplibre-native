#include <mln/test/util.hpp>

#include <mln/style/conversion/filter.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/layer.hpp>
#include <mln/style/layers/background_layer_impl.hpp>
#include <mln/style/layers/fill_extrusion_layer_impl.hpp>

#include <rapidjson/prettywriter.h>

using namespace mln;
using namespace mln::style;
using namespace mln::style::conversion;
using namespace std::literals::chrono_literals;

std::unique_ptr<Layer> parseLayer(const std::string& src) {
    Error error;
    auto layer = convertJSON<std::unique_ptr<Layer>>(src, error);
    if (layer) return std::move(*layer);
    return nullptr;
}

Filter parseFilter(const std::string& expression) {
    Error error;
    return *convertJSON<Filter>(expression, error);
}

std::string stringifyLayer(const Value& value) {
    rapidjson::StringBuffer s;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);
    writer.SetIndent(' ', 2u);
    stringify(writer, value);
    return s.GetString();
}

TEST(StyleConversion, LayerTransition) {
    auto layer = parseLayer(R"JSON({
        "type": "background",
        "id": "background",
        "paint": {
            "background-color-transition": {
                "duration": 400,
                "delay": 500
            }
        }
    })JSON");
    ASSERT_STREQ("background", layer->getTypeInfo()->type);
    ASSERT_EQ(400ms, *static_cast<BackgroundLayer*>(layer.get())->impl().paint.get<BackgroundColor>().options.duration);
    ASSERT_EQ(500ms, *static_cast<BackgroundLayer*>(layer.get())->impl().paint.get<BackgroundColor>().options.delay);
}

TEST(StyleConversion, SerializeDefaults) {
    auto layer = parseLayer(R"JSON({
        "type": "background",
        "id": "background"
    })JSON");

    EXPECT_NE(nullptr, layer);
    auto value = layer->serialize();
    EXPECT_NE(nullptr, value.getObject());
    EXPECT_EQ(2u, value.getObject()->size());
}

TEST(StyleConversion, RoundtripWithTransitions) {
    auto layer = parseLayer(R"JSON({
        "type": "background",
        "id": "background",
        "paint": {
            "background-color-transition": {
                "duration": 400,
                "delay": 500
            }
        }
    })JSON");

    EXPECT_NE(nullptr, layer);
    auto value = layer->serialize();
    EXPECT_NE(nullptr, value.getObject());
    EXPECT_EQ(3u, value.getObject()->size());

    auto roundTripped = parseLayer(stringifyLayer(value));
    EXPECT_NE(nullptr, roundTripped);
    auto roundTrippedValue = roundTripped->serialize();
    EXPECT_NE(nullptr, roundTrippedValue.getObject());
    EXPECT_EQ(3u, roundTrippedValue.getObject()->size());
}

TEST(StyleConversion, OverrideDefaults) {
    auto layer = parseLayer(R"JSON({
        "type": "symbol",
        "id": "symbol",
        "source": "composite",
        "source-layer": "landmarks",
        "filter": ["has", "monuments"],
        "minzoom": 12,
        "maxzoom": 18,
        "layout": {
            "visibility": "none",
            "text-field": ["format",
                            "Hello",
                            ["image", ["get", "world"]],
                            "Example", {"text-color": "rgba(128, 255, 39, 1)"}
                          ],
            "icon-image": ["image", ["get", "landmark_image"]],
            "text-size": 24
        },
        "paint": {
            "text-color": "turquoise",
            "text-color-transition": {
                "duration": 300,
                "delay": 100
            }
        }
    })JSON");

    EXPECT_NE(nullptr, layer);
    auto value = layer->serialize();
    EXPECT_NE(nullptr, value.getObject());
    const auto& object = *(value.getObject());
    EXPECT_EQ(9u, object.size());
    EXPECT_EQ(4u, object.at("layout").getObject()->size());
    EXPECT_EQ(2u, object.at("paint").getObject()->size());

    auto roundTripped = parseLayer(stringifyLayer(value));
    EXPECT_NE(nullptr, roundTripped);
    auto roundTrippedValue = roundTripped->serialize();
    const auto& roundTrippedObject = *(roundTrippedValue.getObject());
    EXPECT_NE(nullptr, roundTrippedValue.getObject());
    EXPECT_EQ(9u, roundTrippedObject.size());
    EXPECT_EQ(4u, roundTrippedObject.at("layout").getObject()->size());
    EXPECT_EQ(2u, roundTrippedObject.at("paint").getObject()->size());
}

TEST(StyleConversion, SetGenericProperties) {
    auto layer = parseLayer(R"JSON({
        "type": "symbol",
        "id": "symbol",
        "source": "composite",
        "source-layer": "landmarks",
        "filter": ["has", "monuments"],
        "minzoom": 12,
        "maxzoom": 18
    })JSON");

    ASSERT_NE(nullptr, layer);
    EXPECT_EQ(parseFilter(R"FILTER(["has", "monuments"])FILTER").serialize(), layer->getFilter().serialize());
    EXPECT_EQ(12.0f, layer->getMinZoom());
    EXPECT_EQ(18.0f, layer->getMaxZoom());
    EXPECT_EQ("landmarks", layer->getSourceLayer());

    const JSValue newComposite("composite_2");
    layer->setProperty("source", Convertible(&newComposite));
    EXPECT_EQ("composite_2", layer->getSourceID());

    const JSValue newSourceLayer("poi");
    layer->setProperty("source-layer", Convertible(&newSourceLayer));
    EXPECT_EQ("poi", layer->getSourceLayer());

    const JSValue newMinZoom(1.0f);
    layer->setProperty("minzoom", Convertible(&newMinZoom));
    EXPECT_EQ(1.0f, layer->getMinZoom());

    const JSValue newMaxZoom(22.0f);
    layer->setProperty("maxzoom", Convertible(&newMaxZoom));
    EXPECT_EQ(22.0f, layer->getMaxZoom());
}

// The `fill-extrusion-shadow-*` paint properties are declared in scripts/style-spec.mjs rather than
// in the vendored scripts/style-spec-reference/v8.json, because `npm run copy-style-spec` replaces
// that file wholesale from the npm package. This suite references the generated symbols by name, so
// it stops compiling if the properties are ever dropped from the spec, the generator or the
// templates -- which is the point.

TEST(StyleConversion, FillExtrusionShadowDefaults) {
    const FillExtrusionLayer layer("shadowed", "source");

    // Undefined until a style sets them, so the renderer can tell "untouched" from "explicitly 0".
    EXPECT_TRUE(layer.getFillExtrusionShadowColor().isUndefined());
    EXPECT_TRUE(layer.getFillExtrusionShadowOpacity().isUndefined());
    EXPECT_TRUE(layer.getFillExtrusionShadowLength().isUndefined());
    EXPECT_TRUE(layer.getFillExtrusionShadowAzimuth().isUndefined());

    // Shadows are off by default: opacity 0 means the renderer allocates nothing at all.
    EXPECT_EQ(PropertyValue<float>(0.0f), FillExtrusionLayer::getDefaultFillExtrusionShadowOpacity());
    EXPECT_EQ(PropertyValue<Color>(Color(0.0f, 0.0f, 0.0f, 0.35f)),
              FillExtrusionLayer::getDefaultFillExtrusionShadowColor());
    EXPECT_EQ(PropertyValue<float>(0.32f), FillExtrusionLayer::getDefaultFillExtrusionShadowLength());
    EXPECT_EQ(PropertyValue<float>(225.0f), FillExtrusionLayer::getDefaultFillExtrusionShadowAzimuth());
}

TEST(StyleConversion, FillExtrusionShadowRoundtrip) {
    auto layer = parseLayer(R"JSON({
        "type": "fill-extrusion",
        "id": "shadowed",
        "source": "composite",
        "paint": {
            "fill-extrusion-shadow-color": "rgba(10, 20, 30, 1)",
            "fill-extrusion-shadow-opacity": 0.5,
            "fill-extrusion-shadow-length": 1.5,
            "fill-extrusion-shadow-azimuth": 315
        }
    })JSON");

    ASSERT_NE(nullptr, layer);
    ASSERT_STREQ("fill-extrusion", layer->getTypeInfo()->type);

    const auto* fillExtrusion = static_cast<const FillExtrusionLayer*>(layer.get());

    // Compare channels with a tolerance: the CSS parser's 8-bit-to-float division does not
    // land on the same ULP as writing `30.0f / 255` here.
    const auto& shadowColor = fillExtrusion->getFillExtrusionShadowColor();
    ASSERT_TRUE(shadowColor.isConstant());
    EXPECT_NEAR(10.0f / 255, shadowColor.asConstant().r, 1e-5);
    EXPECT_NEAR(20.0f / 255, shadowColor.asConstant().g, 1e-5);
    EXPECT_NEAR(30.0f / 255, shadowColor.asConstant().b, 1e-5);
    EXPECT_NEAR(1.0f, shadowColor.asConstant().a, 1e-5);

    EXPECT_EQ(PropertyValue<float>(0.5f), fillExtrusion->getFillExtrusionShadowOpacity());
    EXPECT_EQ(PropertyValue<float>(1.5f), fillExtrusion->getFillExtrusionShadowLength());
    EXPECT_EQ(PropertyValue<float>(315.0f), fillExtrusion->getFillExtrusionShadowAzimuth());

    // All four survive a serialize/parse round trip.
    auto value = layer->serialize();
    ASSERT_NE(nullptr, value.getObject());
    const auto& paint = *value.getObject()->at("paint").getObject();
    EXPECT_EQ(4u, paint.size());

    auto roundTripped = parseLayer(stringifyLayer(value));
    ASSERT_NE(nullptr, roundTripped);
    const auto* roundTrippedFillExtrusion = static_cast<const FillExtrusionLayer*>(roundTripped.get());
    EXPECT_EQ(fillExtrusion->getFillExtrusionShadowColor(), roundTrippedFillExtrusion->getFillExtrusionShadowColor());
    EXPECT_EQ(fillExtrusion->getFillExtrusionShadowLength(), roundTrippedFillExtrusion->getFillExtrusionShadowLength());
    EXPECT_EQ(fillExtrusion->getFillExtrusionShadowAzimuth(),
              roundTrippedFillExtrusion->getFillExtrusionShadowAzimuth());
}

TEST(StyleConversion, FillExtrusionShadowAcceptsCameraExpression) {
    auto layer = parseLayer(R"JSON({
        "type": "fill-extrusion",
        "id": "shadowed",
        "source": "composite",
        "paint": {
            "fill-extrusion-shadow-opacity": ["interpolate", ["linear"], ["zoom"], 14, 0, 18, 0.6]
        }
    })JSON");

    ASSERT_NE(nullptr, layer);
    EXPECT_TRUE(static_cast<const FillExtrusionLayer*>(layer.get())->getFillExtrusionShadowOpacity().isExpression());
}

TEST(StyleConversion, FillExtrusionShadowRejectsDataExpression) {
    // These are data-constant: one per-layer uniform buffer, no vertex attributes, no shader
    // permutations. If anyone flips `property-type` to `data-driven` in the spec, this fails and
    // sends them to the renderer to add the attribute plumbing first.
    EXPECT_EQ(nullptr, parseLayer(R"JSON({
        "type": "fill-extrusion",
        "id": "shadowed",
        "source": "composite",
        "paint": {"fill-extrusion-shadow-opacity": ["get", "shadow"]}
    })JSON"));

    EXPECT_EQ(nullptr, parseLayer(R"JSON({
        "type": "fill-extrusion",
        "id": "shadowed",
        "source": "composite",
        "paint": {"fill-extrusion-shadow-color": ["get", "shadow"]}
    })JSON"));
}

TEST(StyleConversion, FillExtrusionShadowTransition) {
    auto layer = parseLayer(R"JSON({
        "type": "fill-extrusion",
        "id": "shadowed",
        "source": "composite",
        "paint": {
            "fill-extrusion-shadow-color-transition": {
                "duration": 400,
                "delay": 500
            }
        }
    })JSON");

    ASSERT_NE(nullptr, layer);
    const auto& options =
        static_cast<const FillExtrusionLayer*>(layer.get())->impl().paint.get<FillExtrusionShadowColor>().options;
    ASSERT_EQ(400ms, *options.duration);
    ASSERT_EQ(500ms, *options.delay);
}
