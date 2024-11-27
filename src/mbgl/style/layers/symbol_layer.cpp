// clang-format off

// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mbgl/style/layers/symbol_layer.hpp>
#include <mbgl/style/layers/symbol_layer_impl.hpp>
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
const LayerTypeInfo* SymbolLayer::Impl::staticTypeInfo() noexcept {
    const static LayerTypeInfo typeInfo{"symbol",
                                        LayerTypeInfo::Source::Required,
                                        LayerTypeInfo::Pass3D::NotRequired,
                                        LayerTypeInfo::Layout::Required,
                                        LayerTypeInfo::FadingTiles::Required,
                                        LayerTypeInfo::CrossTileIndex::Required,
                                        LayerTypeInfo::TileKind::Geometry};
    return &typeInfo;
}


SymbolLayer::SymbolLayer(const std::string& layerID, const std::string& sourceID)
    : Layer(makeMutable<Impl>(layerID, sourceID)) {
}

SymbolLayer::SymbolLayer(Immutable<Impl> impl_)
    : Layer(std::move(impl_)) {
}

SymbolLayer::~SymbolLayer() = default;

const SymbolLayer::Impl& SymbolLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<SymbolLayer::Impl> SymbolLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

std::unique_ptr<Layer> SymbolLayer::cloneRef(const std::string& id_) const {
    auto impl_ = mutableImpl();
    impl_->id = id_;
    impl_->paint = SymbolPaintProperties::Transitionable();
    return std::make_unique<SymbolLayer>(std::move(impl_));
}

void SymbolLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>& writer) const {
    layout.stringify(writer);
}

// Layout properties

PropertyValue<bool> SymbolLayer::getDefaultIconAllowOverlap() {
    return IconAllowOverlap::defaultValue();
}

const PropertyValue<bool>& SymbolLayer::getIconAllowOverlap() const {
    return impl().layout.get<IconAllowOverlap>();
}

