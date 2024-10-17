// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/layers/line_layer_impl.hpp>
#include <mbgl/style/layer_observer.hpp>
#include <mbgl/style/conversion/color_ramp_property_value.hpp>
#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion/property_value.hpp>
#include <mbgl/style/conversion/transition_options.hpp>
#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/util/enum.hpp>

#include <mapbox/eternal.hpp>

namespace mbgl {
namespace style {


// static
const LayerTypeInfo* LineLayer::Impl::staticTypeInfo() noexcept {
    const static LayerTypeInfo typeInfo{"line",
                                        LayerTypeInfo::Source::Required,
                                        LayerTypeInfo::Pass3D::NotRequired,
                                        LayerTypeInfo::Layout::Required,
                                        LayerTypeInfo::FadingTiles::NotRequired,
                                        LayerTypeInfo::CrossTileIndex::NotRequired,
                                        LayerTypeInfo::TileKind::Geometry};
    return &typeInfo;
}


LineLayer::LineLayer(const std::string& layerID, const std::string& sourceID)
    : Layer(makeMutable<Impl>(layerID, sourceID)) {
}

LineLayer::LineLayer(Immutable<Impl> impl_)
    : Layer(std::move(impl_)) {
}

LineLayer::~LineLayer() = default;

const LineLayer::Impl& LineLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<LineLayer::Impl> LineLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

std::unique_ptr<Layer> LineLayer::cloneRef(const std::string& id_) const {
    auto impl_ = mutableImpl();
    impl_->id = id_;
    impl_->paint = LinePaintProperties::Transitionable();
    return std::make_unique<LineLayer>(std::move(impl_));
}

void LineLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>& writer) const {
    layout.stringify(writer);
}

// Layout properties

PropertyValue<LineCapType> LineLayer::getDefaultLineCap() {
    return LineCap::defaultValue();
}

const PropertyValue<LineCapType>& LineLayer::getLineCap() const {
    return impl().layout.get<LineCap>();
}

void LineLayer::setLineCap(const PropertyValue<LineCapType>& value) {
    if (value == getLineCap()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<LineCap>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<LineJoinType> LineLayer::getDefaultLineJoin() {
    return LineJoin::defaultValue();
}

const PropertyValue<LineJoinType>& LineLayer::getLineJoin() const {
    return impl().layout.get<LineJoin>();
}

void LineLayer::setLineJoin(const PropertyValue<LineJoinType>& value) {
    if (value == getLineJoin()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<LineJoin>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> LineLayer::getDefaultLineMiterLimit() {
    return LineMiterLimit::defaultValue();
}

const PropertyValue<float>& LineLayer::getLineMiterLimit() const {
    return impl().layout.get<LineMiterLimit>();
}

void LineLayer::setLineMiterLimit(const PropertyValue<float>& value) {
    if (value == getLineMiterLimit()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<LineMiterLimit>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> LineLayer::getDefaultLineRoundLimit() {
    return LineRoundLimit::defaultValue();
}

const PropertyValue<float>& LineLayer::getLineRoundLimit() const {
    return impl().layout.get<LineRoundLimit>();
}

void LineLayer::setLineRoundLimit(const PropertyValue<float>& value) {
    if (value == getLineRoundLimit()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<LineRoundLimit>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> LineLayer::getDefaultLineSortKey() {
    return LineSortKey::defaultValue();
}

const PropertyValue<float>& LineLayer::getLineSortKey() const {
    return impl().layout.get<LineSortKey>();
}

void LineLayer::setLineSortKey(const PropertyValue<float>& value) {
    if (value == getLineSortKey()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<LineSortKey>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

// Paint properties

PropertyValue<float> LineLayer::getDefaultLineBlur() {
    return {0.f};
}

const PropertyValue<float>& LineLayer::getLineBlur() const {
    return impl().paint.template get<LineBlur>().value;
}

void LineLayer::setLineBlur(const PropertyValue<float>& value) {
    if (value == getLineBlur())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineBlur>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void LineLayer::setLineBlurTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineBlur>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions LineLayer::getLineBlurTransition() const {
    return impl().paint.template get<LineBlur>().options;
}

PropertyValue<Color> LineLayer::getDefaultLineColor() {
    return {Color::black()};
}

const PropertyValue<Color>& LineLayer::getLineColor() const {
    return impl().paint.template get<LineColor>().value;
}

void LineLayer::setLineColor(const PropertyValue<Color>& value) {
    if (value == getLineColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void LineLayer::setLineColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions LineLayer::getLineColorTransition() const {
    return impl().paint.template get<LineColor>().options;
}

PropertyValue<std::vector<float>> LineLayer::getDefaultLineDasharray() {
    return {{}};
}

const PropertyValue<std::vector<float>>& LineLayer::getLineDasharray() const {
    return impl().paint.template get<LineDasharray>().value;
}

void LineLayer::setLineDasharray(const PropertyValue<std::vector<float>>& value) {
    if (value == getLineDasharray())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineDasharray>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void LineLayer::setLineDasharrayTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineDasharray>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions LineLayer::getLineDasharrayTransition() const {
    return impl().paint.template get<LineDasharray>().options;
}

PropertyValue<float> LineLayer::getDefaultLineGapWidth() {
    return {0.f};
}

const PropertyValue<float>& LineLayer::getLineGapWidth() const {
    return impl().paint.template get<LineGapWidth>().value;
}

void LineLayer::setLineGapWidth(const PropertyValue<float>& value) {
    if (value == getLineGapWidth())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineGapWidth>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void LineLayer::setLineGapWidthTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineGapWidth>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions LineLayer::getLineGapWidthTransition() const {
    return impl().paint.template get<LineGapWidth>().options;
}

ColorRampPropertyValue LineLayer::getDefaultLineGradient() {
    return {{}};
}

const ColorRampPropertyValue& LineLayer::getLineGradient() const {
    return impl().paint.template get<LineGradient>().value;
}

void LineLayer::setLineGradient(const ColorRampPropertyValue& value) {
    if (value == getLineGradient())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineGradient>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void LineLayer::setLineGradientTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineGradient>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions LineLayer::getLineGradientTransition() const {
    return impl().paint.template get<LineGradient>().options;
}

PropertyValue<float> LineLayer::getDefaultLineOffset() {
    return {0.f};
}

const PropertyValue<float>& LineLayer::getLineOffset() const {
    return impl().paint.template get<LineOffset>().value;
}

void LineLayer::setLineOffset(const PropertyValue<float>& value) {
    if (value == getLineOffset())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineOffset>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void LineLayer::setLineOffsetTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineOffset>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions LineLayer::getLineOffsetTransition() const {
    return impl().paint.template get<LineOffset>().options;
}

PropertyValue<float> LineLayer::getDefaultLineOpacity() {
    return {1.f};
}

const PropertyValue<float>& LineLayer::getLineOpacity() const {
    return impl().paint.template get<LineOpacity>().value;
}

void LineLayer::setLineOpacity(const PropertyValue<float>& value) {
    if (value == getLineOpacity())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineOpacity>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void LineLayer::setLineOpacityTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineOpacity>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions LineLayer::getLineOpacityTransition() const {
    return impl().paint.template get<LineOpacity>().options;
}

PropertyValue<expression::Image> LineLayer::getDefaultLinePattern() {
    return {{}};
}

const PropertyValue<expression::Image>& LineLayer::getLinePattern() const {
    return impl().paint.template get<LinePattern>().value;
}

void LineLayer::setLinePattern(const PropertyValue<expression::Image>& value) {
    if (value == getLinePattern())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<LinePattern>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void LineLayer::setLinePatternTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<LinePattern>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions LineLayer::getLinePatternTransition() const {
    return impl().paint.template get<LinePattern>().options;
}

PropertyValue<std::array<float, 2>> LineLayer::getDefaultLineTranslate() {
    return {{{0.f, 0.f}}};
}

const PropertyValue<std::array<float, 2>>& LineLayer::getLineTranslate() const {
    return impl().paint.template get<LineTranslate>().value;
}

void LineLayer::setLineTranslate(const PropertyValue<std::array<float, 2>>& value) {
    if (value == getLineTranslate())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineTranslate>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void LineLayer::setLineTranslateTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineTranslate>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions LineLayer::getLineTranslateTransition() const {
    return impl().paint.template get<LineTranslate>().options;
}

PropertyValue<TranslateAnchorType> LineLayer::getDefaultLineTranslateAnchor() {
    return {TranslateAnchorType::Map};
}

const PropertyValue<TranslateAnchorType>& LineLayer::getLineTranslateAnchor() const {
    return impl().paint.template get<LineTranslateAnchor>().value;
}

void LineLayer::setLineTranslateAnchor(const PropertyValue<TranslateAnchorType>& value) {
    if (value == getLineTranslateAnchor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineTranslateAnchor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void LineLayer::setLineTranslateAnchorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineTranslateAnchor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions LineLayer::getLineTranslateAnchorTransition() const {
    return impl().paint.template get<LineTranslateAnchor>().options;
}

PropertyValue<float> LineLayer::getDefaultLineWidth() {
    return {1.f};
}

const PropertyValue<float>& LineLayer::getLineWidth() const {
    return impl().paint.template get<LineWidth>().value;
}

void LineLayer::setLineWidth(const PropertyValue<float>& value) {
    if (value == getLineWidth())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineWidth>().value = value;
    impl_->paint.template get<LineFloorWidth>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void LineLayer::setLineWidthTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<LineWidth>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions LineLayer::getLineWidthTransition() const {
    return impl().paint.template get<LineWidth>().options;
}

using namespace conversion;

namespace {


constexpr uint8_t kPaintPropertyCountLine = 22u;


enum class LineProperty : uint8_t {
    LineBlur,
    LineColor,
    LineDasharray,
    LineGapWidth,
    LineGradient,
    LineOffset,
    LineOpacity,
    LinePattern,
    LineTranslate,
    LineTranslateAnchor,
    LineWidth,
    LineBlurTransition,
    LineColorTransition,
    LineDasharrayTransition,
    LineGapWidthTransition,
    LineGradientTransition,
    LineOffsetTransition,
    LineOpacityTransition,
    LinePatternTransition,
    LineTranslateTransition,
    LineTranslateAnchorTransition,
    LineWidthTransition,
    LineCap = kPaintPropertyCountLine,
    LineJoin,
    LineMiterLimit,
    LineRoundLimit,
    LineSortKey,
};


constexpr const auto lineLayerProperties = mapbox::eternal::hash_map<mapbox::eternal::string, uint8_t>(
    {{"line-blur", toUint8(LineProperty::LineBlur)},
     {"line-color", toUint8(LineProperty::LineColor)},
     {"line-dasharray", toUint8(LineProperty::LineDasharray)},
     {"line-gap-width", toUint8(LineProperty::LineGapWidth)},
     {"line-gradient", toUint8(LineProperty::LineGradient)},
     {"line-offset", toUint8(LineProperty::LineOffset)},
     {"line-opacity", toUint8(LineProperty::LineOpacity)},
     {"line-pattern", toUint8(LineProperty::LinePattern)},
     {"line-translate", toUint8(LineProperty::LineTranslate)},
     {"line-translate-anchor", toUint8(LineProperty::LineTranslateAnchor)},
     {"line-width", toUint8(LineProperty::LineWidth)},
     {"line-blur-transition", toUint8(LineProperty::LineBlurTransition)},
     {"line-color-transition", toUint8(LineProperty::LineColorTransition)},
     {"line-dasharray-transition", toUint8(LineProperty::LineDasharrayTransition)},
     {"line-gap-width-transition", toUint8(LineProperty::LineGapWidthTransition)},
     {"line-gradient-transition", toUint8(LineProperty::LineGradientTransition)},
     {"line-offset-transition", toUint8(LineProperty::LineOffsetTransition)},
     {"line-opacity-transition", toUint8(LineProperty::LineOpacityTransition)},
     {"line-pattern-transition", toUint8(LineProperty::LinePatternTransition)},
     {"line-translate-transition", toUint8(LineProperty::LineTranslateTransition)},
     {"line-translate-anchor-transition", toUint8(LineProperty::LineTranslateAnchorTransition)},
     {"line-width-transition", toUint8(LineProperty::LineWidthTransition)},
     {"line-cap", toUint8(LineProperty::LineCap)},
     {"line-join", toUint8(LineProperty::LineJoin)},
     {"line-miter-limit", toUint8(LineProperty::LineMiterLimit)},
     {"line-round-limit", toUint8(LineProperty::LineRoundLimit)},
     {"line-sort-key", toUint8(LineProperty::LineSortKey)}});

StyleProperty getLayerProperty(const LineLayer& layer, LineProperty property) {
    switch (property) {
        case LineProperty::LineBlur:
            return makeStyleProperty(layer.getLineBlur());
        case LineProperty::LineColor:
            return makeStyleProperty(layer.getLineColor());
        case LineProperty::LineDasharray:
            return makeStyleProperty(layer.getLineDasharray());
        case LineProperty::LineGapWidth:
            return makeStyleProperty(layer.getLineGapWidth());
        case LineProperty::LineGradient:
            return makeStyleProperty(layer.getLineGradient());
        case LineProperty::LineOffset:
            return makeStyleProperty(layer.getLineOffset());
        case LineProperty::LineOpacity:
            return makeStyleProperty(layer.getLineOpacity());
        case LineProperty::LinePattern:
            return makeStyleProperty(layer.getLinePattern());
        case LineProperty::LineTranslate:
            return makeStyleProperty(layer.getLineTranslate());
        case LineProperty::LineTranslateAnchor:
            return makeStyleProperty(layer.getLineTranslateAnchor());
        case LineProperty::LineWidth:
            return makeStyleProperty(layer.getLineWidth());
        case LineProperty::LineBlurTransition:
            return makeStyleProperty(layer.getLineBlurTransition());
        case LineProperty::LineColorTransition:
            return makeStyleProperty(layer.getLineColorTransition());
        case LineProperty::LineDasharrayTransition:
            return makeStyleProperty(layer.getLineDasharrayTransition());
        case LineProperty::LineGapWidthTransition:
            return makeStyleProperty(layer.getLineGapWidthTransition());
        case LineProperty::LineGradientTransition:
            return makeStyleProperty(layer.getLineGradientTransition());
        case LineProperty::LineOffsetTransition:
            return makeStyleProperty(layer.getLineOffsetTransition());
        case LineProperty::LineOpacityTransition:
            return makeStyleProperty(layer.getLineOpacityTransition());
        case LineProperty::LinePatternTransition:
            return makeStyleProperty(layer.getLinePatternTransition());
        case LineProperty::LineTranslateTransition:
            return makeStyleProperty(layer.getLineTranslateTransition());
        case LineProperty::LineTranslateAnchorTransition:
            return makeStyleProperty(layer.getLineTranslateAnchorTransition());
        case LineProperty::LineWidthTransition:
            return makeStyleProperty(layer.getLineWidthTransition());
        case LineProperty::LineCap:
            return makeStyleProperty(layer.getLineCap());
        case LineProperty::LineJoin:
            return makeStyleProperty(layer.getLineJoin());
        case LineProperty::LineMiterLimit:
            return makeStyleProperty(layer.getLineMiterLimit());
        case LineProperty::LineRoundLimit:
            return makeStyleProperty(layer.getLineRoundLimit());
        case LineProperty::LineSortKey:
            return makeStyleProperty(layer.getLineSortKey());
    }
    return {};
}

StyleProperty getLayerProperty(const LineLayer& layer, const std::string& name) {
    const auto it = lineLayerProperties.find(name.c_str());
    if (it == lineLayerProperties.end()) {
        return {};
    }
    return getLayerProperty(layer, static_cast<LineProperty>(it->second));
}

} // namespace

Value LineLayer::serialize() const {
    auto result = Layer::serialize();
    assert(result.getObject());
    for (const auto& property : lineLayerProperties) {
        auto styleProperty = getLayerProperty(*this, static_cast<LineProperty>(property.second));
        if (styleProperty.getKind() == StyleProperty::Kind::Undefined) continue;
        serializeProperty(result, styleProperty, property.first.c_str(), property.second < kPaintPropertyCountLine);
    }
    return result;
}

std::optional<Error> LineLayer::setPropertyInternal(const std::string& name, const Convertible& value) {
    const auto it = lineLayerProperties.find(name.c_str());
    if (it == lineLayerProperties.end()) return Error{"layer doesn't support this property"};

    auto property = static_cast<LineProperty>(it->second);

    if (property == LineProperty::LineBlur || property == LineProperty::LineGapWidth ||
        property == LineProperty::LineOffset || property == LineProperty::LineOpacity ||
        property == LineProperty::LineWidth || property == LineProperty::LineSortKey) {
        Error error;
        const auto& typedValue = convert<PropertyValue<float>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        if (property == LineProperty::LineBlur) {
            setLineBlur(*typedValue);
            return std::nullopt;
        }

        if (property == LineProperty::LineGapWidth) {
            setLineGapWidth(*typedValue);
            return std::nullopt;
        }

        if (property == LineProperty::LineOffset) {
            setLineOffset(*typedValue);
            return std::nullopt;
        }

        if (property == LineProperty::LineOpacity) {
            setLineOpacity(*typedValue);
            return std::nullopt;
        }

        if (property == LineProperty::LineWidth) {
            setLineWidth(*typedValue);
            return std::nullopt;
        }

        if (property == LineProperty::LineSortKey) {
            setLineSortKey(*typedValue);
            return std::nullopt;
        }
    }
    if (property == LineProperty::LineColor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<Color>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        setLineColor(*typedValue);
        return std::nullopt;
    }
    if (property == LineProperty::LineDasharray) {
        Error error;
        const auto& typedValue = convert<PropertyValue<std::vector<float>>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setLineDasharray(*typedValue);
        return std::nullopt;
    }
    if (property == LineProperty::LineGradient) {
        Error error;
        const auto& typedValue = convert<ColorRampPropertyValue>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setLineGradient(*typedValue);
        return std::nullopt;
    }
    if (property == LineProperty::LinePattern) {
        Error error;
        const auto& typedValue = convert<PropertyValue<expression::Image>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        setLinePattern(*typedValue);
        return std::nullopt;
    }
    if (property == LineProperty::LineTranslate) {
        Error error;
        const auto& typedValue = convert<PropertyValue<std::array<float, 2>>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setLineTranslate(*typedValue);
        return std::nullopt;
    }
    if (property == LineProperty::LineTranslateAnchor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<TranslateAnchorType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setLineTranslateAnchor(*typedValue);
        return std::nullopt;
    }
    if (property == LineProperty::LineCap) {
        Error error;
        const auto& typedValue = convert<PropertyValue<LineCapType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setLineCap(*typedValue);
        return std::nullopt;
    }
    if (property == LineProperty::LineJoin) {
        Error error;
        const auto& typedValue = convert<PropertyValue<LineJoinType>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        setLineJoin(*typedValue);
        return std::nullopt;
    }
    if (property == LineProperty::LineMiterLimit || property == LineProperty::LineRoundLimit) {
        Error error;
        const auto& typedValue = convert<PropertyValue<float>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        if (property == LineProperty::LineMiterLimit) {
            setLineMiterLimit(*typedValue);
            return std::nullopt;
        }

        if (property == LineProperty::LineRoundLimit) {
            setLineRoundLimit(*typedValue);
            return std::nullopt;
        }
    }

    Error error;
    std::optional<TransitionOptions> transition = convert<TransitionOptions>(value, error);
    if (!transition) {
        return error;
    }

    if (property == LineProperty::LineBlurTransition) {
        setLineBlurTransition(*transition);
        return std::nullopt;
    }

    if (property == LineProperty::LineColorTransition) {
        setLineColorTransition(*transition);
        return std::nullopt;
    }

    if (property == LineProperty::LineDasharrayTransition) {
        setLineDasharrayTransition(*transition);
        return std::nullopt;
    }

    if (property == LineProperty::LineGapWidthTransition) {
        setLineGapWidthTransition(*transition);
        return std::nullopt;
    }

    if (property == LineProperty::LineGradientTransition) {
        setLineGradientTransition(*transition);
        return std::nullopt;
    }

    if (property == LineProperty::LineOffsetTransition) {
        setLineOffsetTransition(*transition);
        return std::nullopt;
    }

    if (property == LineProperty::LineOpacityTransition) {
        setLineOpacityTransition(*transition);
        return std::nullopt;
    }

    if (property == LineProperty::LinePatternTransition) {
        setLinePatternTransition(*transition);
        return std::nullopt;
    }

    if (property == LineProperty::LineTranslateTransition) {
        setLineTranslateTransition(*transition);
        return std::nullopt;
    }

    if (property == LineProperty::LineTranslateAnchorTransition) {
        setLineTranslateAnchorTransition(*transition);
        return std::nullopt;
    }

    if (property == LineProperty::LineWidthTransition) {
        setLineWidthTransition(*transition);
        return std::nullopt;
    }

    return Error{"layer doesn't support this property"};
}

StyleProperty LineLayer::getProperty(const std::string& name) const {
    return getLayerProperty(*this, name);
}

Mutable<Layer::Impl> LineLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

} // namespace style
} // namespace mbgl

// clang-format on
