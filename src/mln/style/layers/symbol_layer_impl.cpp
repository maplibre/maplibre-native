#include <mln/style/layers/symbol_layer_impl.hpp>
#include <mln/util/logging.hpp>

namespace mln {
namespace style {

bool SymbolLayer::Impl::hasFormatSectionOverrides() const {
    if (!hasFormatSectionOverrides_) {
        hasFormatSectionOverrides_ = SymbolLayerPaintPropertyOverrides::hasOverrides(layout.get<TextField>());
    }
    return *hasFormatSectionOverrides_;
}

namespace {
/// Line labels and variable anchors carry `*-translate` in the positions placement lays out for the whole bucket.
bool laysOutTranslation(const SymbolLayoutProperties::Unevaluated& layout) {
    const auto& placement = layout.get<SymbolPlacement>();
    return !(placement.isConstant() && placement.asConstant() == SymbolPlacementType::Point) ||
           !layout.get<TextVariableAnchor>().isUndefined() || !layout.get<TextVariableAnchorOffset>().isUndefined();
}

bool hasTranslationDifference(const SymbolPaintProperties::Transitionable& a,
                              const SymbolPaintProperties::Transitionable& b) {
    return a.get<TextTranslate>().value != b.get<TextTranslate>().value ||
           a.get<TextTranslateAnchor>().value != b.get<TextTranslateAnchor>().value ||
           a.get<IconTranslate>().value != b.get<IconTranslate>().value ||
           a.get<IconTranslateAnchor>().value != b.get<IconTranslateAnchor>().value;
}
} // namespace

bool SymbolLayer::Impl::hasLayoutDifference(const Layer::Impl& other) const {
    assert(other.getTypeInfo() == getTypeInfo());
    const auto& impl = static_cast<const style::SymbolLayer::Impl&>(other);
    return filter != impl.filter || visibility != impl.visibility || layout != impl.layout ||
           paint.hasDataDrivenPropertyDifference(impl.paint) ||
           (hasFormatSectionOverrides() &&
            SymbolLayerPaintPropertyOverrides::hasPaintPropertyDifference(paint, impl.paint)) ||
           (laysOutTranslation(layout) && hasTranslationDifference(paint, impl.paint));
}

void SymbolLayer::Impl::populateFontStack(std::set<FontStack>& fontStack) const {
    if (layout.get<TextField>().isUndefined()) {
        return;
    }

    layout.get<TextFont>().match(
        [&fontStack](Undefined) {
            fontStack.insert({util::LAST_RESORT_ALPHABETIC_FONT, util::LAST_RESORT_PAN_UNICODE_FONT});
        },
        [&fontStack](const FontStack& constant) { fontStack.insert(constant); },
        [&](const auto& function) {
            for (const auto& value : function.possibleOutputs()) {
                if (value) {
                    fontStack.insert(*value);
                } else {
                    Log::Warning(Event::ParseStyle,
                                 "Layer '" + id +
                                     "' has an invalid value for text-font and will not "
                                     "render text. Output values "
                                     "must be contained as literals within the "
                                     "expression.");
                    break;
                }
            }
        });
}

} // namespace style
} // namespace mln
