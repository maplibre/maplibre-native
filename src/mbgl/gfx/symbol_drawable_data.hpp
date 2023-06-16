#pragma once

#include <mbgl/gfx/drawable_data.hpp>
#include <mbgl/style/types.hpp>

#include <memory>

namespace mbgl {
namespace gfx {

struct SymbolDrawableData : public DrawableData {
    SymbolDrawableData(const bool isText_,
                       const bool hasVariablePlacement_,
                       const style::AlignmentType pitchAlignment_,
                       const style::AlignmentType rotationAlignment_,
                       const style::SymbolPlacementType placement_,
                       const style::IconTextFitType textFit_)
        : isText(isText_),
          hasVariablePlacement(hasVariablePlacement_),
          pitchAlignment(pitchAlignment_),
          rotationAlignment(rotationAlignment_),
          placement(placement_),
          textFit(textFit_) {}
    ~SymbolDrawableData() = default;

    const bool isText;
    const bool hasVariablePlacement;
    const style::AlignmentType pitchAlignment;
    const style::AlignmentType rotationAlignment;
    const style::SymbolPlacementType placement;
    const style::IconTextFitType textFit;
};

using UniqueSymbolDrawableData = std::unique_ptr<SymbolDrawableData>;

} // namespace gfx
} // namespace mbgl
