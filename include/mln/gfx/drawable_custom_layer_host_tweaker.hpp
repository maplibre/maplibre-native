#pragma once

#include <mln/gfx/drawable_tweaker.hpp>
#include <mln/style/layers/custom_layer.hpp>

#include <memory>
#include <string>

namespace mln {

class PaintParameters;

namespace gfx {

class Drawable;

/**
 Tweaker that enables drawing a CustomLayer via their CustomLayerHost
 */
class DrawableCustomLayerHostTweaker : public gfx::DrawableTweaker {
public:
    DrawableCustomLayerHostTweaker(std::shared_ptr<style::CustomLayerHost> host_)
        : host(host_) {}
    ~DrawableCustomLayerHostTweaker() override = default;

    void init(Drawable&) override {};

    void execute(Drawable&, PaintParameters&) override;

protected:
    std::shared_ptr<style::CustomLayerHost> host;
};

} // namespace gfx
} // namespace mln