void SymbolLayer::setIconAllowOverlap(const PropertyValue<bool>& value) {
    if (value == getIconAllowOverlap()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<IconAllowOverlap>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<SymbolAnchorType> SymbolLayer::getDefaultIconAnchor() {
    return IconAnchor::defaultValue();
}

const PropertyValue<SymbolAnchorType>& SymbolLayer::getIconAnchor() const {
    return impl().layout.get<IconAnchor>();
}

void SymbolLayer::setIconAnchor(const PropertyValue<SymbolAnchorType>& value) {
    if (value == getIconAnchor()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<IconAnchor>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<bool> SymbolLayer::getDefaultIconIgnorePlacement() {
    return IconIgnorePlacement::defaultValue();
}

const PropertyValue<bool>& SymbolLayer::getIconIgnorePlacement() const {
    return impl().layout.get<IconIgnorePlacement>();
}

void SymbolLayer::setIconIgnorePlacement(const PropertyValue<bool>& value) {
    if (value == getIconIgnorePlacement()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<IconIgnorePlacement>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<expression::Image> SymbolLayer::getDefaultIconImage() {
    return IconImage::defaultValue();
}

const PropertyValue<expression::Image>& SymbolLayer::getIconImage() const {
    return impl().layout.get<IconImage>();
}

void SymbolLayer::setIconImage(const PropertyValue<expression::Image>& value) {
    if (value == getIconImage()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<IconImage>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<bool> SymbolLayer::getDefaultIconKeepUpright() {
    return IconKeepUpright::defaultValue();
}

const PropertyValue<bool>& SymbolLayer::getIconKeepUpright() const {
    return impl().layout.get<IconKeepUpright>();
}

void SymbolLayer::setIconKeepUpright(const PropertyValue<bool>& value) {
    if (value == getIconKeepUpright()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<IconKeepUpright>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<std::array<float, 2>> SymbolLayer::getDefaultIconOffset() {
    return IconOffset::defaultValue();
}

const PropertyValue<std::array<float, 2>>& SymbolLayer::getIconOffset() const {
    return impl().layout.get<IconOffset>();
}

void SymbolLayer::setIconOffset(const PropertyValue<std::array<float, 2>>& value) {
    if (value == getIconOffset()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<IconOffset>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<bool> SymbolLayer::getDefaultIconOptional() {
    return IconOptional::defaultValue();
}

const PropertyValue<bool>& SymbolLayer::getIconOptional() const {
    return impl().layout.get<IconOptional>();
}

void SymbolLayer::setIconOptional(const PropertyValue<bool>& value) {
    if (value == getIconOptional()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<IconOptional>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<Padding> SymbolLayer::getDefaultIconPadding() {
    return IconPadding::defaultValue();
}

const PropertyValue<Padding>& SymbolLayer::getIconPadding() const {
    return impl().layout.get<IconPadding>();
}

void SymbolLayer::setIconPadding(const PropertyValue<Padding>& value) {
    if (value == getIconPadding()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<IconPadding>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<AlignmentType> SymbolLayer::getDefaultIconPitchAlignment() {
    return IconPitchAlignment::defaultValue();
}

const PropertyValue<AlignmentType>& SymbolLayer::getIconPitchAlignment() const {
    return impl().layout.get<IconPitchAlignment>();
}

void SymbolLayer::setIconPitchAlignment(const PropertyValue<AlignmentType>& value) {
    if (value == getIconPitchAlignment()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<IconPitchAlignment>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> SymbolLayer::getDefaultIconRotate() {
    return IconRotate::defaultValue();
}

const PropertyValue<float>& SymbolLayer::getIconRotate() const {
    return impl().layout.get<IconRotate>();
}

void SymbolLayer::setIconRotate(const PropertyValue<float>& value) {
    if (value == getIconRotate()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<IconRotate>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<AlignmentType> SymbolLayer::getDefaultIconRotationAlignment() {
    return IconRotationAlignment::defaultValue();
}

const PropertyValue<AlignmentType>& SymbolLayer::getIconRotationAlignment() const {
    return impl().layout.get<IconRotationAlignment>();
}

void SymbolLayer::setIconRotationAlignment(const PropertyValue<AlignmentType>& value) {
    if (value == getIconRotationAlignment()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<IconRotationAlignment>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> SymbolLayer::getDefaultIconSize() {
    return IconSize::defaultValue();
}

const PropertyValue<float>& SymbolLayer::getIconSize() const {
    return impl().layout.get<IconSize>();
}

void SymbolLayer::setIconSize(const PropertyValue<float>& value) {
    if (value == getIconSize()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<IconSize>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<IconTextFitType> SymbolLayer::getDefaultIconTextFit() {
    return IconTextFit::defaultValue();
}

const PropertyValue<IconTextFitType>& SymbolLayer::getIconTextFit() const {
    return impl().layout.get<IconTextFit>();
}

void SymbolLayer::setIconTextFit(const PropertyValue<IconTextFitType>& value) {
    if (value == getIconTextFit()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<IconTextFit>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<std::array<float, 4>> SymbolLayer::getDefaultIconTextFitPadding() {
    return IconTextFitPadding::defaultValue();
}

const PropertyValue<std::array<float, 4>>& SymbolLayer::getIconTextFitPadding() const {
    return impl().layout.get<IconTextFitPadding>();
}

void SymbolLayer::setIconTextFitPadding(const PropertyValue<std::array<float, 4>>& value) {
    if (value == getIconTextFitPadding()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<IconTextFitPadding>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<bool> SymbolLayer::getDefaultSymbolAvoidEdges() {
    return SymbolAvoidEdges::defaultValue();
}

const PropertyValue<bool>& SymbolLayer::getSymbolAvoidEdges() const {
    return impl().layout.get<SymbolAvoidEdges>();
}

void SymbolLayer::setSymbolAvoidEdges(const PropertyValue<bool>& value) {
    if (value == getSymbolAvoidEdges()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<SymbolAvoidEdges>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<SymbolPlacementType> SymbolLayer::getDefaultSymbolPlacement() {
    return SymbolPlacement::defaultValue();
}

const PropertyValue<SymbolPlacementType>& SymbolLayer::getSymbolPlacement() const {
    return impl().layout.get<SymbolPlacement>();
}

void SymbolLayer::setSymbolPlacement(const PropertyValue<SymbolPlacementType>& value) {
    if (value == getSymbolPlacement()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<SymbolPlacement>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> SymbolLayer::getDefaultSymbolSortKey() {
    return SymbolSortKey::defaultValue();
}

const PropertyValue<float>& SymbolLayer::getSymbolSortKey() const {
    return impl().layout.get<SymbolSortKey>();
}

void SymbolLayer::setSymbolSortKey(const PropertyValue<float>& value) {
    if (value == getSymbolSortKey()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<SymbolSortKey>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> SymbolLayer::getDefaultSymbolSpacing() {
    return SymbolSpacing::defaultValue();
}

const PropertyValue<float>& SymbolLayer::getSymbolSpacing() const {
    return impl().layout.get<SymbolSpacing>();
}

void SymbolLayer::setSymbolSpacing(const PropertyValue<float>& value) {
    if (value == getSymbolSpacing()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<SymbolSpacing>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<SymbolZOrderType> SymbolLayer::getDefaultSymbolZOrder() {
    return SymbolZOrder::defaultValue();
}

const PropertyValue<SymbolZOrderType>& SymbolLayer::getSymbolZOrder() const {
    return impl().layout.get<SymbolZOrder>();
}

void SymbolLayer::setSymbolZOrder(const PropertyValue<SymbolZOrderType>& value) {
    if (value == getSymbolZOrder()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<SymbolZOrder>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<bool> SymbolLayer::getDefaultTextAllowOverlap() {
    return TextAllowOverlap::defaultValue();
}

const PropertyValue<bool>& SymbolLayer::getTextAllowOverlap() const {
    return impl().layout.get<TextAllowOverlap>();
}

void SymbolLayer::setTextAllowOverlap(const PropertyValue<bool>& value) {
    if (value == getTextAllowOverlap()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextAllowOverlap>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<SymbolAnchorType> SymbolLayer::getDefaultTextAnchor() {
    return TextAnchor::defaultValue();
}

const PropertyValue<SymbolAnchorType>& SymbolLayer::getTextAnchor() const {
    return impl().layout.get<TextAnchor>();
}

void SymbolLayer::setTextAnchor(const PropertyValue<SymbolAnchorType>& value) {
    if (value == getTextAnchor()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextAnchor>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<expression::Formatted> SymbolLayer::getDefaultTextField() {
    return TextField::defaultValue();
}

const PropertyValue<expression::Formatted>& SymbolLayer::getTextField() const {
    return impl().layout.get<TextField>();
}

void SymbolLayer::setTextField(const PropertyValue<expression::Formatted>& value) {
    if (value == getTextField()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextField>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<std::vector<std::string>> SymbolLayer::getDefaultTextFont() {
    return TextFont::defaultValue();
}

const PropertyValue<std::vector<std::string>>& SymbolLayer::getTextFont() const {
    return impl().layout.get<TextFont>();
}

void SymbolLayer::setTextFont(const PropertyValue<std::vector<std::string>>& value) {
    if (value == getTextFont()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextFont>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<bool> SymbolLayer::getDefaultTextIgnorePlacement() {
    return TextIgnorePlacement::defaultValue();
}

const PropertyValue<bool>& SymbolLayer::getTextIgnorePlacement() const {
    return impl().layout.get<TextIgnorePlacement>();
}

void SymbolLayer::setTextIgnorePlacement(const PropertyValue<bool>& value) {
    if (value == getTextIgnorePlacement()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextIgnorePlacement>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<TextJustifyType> SymbolLayer::getDefaultTextJustify() {
    return TextJustify::defaultValue();
}

const PropertyValue<TextJustifyType>& SymbolLayer::getTextJustify() const {
    return impl().layout.get<TextJustify>();
}

void SymbolLayer::setTextJustify(const PropertyValue<TextJustifyType>& value) {
    if (value == getTextJustify()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextJustify>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<bool> SymbolLayer::getDefaultTextKeepUpright() {
    return TextKeepUpright::defaultValue();
}

const PropertyValue<bool>& SymbolLayer::getTextKeepUpright() const {
    return impl().layout.get<TextKeepUpright>();
}

void SymbolLayer::setTextKeepUpright(const PropertyValue<bool>& value) {
    if (value == getTextKeepUpright()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextKeepUpright>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> SymbolLayer::getDefaultTextLetterSpacing() {
    return TextLetterSpacing::defaultValue();
}

const PropertyValue<float>& SymbolLayer::getTextLetterSpacing() const {
    return impl().layout.get<TextLetterSpacing>();
}

void SymbolLayer::setTextLetterSpacing(const PropertyValue<float>& value) {
    if (value == getTextLetterSpacing()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextLetterSpacing>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> SymbolLayer::getDefaultTextLineHeight() {
    return TextLineHeight::defaultValue();
}

const PropertyValue<float>& SymbolLayer::getTextLineHeight() const {
    return impl().layout.get<TextLineHeight>();
}

void SymbolLayer::setTextLineHeight(const PropertyValue<float>& value) {
    if (value == getTextLineHeight()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextLineHeight>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> SymbolLayer::getDefaultTextMaxAngle() {
    return TextMaxAngle::defaultValue();
}

const PropertyValue<float>& SymbolLayer::getTextMaxAngle() const {
    return impl().layout.get<TextMaxAngle>();
}

void SymbolLayer::setTextMaxAngle(const PropertyValue<float>& value) {
    if (value == getTextMaxAngle()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextMaxAngle>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> SymbolLayer::getDefaultTextMaxWidth() {
    return TextMaxWidth::defaultValue();
}

const PropertyValue<float>& SymbolLayer::getTextMaxWidth() const {
    return impl().layout.get<TextMaxWidth>();
}

void SymbolLayer::setTextMaxWidth(const PropertyValue<float>& value) {
    if (value == getTextMaxWidth()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextMaxWidth>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<std::array<float, 2>> SymbolLayer::getDefaultTextOffset() {
    return TextOffset::defaultValue();
}

const PropertyValue<std::array<float, 2>>& SymbolLayer::getTextOffset() const {
    return impl().layout.get<TextOffset>();
}

void SymbolLayer::setTextOffset(const PropertyValue<std::array<float, 2>>& value) {
    if (value == getTextOffset()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextOffset>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<bool> SymbolLayer::getDefaultTextOptional() {
    return TextOptional::defaultValue();
}

const PropertyValue<bool>& SymbolLayer::getTextOptional() const {
    return impl().layout.get<TextOptional>();
}

void SymbolLayer::setTextOptional(const PropertyValue<bool>& value) {
    if (value == getTextOptional()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextOptional>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> SymbolLayer::getDefaultTextPadding() {
    return TextPadding::defaultValue();
}

const PropertyValue<float>& SymbolLayer::getTextPadding() const {
    return impl().layout.get<TextPadding>();
}

void SymbolLayer::setTextPadding(const PropertyValue<float>& value) {
    if (value == getTextPadding()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextPadding>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<AlignmentType> SymbolLayer::getDefaultTextPitchAlignment() {
    return TextPitchAlignment::defaultValue();
}

const PropertyValue<AlignmentType>& SymbolLayer::getTextPitchAlignment() const {
    return impl().layout.get<TextPitchAlignment>();
}

void SymbolLayer::setTextPitchAlignment(const PropertyValue<AlignmentType>& value) {
    if (value == getTextPitchAlignment()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextPitchAlignment>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> SymbolLayer::getDefaultTextRadialOffset() {
    return TextRadialOffset::defaultValue();
}

const PropertyValue<float>& SymbolLayer::getTextRadialOffset() const {
    return impl().layout.get<TextRadialOffset>();
}

void SymbolLayer::setTextRadialOffset(const PropertyValue<float>& value) {
    if (value == getTextRadialOffset()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextRadialOffset>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> SymbolLayer::getDefaultTextRotate() {
    return TextRotate::defaultValue();
}

const PropertyValue<float>& SymbolLayer::getTextRotate() const {
    return impl().layout.get<TextRotate>();
}

void SymbolLayer::setTextRotate(const PropertyValue<float>& value) {
    if (value == getTextRotate()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextRotate>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<AlignmentType> SymbolLayer::getDefaultTextRotationAlignment() {
    return TextRotationAlignment::defaultValue();
}

const PropertyValue<AlignmentType>& SymbolLayer::getTextRotationAlignment() const {
    return impl().layout.get<TextRotationAlignment>();
}

void SymbolLayer::setTextRotationAlignment(const PropertyValue<AlignmentType>& value) {
    if (value == getTextRotationAlignment()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextRotationAlignment>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<float> SymbolLayer::getDefaultTextSize() {
    return TextSize::defaultValue();
}

const PropertyValue<float>& SymbolLayer::getTextSize() const {
    return impl().layout.get<TextSize>();
}

void SymbolLayer::setTextSize(const PropertyValue<float>& value) {
    if (value == getTextSize()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextSize>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<TextTransformType> SymbolLayer::getDefaultTextTransform() {
    return TextTransform::defaultValue();
}

const PropertyValue<TextTransformType>& SymbolLayer::getTextTransform() const {
    return impl().layout.get<TextTransform>();
}

void SymbolLayer::setTextTransform(const PropertyValue<TextTransformType>& value) {
    if (value == getTextTransform()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextTransform>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<std::vector<TextVariableAnchorType>> SymbolLayer::getDefaultTextVariableAnchor() {
    return TextVariableAnchor::defaultValue();
}

const PropertyValue<std::vector<TextVariableAnchorType>>& SymbolLayer::getTextVariableAnchor() const {
    return impl().layout.get<TextVariableAnchor>();
}

void SymbolLayer::setTextVariableAnchor(const PropertyValue<std::vector<TextVariableAnchorType>>& value) {
    if (value == getTextVariableAnchor()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextVariableAnchor>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<VariableAnchorOffsetCollection> SymbolLayer::getDefaultTextVariableAnchorOffset() {
    return TextVariableAnchorOffset::defaultValue();
}

const PropertyValue<VariableAnchorOffsetCollection>& SymbolLayer::getTextVariableAnchorOffset() const {
    return impl().layout.get<TextVariableAnchorOffset>();
}

void SymbolLayer::setTextVariableAnchorOffset(const PropertyValue<VariableAnchorOffsetCollection>& value) {
    if (value == getTextVariableAnchorOffset()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextVariableAnchorOffset>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}
PropertyValue<std::vector<TextWritingModeType>> SymbolLayer::getDefaultTextWritingMode() {
    return TextWritingMode::defaultValue();
}

const PropertyValue<std::vector<TextWritingModeType>>& SymbolLayer::getTextWritingMode() const {
    return impl().layout.get<TextWritingMode>();
}

void SymbolLayer::setTextWritingMode(const PropertyValue<std::vector<TextWritingModeType>>& value) {
    if (value == getTextWritingMode()) return;
    auto impl_ = mutableImpl();
    impl_->layout.get<TextWritingMode>() = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

// Paint properties

PropertyValue<Color> SymbolLayer::getDefaultIconColor() {
    return {Color::black()};
}

const PropertyValue<Color>& SymbolLayer::getIconColor() const {
    return impl().paint.template get<IconColor>().value;
}

void SymbolLayer::setIconColor(const PropertyValue<Color>& value) {
    if (value == getIconColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<IconColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void SymbolLayer::setIconColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<IconColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions SymbolLayer::getIconColorTransition() const {
    return impl().paint.template get<IconColor>().options;
}

PropertyValue<float> SymbolLayer::getDefaultIconHaloBlur() {
    return {0.f};
}

const PropertyValue<float>& SymbolLayer::getIconHaloBlur() const {
    return impl().paint.template get<IconHaloBlur>().value;
}

void SymbolLayer::setIconHaloBlur(const PropertyValue<float>& value) {
    if (value == getIconHaloBlur())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<IconHaloBlur>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void SymbolLayer::setIconHaloBlurTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<IconHaloBlur>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions SymbolLayer::getIconHaloBlurTransition() const {
    return impl().paint.template get<IconHaloBlur>().options;
}

PropertyValue<Color> SymbolLayer::getDefaultIconHaloColor() {
    return {{}};
}

const PropertyValue<Color>& SymbolLayer::getIconHaloColor() const {
    return impl().paint.template get<IconHaloColor>().value;
}

void SymbolLayer::setIconHaloColor(const PropertyValue<Color>& value) {
    if (value == getIconHaloColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<IconHaloColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void SymbolLayer::setIconHaloColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<IconHaloColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions SymbolLayer::getIconHaloColorTransition() const {
    return impl().paint.template get<IconHaloColor>().options;
}

PropertyValue<float> SymbolLayer::getDefaultIconHaloWidth() {
    return {0.f};
}

const PropertyValue<float>& SymbolLayer::getIconHaloWidth() const {
    return impl().paint.template get<IconHaloWidth>().value;
}

void SymbolLayer::setIconHaloWidth(const PropertyValue<float>& value) {
    if (value == getIconHaloWidth())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<IconHaloWidth>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void SymbolLayer::setIconHaloWidthTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<IconHaloWidth>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions SymbolLayer::getIconHaloWidthTransition() const {
    return impl().paint.template get<IconHaloWidth>().options;
}

PropertyValue<float> SymbolLayer::getDefaultIconOpacity() {
    return {1.f};
}

const PropertyValue<float>& SymbolLayer::getIconOpacity() const {
    return impl().paint.template get<IconOpacity>().value;
}

void SymbolLayer::setIconOpacity(const PropertyValue<float>& value) {
    if (value == getIconOpacity())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<IconOpacity>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void SymbolLayer::setIconOpacityTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<IconOpacity>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions SymbolLayer::getIconOpacityTransition() const {
    return impl().paint.template get<IconOpacity>().options;
}

PropertyValue<std::array<float, 2>> SymbolLayer::getDefaultIconTranslate() {
    return {{{0.f, 0.f}}};
}

const PropertyValue<std::array<float, 2>>& SymbolLayer::getIconTranslate() const {
    return impl().paint.template get<IconTranslate>().value;
}

void SymbolLayer::setIconTranslate(const PropertyValue<std::array<float, 2>>& value) {
    if (value == getIconTranslate())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<IconTranslate>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void SymbolLayer::setIconTranslateTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<IconTranslate>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions SymbolLayer::getIconTranslateTransition() const {
    return impl().paint.template get<IconTranslate>().options;
}

PropertyValue<TranslateAnchorType> SymbolLayer::getDefaultIconTranslateAnchor() {
    return {TranslateAnchorType::Map};
}

const PropertyValue<TranslateAnchorType>& SymbolLayer::getIconTranslateAnchor() const {
    return impl().paint.template get<IconTranslateAnchor>().value;
}

void SymbolLayer::setIconTranslateAnchor(const PropertyValue<TranslateAnchorType>& value) {
    if (value == getIconTranslateAnchor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<IconTranslateAnchor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void SymbolLayer::setIconTranslateAnchorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<IconTranslateAnchor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions SymbolLayer::getIconTranslateAnchorTransition() const {
    return impl().paint.template get<IconTranslateAnchor>().options;
}

PropertyValue<Color> SymbolLayer::getDefaultTextColor() {
    return {Color::black()};
}

const PropertyValue<Color>& SymbolLayer::getTextColor() const {
    return impl().paint.template get<TextColor>().value;
}

void SymbolLayer::setTextColor(const PropertyValue<Color>& value) {
    if (value == getTextColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<TextColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void SymbolLayer::setTextColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<TextColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions SymbolLayer::getTextColorTransition() const {
    return impl().paint.template get<TextColor>().options;
}

PropertyValue<float> SymbolLayer::getDefaultTextHaloBlur() {
    return {0.f};
}

const PropertyValue<float>& SymbolLayer::getTextHaloBlur() const {
    return impl().paint.template get<TextHaloBlur>().value;
}

void SymbolLayer::setTextHaloBlur(const PropertyValue<float>& value) {
    if (value == getTextHaloBlur())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<TextHaloBlur>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void SymbolLayer::setTextHaloBlurTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<TextHaloBlur>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions SymbolLayer::getTextHaloBlurTransition() const {
    return impl().paint.template get<TextHaloBlur>().options;
}

PropertyValue<Color> SymbolLayer::getDefaultTextHaloColor() {
    return {{}};
}

const PropertyValue<Color>& SymbolLayer::getTextHaloColor() const {
    return impl().paint.template get<TextHaloColor>().value;
}

void SymbolLayer::setTextHaloColor(const PropertyValue<Color>& value) {
    if (value == getTextHaloColor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<TextHaloColor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void SymbolLayer::setTextHaloColorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<TextHaloColor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions SymbolLayer::getTextHaloColorTransition() const {
    return impl().paint.template get<TextHaloColor>().options;
}

PropertyValue<float> SymbolLayer::getDefaultTextHaloWidth() {
    return {0.f};
}

const PropertyValue<float>& SymbolLayer::getTextHaloWidth() const {
    return impl().paint.template get<TextHaloWidth>().value;
}

void SymbolLayer::setTextHaloWidth(const PropertyValue<float>& value) {
    if (value == getTextHaloWidth())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<TextHaloWidth>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void SymbolLayer::setTextHaloWidthTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<TextHaloWidth>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions SymbolLayer::getTextHaloWidthTransition() const {
    return impl().paint.template get<TextHaloWidth>().options;
}

PropertyValue<float> SymbolLayer::getDefaultTextOpacity() {
    return {1.f};
}

const PropertyValue<float>& SymbolLayer::getTextOpacity() const {
    return impl().paint.template get<TextOpacity>().value;
}

void SymbolLayer::setTextOpacity(const PropertyValue<float>& value) {
    if (value == getTextOpacity())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<TextOpacity>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void SymbolLayer::setTextOpacityTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<TextOpacity>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions SymbolLayer::getTextOpacityTransition() const {
    return impl().paint.template get<TextOpacity>().options;
}

PropertyValue<std::array<float, 2>> SymbolLayer::getDefaultTextTranslate() {
    return {{{0.f, 0.f}}};
}

const PropertyValue<std::array<float, 2>>& SymbolLayer::getTextTranslate() const {
    return impl().paint.template get<TextTranslate>().value;
}

void SymbolLayer::setTextTranslate(const PropertyValue<std::array<float, 2>>& value) {
    if (value == getTextTranslate())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<TextTranslate>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void SymbolLayer::setTextTranslateTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<TextTranslate>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions SymbolLayer::getTextTranslateTransition() const {
    return impl().paint.template get<TextTranslate>().options;
}

PropertyValue<TranslateAnchorType> SymbolLayer::getDefaultTextTranslateAnchor() {
    return {TranslateAnchorType::Map};
}

const PropertyValue<TranslateAnchorType>& SymbolLayer::getTextTranslateAnchor() const {
    return impl().paint.template get<TextTranslateAnchor>().value;
}

void SymbolLayer::setTextTranslateAnchor(const PropertyValue<TranslateAnchorType>& value) {
    if (value == getTextTranslateAnchor())
        return;
    auto impl_ = mutableImpl();
    impl_->paint.template get<TextTranslateAnchor>().value = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void SymbolLayer::setTextTranslateAnchorTransition(const TransitionOptions& options) {
    auto impl_ = mutableImpl();
    impl_->paint.template get<TextTranslateAnchor>().options = options;
    baseImpl = std::move(impl_);
}

TransitionOptions SymbolLayer::getTextTranslateAnchorTransition() const {
    return impl().paint.template get<TextTranslateAnchor>().options;
}

using namespace conversion;

namespace {


constexpr uint8_t kPaintPropertyCountSymbol = 28u;


enum class SymbolProperty : uint8_t {
    IconColor,
    IconHaloBlur,
    IconHaloColor,
    IconHaloWidth,
    IconOpacity,
    IconTranslate,
    IconTranslateAnchor,
    TextColor,
    TextHaloBlur,
    TextHaloColor,
    TextHaloWidth,
    TextOpacity,
    TextTranslate,
    TextTranslateAnchor,
    IconColorTransition,
    IconHaloBlurTransition,
    IconHaloColorTransition,
    IconHaloWidthTransition,
    IconOpacityTransition,
    IconTranslateTransition,
    IconTranslateAnchorTransition,
    TextColorTransition,
    TextHaloBlurTransition,
    TextHaloColorTransition,
    TextHaloWidthTransition,
    TextOpacityTransition,
    TextTranslateTransition,
    TextTranslateAnchorTransition,
    IconAllowOverlap = kPaintPropertyCountSymbol,
    IconAnchor,
    IconIgnorePlacement,
    IconImage,
    IconKeepUpright,
    IconOffset,
    IconOptional,
    IconPadding,
    IconPitchAlignment,
    IconRotate,
    IconRotationAlignment,
    IconSize,
    IconTextFit,
    IconTextFitPadding,
    SymbolAvoidEdges,
    SymbolPlacement,
    SymbolSortKey,
    SymbolSpacing,
    SymbolZOrder,
    TextAllowOverlap,
    TextAnchor,
    TextField,
    TextFont,
    TextIgnorePlacement,
    TextJustify,
    TextKeepUpright,
    TextLetterSpacing,
    TextLineHeight,
    TextMaxAngle,
    TextMaxWidth,
    TextOffset,
    TextOptional,
    TextPadding,
    TextPitchAlignment,
    TextRadialOffset,
    TextRotate,
    TextRotationAlignment,
    TextSize,
    TextTransform,
    TextVariableAnchor,
    TextVariableAnchorOffset,
    TextWritingMode,
};


constexpr const auto symbolLayerProperties = mapbox::eternal::hash_map<mapbox::eternal::string, uint8_t>(
    {{"icon-color", toUint8(SymbolProperty::IconColor)},
     {"icon-halo-blur", toUint8(SymbolProperty::IconHaloBlur)},
     {"icon-halo-color", toUint8(SymbolProperty::IconHaloColor)},
     {"icon-halo-width", toUint8(SymbolProperty::IconHaloWidth)},
     {"icon-opacity", toUint8(SymbolProperty::IconOpacity)},
     {"icon-translate", toUint8(SymbolProperty::IconTranslate)},
     {"icon-translate-anchor", toUint8(SymbolProperty::IconTranslateAnchor)},
     {"text-color", toUint8(SymbolProperty::TextColor)},
     {"text-halo-blur", toUint8(SymbolProperty::TextHaloBlur)},
     {"text-halo-color", toUint8(SymbolProperty::TextHaloColor)},
     {"text-halo-width", toUint8(SymbolProperty::TextHaloWidth)},
     {"text-opacity", toUint8(SymbolProperty::TextOpacity)},
     {"text-translate", toUint8(SymbolProperty::TextTranslate)},
     {"text-translate-anchor", toUint8(SymbolProperty::TextTranslateAnchor)},
     {"icon-color-transition", toUint8(SymbolProperty::IconColorTransition)},
     {"icon-halo-blur-transition", toUint8(SymbolProperty::IconHaloBlurTransition)},
     {"icon-halo-color-transition", toUint8(SymbolProperty::IconHaloColorTransition)},
     {"icon-halo-width-transition", toUint8(SymbolProperty::IconHaloWidthTransition)},
     {"icon-opacity-transition", toUint8(SymbolProperty::IconOpacityTransition)},
     {"icon-translate-transition", toUint8(SymbolProperty::IconTranslateTransition)},
     {"icon-translate-anchor-transition", toUint8(SymbolProperty::IconTranslateAnchorTransition)},
     {"text-color-transition", toUint8(SymbolProperty::TextColorTransition)},
     {"text-halo-blur-transition", toUint8(SymbolProperty::TextHaloBlurTransition)},
     {"text-halo-color-transition", toUint8(SymbolProperty::TextHaloColorTransition)},
     {"text-halo-width-transition", toUint8(SymbolProperty::TextHaloWidthTransition)},
     {"text-opacity-transition", toUint8(SymbolProperty::TextOpacityTransition)},
     {"text-translate-transition", toUint8(SymbolProperty::TextTranslateTransition)},
     {"text-translate-anchor-transition", toUint8(SymbolProperty::TextTranslateAnchorTransition)},
     {"icon-allow-overlap", toUint8(SymbolProperty::IconAllowOverlap)},
     {"icon-anchor", toUint8(SymbolProperty::IconAnchor)},
     {"icon-ignore-placement", toUint8(SymbolProperty::IconIgnorePlacement)},
     {"icon-image", toUint8(SymbolProperty::IconImage)},
     {"icon-keep-upright", toUint8(SymbolProperty::IconKeepUpright)},
     {"icon-offset", toUint8(SymbolProperty::IconOffset)},
     {"icon-optional", toUint8(SymbolProperty::IconOptional)},
     {"icon-padding", toUint8(SymbolProperty::IconPadding)},
     {"icon-pitch-alignment", toUint8(SymbolProperty::IconPitchAlignment)},
     {"icon-rotate", toUint8(SymbolProperty::IconRotate)},
     {"icon-rotation-alignment", toUint8(SymbolProperty::IconRotationAlignment)},
     {"icon-size", toUint8(SymbolProperty::IconSize)},
     {"icon-text-fit", toUint8(SymbolProperty::IconTextFit)},
     {"icon-text-fit-padding", toUint8(SymbolProperty::IconTextFitPadding)},
     {"symbol-avoid-edges", toUint8(SymbolProperty::SymbolAvoidEdges)},
     {"symbol-placement", toUint8(SymbolProperty::SymbolPlacement)},
     {"symbol-sort-key", toUint8(SymbolProperty::SymbolSortKey)},
     {"symbol-spacing", toUint8(SymbolProperty::SymbolSpacing)},
     {"symbol-z-order", toUint8(SymbolProperty::SymbolZOrder)},
     {"text-allow-overlap", toUint8(SymbolProperty::TextAllowOverlap)},
     {"text-anchor", toUint8(SymbolProperty::TextAnchor)},
     {"text-field", toUint8(SymbolProperty::TextField)},
     {"text-font", toUint8(SymbolProperty::TextFont)},
     {"text-ignore-placement", toUint8(SymbolProperty::TextIgnorePlacement)},
     {"text-justify", toUint8(SymbolProperty::TextJustify)},
     {"text-keep-upright", toUint8(SymbolProperty::TextKeepUpright)},
     {"text-letter-spacing", toUint8(SymbolProperty::TextLetterSpacing)},
     {"text-line-height", toUint8(SymbolProperty::TextLineHeight)},
     {"text-max-angle", toUint8(SymbolProperty::TextMaxAngle)},
     {"text-max-width", toUint8(SymbolProperty::TextMaxWidth)},
     {"text-offset", toUint8(SymbolProperty::TextOffset)},
     {"text-optional", toUint8(SymbolProperty::TextOptional)},
     {"text-padding", toUint8(SymbolProperty::TextPadding)},
     {"text-pitch-alignment", toUint8(SymbolProperty::TextPitchAlignment)},
     {"text-radial-offset", toUint8(SymbolProperty::TextRadialOffset)},
     {"text-rotate", toUint8(SymbolProperty::TextRotate)},
     {"text-rotation-alignment", toUint8(SymbolProperty::TextRotationAlignment)},
     {"text-size", toUint8(SymbolProperty::TextSize)},
     {"text-transform", toUint8(SymbolProperty::TextTransform)},
     {"text-variable-anchor", toUint8(SymbolProperty::TextVariableAnchor)},
     {"text-variable-anchor-offset", toUint8(SymbolProperty::TextVariableAnchorOffset)},
     {"text-writing-mode", toUint8(SymbolProperty::TextWritingMode)}});

StyleProperty getLayerProperty(const SymbolLayer& layer, SymbolProperty property) {
    switch (property) {
        case SymbolProperty::IconColor:
            return makeStyleProperty(layer.getIconColor());
        case SymbolProperty::IconHaloBlur:
            return makeStyleProperty(layer.getIconHaloBlur());
        case SymbolProperty::IconHaloColor:
            return makeStyleProperty(layer.getIconHaloColor());
        case SymbolProperty::IconHaloWidth:
            return makeStyleProperty(layer.getIconHaloWidth());
        case SymbolProperty::IconOpacity:
            return makeStyleProperty(layer.getIconOpacity());
        case SymbolProperty::IconTranslate:
            return makeStyleProperty(layer.getIconTranslate());
        case SymbolProperty::IconTranslateAnchor:
            return makeStyleProperty(layer.getIconTranslateAnchor());
        case SymbolProperty::TextColor:
            return makeStyleProperty(layer.getTextColor());
        case SymbolProperty::TextHaloBlur:
            return makeStyleProperty(layer.getTextHaloBlur());
        case SymbolProperty::TextHaloColor:
            return makeStyleProperty(layer.getTextHaloColor());
        case SymbolProperty::TextHaloWidth:
            return makeStyleProperty(layer.getTextHaloWidth());
        case SymbolProperty::TextOpacity:
            return makeStyleProperty(layer.getTextOpacity());
        case SymbolProperty::TextTranslate:
            return makeStyleProperty(layer.getTextTranslate());
        case SymbolProperty::TextTranslateAnchor:
            return makeStyleProperty(layer.getTextTranslateAnchor());
        case SymbolProperty::IconColorTransition:
            return makeStyleProperty(layer.getIconColorTransition());
        case SymbolProperty::IconHaloBlurTransition:
            return makeStyleProperty(layer.getIconHaloBlurTransition());
        case SymbolProperty::IconHaloColorTransition:
            return makeStyleProperty(layer.getIconHaloColorTransition());
        case SymbolProperty::IconHaloWidthTransition:
            return makeStyleProperty(layer.getIconHaloWidthTransition());
        case SymbolProperty::IconOpacityTransition:
            return makeStyleProperty(layer.getIconOpacityTransition());
        case SymbolProperty::IconTranslateTransition:
            return makeStyleProperty(layer.getIconTranslateTransition());
        case SymbolProperty::IconTranslateAnchorTransition:
            return makeStyleProperty(layer.getIconTranslateAnchorTransition());
        case SymbolProperty::TextColorTransition:
            return makeStyleProperty(layer.getTextColorTransition());
        case SymbolProperty::TextHaloBlurTransition:
            return makeStyleProperty(layer.getTextHaloBlurTransition());
        case SymbolProperty::TextHaloColorTransition:
            return makeStyleProperty(layer.getTextHaloColorTransition());
        case SymbolProperty::TextHaloWidthTransition:
            return makeStyleProperty(layer.getTextHaloWidthTransition());
        case SymbolProperty::TextOpacityTransition:
            return makeStyleProperty(layer.getTextOpacityTransition());
        case SymbolProperty::TextTranslateTransition:
            return makeStyleProperty(layer.getTextTranslateTransition());
        case SymbolProperty::TextTranslateAnchorTransition:
            return makeStyleProperty(layer.getTextTranslateAnchorTransition());
        case SymbolProperty::IconAllowOverlap:
            return makeStyleProperty(layer.getIconAllowOverlap());
        case SymbolProperty::IconAnchor:
            return makeStyleProperty(layer.getIconAnchor());
        case SymbolProperty::IconIgnorePlacement:
            return makeStyleProperty(layer.getIconIgnorePlacement());
        case SymbolProperty::IconImage:
            return makeStyleProperty(layer.getIconImage());
        case SymbolProperty::IconKeepUpright:
            return makeStyleProperty(layer.getIconKeepUpright());
        case SymbolProperty::IconOffset:
            return makeStyleProperty(layer.getIconOffset());
        case SymbolProperty::IconOptional:
            return makeStyleProperty(layer.getIconOptional());
        case SymbolProperty::IconPadding:
            return makeStyleProperty(layer.getIconPadding());
        case SymbolProperty::IconPitchAlignment:
            return makeStyleProperty(layer.getIconPitchAlignment());
        case SymbolProperty::IconRotate:
            return makeStyleProperty(layer.getIconRotate());
        case SymbolProperty::IconRotationAlignment:
            return makeStyleProperty(layer.getIconRotationAlignment());
        case SymbolProperty::IconSize:
            return makeStyleProperty(layer.getIconSize());
        case SymbolProperty::IconTextFit:
            return makeStyleProperty(layer.getIconTextFit());
        case SymbolProperty::IconTextFitPadding:
            return makeStyleProperty(layer.getIconTextFitPadding());
        case SymbolProperty::SymbolAvoidEdges:
            return makeStyleProperty(layer.getSymbolAvoidEdges());
        case SymbolProperty::SymbolPlacement:
            return makeStyleProperty(layer.getSymbolPlacement());
        case SymbolProperty::SymbolSortKey:
            return makeStyleProperty(layer.getSymbolSortKey());
        case SymbolProperty::SymbolSpacing:
            return makeStyleProperty(layer.getSymbolSpacing());
        case SymbolProperty::SymbolZOrder:
            return makeStyleProperty(layer.getSymbolZOrder());
        case SymbolProperty::TextAllowOverlap:
            return makeStyleProperty(layer.getTextAllowOverlap());
        case SymbolProperty::TextAnchor:
            return makeStyleProperty(layer.getTextAnchor());
        case SymbolProperty::TextField:
            return makeStyleProperty(layer.getTextField());
        case SymbolProperty::TextFont:
            return makeStyleProperty(layer.getTextFont());
        case SymbolProperty::TextIgnorePlacement:
            return makeStyleProperty(layer.getTextIgnorePlacement());
        case SymbolProperty::TextJustify:
            return makeStyleProperty(layer.getTextJustify());
        case SymbolProperty::TextKeepUpright:
            return makeStyleProperty(layer.getTextKeepUpright());
        case SymbolProperty::TextLetterSpacing:
            return makeStyleProperty(layer.getTextLetterSpacing());
        case SymbolProperty::TextLineHeight:
            return makeStyleProperty(layer.getTextLineHeight());
        case SymbolProperty::TextMaxAngle:
            return makeStyleProperty(layer.getTextMaxAngle());
        case SymbolProperty::TextMaxWidth:
            return makeStyleProperty(layer.getTextMaxWidth());
        case SymbolProperty::TextOffset:
            return makeStyleProperty(layer.getTextOffset());
        case SymbolProperty::TextOptional:
            return makeStyleProperty(layer.getTextOptional());
        case SymbolProperty::TextPadding:
            return makeStyleProperty(layer.getTextPadding());
        case SymbolProperty::TextPitchAlignment:
            return makeStyleProperty(layer.getTextPitchAlignment());
        case SymbolProperty::TextRadialOffset:
            return makeStyleProperty(layer.getTextRadialOffset());
        case SymbolProperty::TextRotate:
            return makeStyleProperty(layer.getTextRotate());
        case SymbolProperty::TextRotationAlignment:
            return makeStyleProperty(layer.getTextRotationAlignment());
        case SymbolProperty::TextSize:
            return makeStyleProperty(layer.getTextSize());
        case SymbolProperty::TextTransform:
            return makeStyleProperty(layer.getTextTransform());
        case SymbolProperty::TextVariableAnchor:
            return makeStyleProperty(layer.getTextVariableAnchor());
        case SymbolProperty::TextVariableAnchorOffset:
            return makeStyleProperty(layer.getTextVariableAnchorOffset());
        case SymbolProperty::TextWritingMode:
            return makeStyleProperty(layer.getTextWritingMode());
    }
    return {};
}

StyleProperty getLayerProperty(const SymbolLayer& layer, const std::string& name) {
    const auto it = symbolLayerProperties.find(name.c_str());
    if (it == symbolLayerProperties.end()) {
        return {};
    }
    return getLayerProperty(layer, static_cast<SymbolProperty>(it->second));
}

} // namespace

Value SymbolLayer::serialize() const {
    auto result = Layer::serialize();
    assert(result.getObject());
    for (const auto& property : symbolLayerProperties) {
        auto styleProperty = getLayerProperty(*this, static_cast<SymbolProperty>(property.second));
        if (styleProperty.getKind() == StyleProperty::Kind::Undefined) continue;
        serializeProperty(result, styleProperty, property.first.c_str(), property.second < kPaintPropertyCountSymbol);
    }
    return result;
}

std::optional<Error> SymbolLayer::setPropertyInternal(const std::string& name, const Convertible& value) {
    const auto it = symbolLayerProperties.find(name.c_str());
    if (it == symbolLayerProperties.end()) return Error{"layer doesn't support this property"};

    auto property = static_cast<SymbolProperty>(it->second);

    if (property == SymbolProperty::IconColor || property == SymbolProperty::IconHaloColor ||
        property == SymbolProperty::TextColor || property == SymbolProperty::TextHaloColor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<Color>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        if (property == SymbolProperty::IconColor) {
            setIconColor(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::IconHaloColor) {
            setIconHaloColor(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextColor) {
            setTextColor(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextHaloColor) {
            setTextHaloColor(*typedValue);
            return std::nullopt;
        }
    }
    if (property == SymbolProperty::IconHaloBlur || property == SymbolProperty::IconHaloWidth ||
        property == SymbolProperty::IconOpacity || property == SymbolProperty::TextHaloBlur ||
        property == SymbolProperty::TextHaloWidth || property == SymbolProperty::TextOpacity ||
        property == SymbolProperty::IconRotate || property == SymbolProperty::IconSize ||
        property == SymbolProperty::SymbolSortKey || property == SymbolProperty::TextLetterSpacing ||
        property == SymbolProperty::TextMaxWidth || property == SymbolProperty::TextRadialOffset ||
        property == SymbolProperty::TextRotate || property == SymbolProperty::TextSize) {
        Error error;
        const auto& typedValue = convert<PropertyValue<float>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        if (property == SymbolProperty::IconHaloBlur) {
            setIconHaloBlur(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::IconHaloWidth) {
            setIconHaloWidth(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::IconOpacity) {
            setIconOpacity(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextHaloBlur) {
            setTextHaloBlur(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextHaloWidth) {
            setTextHaloWidth(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextOpacity) {
            setTextOpacity(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::IconRotate) {
            setIconRotate(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::IconSize) {
            setIconSize(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::SymbolSortKey) {
            setSymbolSortKey(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextLetterSpacing) {
            setTextLetterSpacing(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextMaxWidth) {
            setTextMaxWidth(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextRadialOffset) {
            setTextRadialOffset(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextRotate) {
            setTextRotate(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextSize) {
            setTextSize(*typedValue);
            return std::nullopt;
        }
    }
    if (property == SymbolProperty::IconTranslate || property == SymbolProperty::TextTranslate) {
        Error error;
        const auto& typedValue = convert<PropertyValue<std::array<float, 2>>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        if (property == SymbolProperty::IconTranslate) {
            setIconTranslate(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextTranslate) {
            setTextTranslate(*typedValue);
            return std::nullopt;
        }
    }
    if (property == SymbolProperty::IconTranslateAnchor || property == SymbolProperty::TextTranslateAnchor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<TranslateAnchorType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        if (property == SymbolProperty::IconTranslateAnchor) {
            setIconTranslateAnchor(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextTranslateAnchor) {
            setTextTranslateAnchor(*typedValue);
            return std::nullopt;
        }
    }
    if (property == SymbolProperty::IconAllowOverlap || property == SymbolProperty::IconIgnorePlacement ||
        property == SymbolProperty::IconKeepUpright || property == SymbolProperty::IconOptional ||
        property == SymbolProperty::SymbolAvoidEdges || property == SymbolProperty::TextAllowOverlap ||
        property == SymbolProperty::TextIgnorePlacement || property == SymbolProperty::TextKeepUpright ||
        property == SymbolProperty::TextOptional) {
        Error error;
        const auto& typedValue = convert<PropertyValue<bool>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        if (property == SymbolProperty::IconAllowOverlap) {
            setIconAllowOverlap(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::IconIgnorePlacement) {
            setIconIgnorePlacement(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::IconKeepUpright) {
            setIconKeepUpright(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::IconOptional) {
            setIconOptional(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::SymbolAvoidEdges) {
            setSymbolAvoidEdges(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextAllowOverlap) {
            setTextAllowOverlap(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextIgnorePlacement) {
            setTextIgnorePlacement(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextKeepUpright) {
            setTextKeepUpright(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextOptional) {
            setTextOptional(*typedValue);
            return std::nullopt;
        }
    }
    if (property == SymbolProperty::IconAnchor || property == SymbolProperty::TextAnchor) {
        Error error;
        const auto& typedValue = convert<PropertyValue<SymbolAnchorType>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        if (property == SymbolProperty::IconAnchor) {
            setIconAnchor(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextAnchor) {
            setTextAnchor(*typedValue);
            return std::nullopt;
        }
    }
    if (property == SymbolProperty::IconImage) {
        Error error;
        const auto& typedValue = convert<PropertyValue<expression::Image>>(value, error, true, true);
        if (!typedValue) {
            return error;
        }

        setIconImage(*typedValue);
        return std::nullopt;
    }
    if (property == SymbolProperty::IconOffset || property == SymbolProperty::TextOffset) {
        Error error;
        const auto& typedValue = convert<PropertyValue<std::array<float, 2>>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        if (property == SymbolProperty::IconOffset) {
            setIconOffset(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextOffset) {
            setTextOffset(*typedValue);
            return std::nullopt;
        }
    }
    if (property == SymbolProperty::IconPadding) {
        Error error;
        const auto& typedValue = convert<PropertyValue<Padding>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        setIconPadding(*typedValue);
        return std::nullopt;
    }
    if (property == SymbolProperty::IconPitchAlignment || property == SymbolProperty::IconRotationAlignment ||
        property == SymbolProperty::TextPitchAlignment || property == SymbolProperty::TextRotationAlignment) {
        Error error;
        const auto& typedValue = convert<PropertyValue<AlignmentType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        if (property == SymbolProperty::IconPitchAlignment) {
            setIconPitchAlignment(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::IconRotationAlignment) {
            setIconRotationAlignment(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextPitchAlignment) {
            setTextPitchAlignment(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextRotationAlignment) {
            setTextRotationAlignment(*typedValue);
            return std::nullopt;
        }
    }
    if (property == SymbolProperty::IconTextFit) {
        Error error;
        const auto& typedValue = convert<PropertyValue<IconTextFitType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setIconTextFit(*typedValue);
        return std::nullopt;
    }
    if (property == SymbolProperty::IconTextFitPadding) {
        Error error;
        const auto& typedValue = convert<PropertyValue<std::array<float, 4>>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setIconTextFitPadding(*typedValue);
        return std::nullopt;
    }
    if (property == SymbolProperty::SymbolPlacement) {
        Error error;
        const auto& typedValue = convert<PropertyValue<SymbolPlacementType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setSymbolPlacement(*typedValue);
        return std::nullopt;
    }
    if (property == SymbolProperty::SymbolSpacing || property == SymbolProperty::TextLineHeight ||
        property == SymbolProperty::TextMaxAngle || property == SymbolProperty::TextPadding) {
        Error error;
        const auto& typedValue = convert<PropertyValue<float>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        if (property == SymbolProperty::SymbolSpacing) {
            setSymbolSpacing(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextLineHeight) {
            setTextLineHeight(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextMaxAngle) {
            setTextMaxAngle(*typedValue);
            return std::nullopt;
        }

        if (property == SymbolProperty::TextPadding) {
            setTextPadding(*typedValue);
            return std::nullopt;
        }
    }
    if (property == SymbolProperty::SymbolZOrder) {
        Error error;
        const auto& typedValue = convert<PropertyValue<SymbolZOrderType>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setSymbolZOrder(*typedValue);
        return std::nullopt;
    }
    if (property == SymbolProperty::TextField) {
        Error error;
        const auto& typedValue = convert<PropertyValue<expression::Formatted>>(value, error, true, true);
        if (!typedValue) {
            return error;
        }

        setTextField(*typedValue);
        return std::nullopt;
    }
    if (property == SymbolProperty::TextFont) {
        Error error;
        const auto& typedValue = convert<PropertyValue<std::vector<std::string>>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        setTextFont(*typedValue);
        return std::nullopt;
    }
    if (property == SymbolProperty::TextJustify) {
        Error error;
        const auto& typedValue = convert<PropertyValue<TextJustifyType>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        setTextJustify(*typedValue);
        return std::nullopt;
    }
    if (property == SymbolProperty::TextTransform) {
        Error error;
        const auto& typedValue = convert<PropertyValue<TextTransformType>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        setTextTransform(*typedValue);
        return std::nullopt;
    }
    if (property == SymbolProperty::TextVariableAnchor) {
        Error error;
        const auto& typedValue =
            convert<PropertyValue<std::vector<TextVariableAnchorType>>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setTextVariableAnchor(*typedValue);
        return std::nullopt;
    }
    if (property == SymbolProperty::TextVariableAnchorOffset) {
        Error error;
        const auto& typedValue = convert<PropertyValue<VariableAnchorOffsetCollection>>(value, error, true, false);
        if (!typedValue) {
            return error;
        }

        setTextVariableAnchorOffset(*typedValue);
        return std::nullopt;
    }
    if (property == SymbolProperty::TextWritingMode) {
        Error error;
        const auto& typedValue = convert<PropertyValue<std::vector<TextWritingModeType>>>(value, error, false, false);
        if (!typedValue) {
            return error;
        }

        setTextWritingMode(*typedValue);
        return std::nullopt;
    }

    Error error;
    std::optional<TransitionOptions> transition = convert<TransitionOptions>(value, error);
    if (!transition) {
        return error;
    }

    if (property == SymbolProperty::IconColorTransition) {
        setIconColorTransition(*transition);
        return std::nullopt;
    }

    if (property == SymbolProperty::IconHaloBlurTransition) {
        setIconHaloBlurTransition(*transition);
        return std::nullopt;
    }

    if (property == SymbolProperty::IconHaloColorTransition) {
        setIconHaloColorTransition(*transition);
        return std::nullopt;
    }

    if (property == SymbolProperty::IconHaloWidthTransition) {
        setIconHaloWidthTransition(*transition);
        return std::nullopt;
    }

    if (property == SymbolProperty::IconOpacityTransition) {
        setIconOpacityTransition(*transition);
        return std::nullopt;
    }

    if (property == SymbolProperty::IconTranslateTransition) {
        setIconTranslateTransition(*transition);
        return std::nullopt;
    }

    if (property == SymbolProperty::IconTranslateAnchorTransition) {
        setIconTranslateAnchorTransition(*transition);
        return std::nullopt;
    }

    if (property == SymbolProperty::TextColorTransition) {
        setTextColorTransition(*transition);
        return std::nullopt;
    }

    if (property == SymbolProperty::TextHaloBlurTransition) {
        setTextHaloBlurTransition(*transition);
        return std::nullopt;
    }

    if (property == SymbolProperty::TextHaloColorTransition) {
        setTextHaloColorTransition(*transition);
        return std::nullopt;
    }

    if (property == SymbolProperty::TextHaloWidthTransition) {
        setTextHaloWidthTransition(*transition);
        return std::nullopt;
    }

    if (property == SymbolProperty::TextOpacityTransition) {
        setTextOpacityTransition(*transition);
        return std::nullopt;
    }

    if (property == SymbolProperty::TextTranslateTransition) {
        setTextTranslateTransition(*transition);
        return std::nullopt;
    }

    if (property == SymbolProperty::TextTranslateAnchorTransition) {
        setTextTranslateAnchorTransition(*transition);
        return std::nullopt;
    }

    return Error{"layer doesn't support this property"};
}

StyleProperty SymbolLayer::getProperty(const std::string& name) const {
    return getLayerProperty(*this, name);
}

Mutable<Layer::Impl> SymbolLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

} // namespace style
} // namespace mbgl

// clang-format on
