#pragma once

#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/render_light.hpp>
#include <mln/renderer/render_sky.hpp>
#include <mln/shaders/shader_program_base.hpp>
#include <mln/util/size.hpp>

#include <optional>

namespace mln {

class TransformState;

namespace gfx {
class Context;
class ShaderRegistry;
} // namespace gfx

/// Owns the source-independent fullscreen passes used by the root `sky` style property.
///
/// The planar sky is rendered before style layers. The physical atmosphere is rendered
/// after them, against the globe depth prepass, so the planet correctly occludes it.
class SkyPass {
public:
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const Size& viewportSize,
                float pixelRatio,
                const std::optional<EvaluatedSky>&,
                const EvaluatedLight&);

    LayerGroupBase* getSkyLayerGroup() const { return skyLayerGroup.get(); }
    LayerGroupBase* getAtmosphereLayerGroup() const { return atmosphereLayerGroup.get(); }
    bool hasAtmosphere() const { return atmosphereLayerGroup && !atmosphereLayerGroup->empty(); }

private:
    LayerGroupPtr skyLayerGroup;
    LayerGroupPtr atmosphereLayerGroup;
    gfx::ShaderProgramBasePtr skyShader;
    gfx::ShaderProgramBasePtr atmosphereShader;
};

} // namespace mln
