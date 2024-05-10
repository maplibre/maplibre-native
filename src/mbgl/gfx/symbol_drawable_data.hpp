#pragma once

#include <mbgl/gfx/drawable_data.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/util/tiny_unordered_map.hpp>

#include <memory>

namespace mbgl {
namespace style {
enum class SymbolType : uint8_t;
} // namespace style
namespace gfx {

struct SymbolDrawableData : public DrawableData {
    SymbolDrawableData(const bool doFill_,
                       const bool doHalo_,
                       const bool bucketVariablePlacement_,
                       const style::SymbolType symbolType_,
                       const style::AlignmentType pitchAlignment_,
                       const style::AlignmentType rotationAlignment_,
                       const style::SymbolPlacementType placement_,
                       const style::IconTextFitType textFit_)
        : doFill(doFill_),
          doHalo(doHalo_),
          bucketVariablePlacement(bucketVariablePlacement_),
          symbolType(symbolType_),
          pitchAlignment(pitchAlignment_),
          rotationAlignment(rotationAlignment_),
          placement(placement_),
          textFit(textFit_) {}
    ~SymbolDrawableData() override = default;

    const bool doFill;
    const bool doHalo;
    bool bucketVariablePlacement;
    const style::SymbolType symbolType;
    const style::AlignmentType pitchAlignment;
    const style::AlignmentType rotationAlignment;
    const style::SymbolPlacementType placement;
    const style::IconTextFitType textFit;
};

using UniqueSymbolDrawableData = std::unique_ptr<SymbolDrawableData>;

} // namespace gfx
} // namespace mbgl
