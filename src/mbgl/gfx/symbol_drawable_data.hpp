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
    SymbolDrawableData(const bool isHalo_,
                       const bool bucketVariablePlacement_,
                       const style::SymbolType symbolType_,
                       const style::AlignmentType pitchAlignment_,
                       const style::AlignmentType rotationAlignment_,
                       const style::SymbolPlacementType placement_,
                       const style::IconTextFitType textFit_,
                       const bool isOffset_)
        : isHalo(isHalo_),
          bucketVariablePlacement(bucketVariablePlacement_),
          symbolType(symbolType_),
          pitchAlignment(pitchAlignment_),
          rotationAlignment(rotationAlignment_),
          placement(placement_),
          textFit(textFit_),
          isOffset(isOffset_) {}
    ~SymbolDrawableData() override = default;

    const bool isHalo;
    bool bucketVariablePlacement;
    const style::SymbolType symbolType;
    const style::AlignmentType pitchAlignment;
    const style::AlignmentType rotationAlignment;
    const style::SymbolPlacementType placement;
    const style::IconTextFitType textFit;
    const bool isOffset;
};

using UniqueSymbolDrawableData = std::unique_ptr<SymbolDrawableData>;

} // namespace gfx
} // namespace mbgl
