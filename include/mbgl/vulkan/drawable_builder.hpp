#pragma once

#include <mbgl/gfx/drawable_builder.hpp>

namespace mbgl {
namespace vulkan {

/**
    Base class for Vulkan-specific drawable builders.
 */
class DrawableBuilder final : public gfx::DrawableBuilder {
public:
    DrawableBuilder(std::string name_)
        : gfx::DrawableBuilder(std::move(name_)) {}
    ~DrawableBuilder() override = default;

    using DrawSegment = gfx::Drawable::DrawSegment;
    std::unique_ptr<DrawSegment> createSegment(gfx::DrawMode, SegmentBase&&) override;

protected:
    gfx::UniqueDrawable createDrawable() const override;

    /// Setup the SDK-specific aspects after all the values are present
    void init() override;
};

} // namespace vulkan
} // namespace mbgl
