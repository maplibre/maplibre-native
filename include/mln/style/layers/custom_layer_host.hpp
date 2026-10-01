#pragma once

#include <mln/style/layers/custom_layer_init_parameters.hpp>
#include <mln/style/layers/custom_layer_render_parameters.hpp>

namespace mln {

namespace gfx {
class Context;
} // namespace gfx

namespace style {

class CustomLayerHost {
public:
    virtual ~CustomLayerHost() = default;
    /**
     * Initialize any GL/Metal/Vulkan state needed by the custom layer. This method
     * is called once, from the main thread, at a point when the graphics context
     * is active but before rendering for the first time.
     *
     * The parameters argument provides backend-specific device handles:
     *   - Vulkan: cast to mln::style::vulkan::CustomLayerInitParameters for
     *     access to vk::Device, vk::PhysicalDevice, and the dispatcher.
     *   - Metal/GL: base CustomLayerInitParameters (no extra handles currently).
     *
     * Resources that are acquired in this method must be released in the
     * `deinitialize` function.
     */
    virtual void initialize(const CustomLayerInitParameters&) = 0;

    /**
     * Called right before the layers start rendering.
     */
    virtual void preRender(const mln::gfx::Context&, const mln::style::CustomLayerRenderParameters&) {};

    /**
     * Render the layer. This method is called once per frame. The
     * implementation should not make any assumptions about the GL/Metal state (other
     * than that the correct context is active). It may make changes to the
     * state, and is not required to reset values such as the depth mask,
     * stencil mask, and corresponding test flags to their original values. Make
     * sure that you are drawing your fragments with a z value of 1 to take
     * advantage of the opaque fragment culling in case there are opaque layers
     * above your custom layer.
     */
    virtual void render(const mln::style::CustomLayerRenderParameters&) = 0;

    /**
     * Called when the system has destroyed the underlying GL/Metal context. The
     * `deinitialize` function will not be called in this case, however
     * `initialize` will be called instead to prepare for a new render.
     *
     */
    virtual void contextLost() = 0;

    /**
     * Destroy any GL/Metal state needed by the custom layer, and deallocate context,
     * if necessary. This method is called once, from the main thread, at a
     * point when the GL/Metal context is active.
     *
     * Note that it may be called even when the `initialize` function has not
     * been called.
     */
    virtual void deinitialize() = 0;
};

} // namespace style
} // namespace mln
