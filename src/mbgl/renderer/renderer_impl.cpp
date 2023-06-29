#include <mbgl/renderer/renderer_impl.hpp>

#include <mbgl/geometry/line_atlas.hpp>
#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/renderer/pattern_atlas.hpp>
#include <mbgl/renderer/renderer_observer.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/logging.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gl/drawable_gl.hpp>
#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>
#endif

#if (MLN_LEGACY_RENDERER && MLN_DRAWABLE_RENDERER)
// DEBUG: Enable a debugging split view to compare drawables and vanilla rendering pathways
// Drawables will be on the left, vanilla rendering on the right
// #define MLN_RENDERER_SPLIT_VIEW 1
// If using SPLIT_VIEW, MLN_RENDERER_QUAD_SPLIT_VIEW will split each half, showing just the opaque
// pass on top and then a composited opaque+translucent pass on the bottom
// #define QUAD_SPLIT_VIEW
#if MLN_RENDERER_SPLIT_VIEW
#include <mbgl/gl/context.hpp>
#endif
#endif

namespace mbgl {

using namespace style;

static RendererObserver& nullObserver() {
    static RendererObserver observer;
    return observer;
}

Renderer::Impl::Impl(gfx::RendererBackend& backend_,
                     float pixelRatio_,
                     const std::optional<std::string>& localFontFamily_)
    : orchestrator(!backend_.contextIsShared(), localFontFamily_),
      backend(backend_),
      observer(&nullObserver()),
      pixelRatio(pixelRatio_) {}

Renderer::Impl::~Impl() {
    assert(gfx::BackendScope::exists());
};

void Renderer::Impl::setObserver(RendererObserver* observer_) {
    observer = observer_ ? observer_ : &nullObserver();
}

void Renderer::Impl::render(const RenderTree& renderTree) {
    if (renderState == RenderState::Never) {
        observer->onWillStartRenderingMap();
    }

    observer->onWillStartRenderingFrame();
    const auto& renderTreeParameters = renderTree.getParameters();

    auto& context = backend.getContext();
    // Blocks execution until the renderable is available.
    backend.getDefaultRenderable().wait();

    if (!staticData) {
        staticData = std::make_unique<RenderStaticData>(pixelRatio, std::make_unique<gfx::ShaderRegistry>());

        // Initialize legacy shader programs
        staticData->programs.registerWith(*staticData->shaders);

#if MLN_DRAWABLE_RENDERER
        // Initialize shaders for drawables
        const auto programParameters = ProgramParameters{pixelRatio, false};
        backend.initShaders(*staticData->shaders, programParameters);
#endif

        // Notify post-shader registration
        observer->onRegisterShaders(*staticData->shaders);
    }
    staticData->has3D = renderTreeParameters.has3D;

    PaintParameters parameters{context,
                               pixelRatio,
                               backend,
                               renderTreeParameters.light,
                               renderTreeParameters.mapMode,
                               renderTreeParameters.debugOptions,
                               renderTreeParameters.timePoint,
                               renderTreeParameters.transformParams,
                               *staticData,
                               renderTree.getLineAtlas(),
                               renderTree.getPatternAtlas()};

    parameters.symbolFadeChange = renderTreeParameters.symbolFadeChange;
    parameters.opaquePassCutoff = renderTreeParameters.opaquePassCutOff;
    const auto& sourceRenderItems = renderTree.getSourceRenderItems();
    const auto& layerRenderItems = renderTree.getLayerRenderItems();

#if MLN_DRAWABLE_RENDERER
    // Run changes
    orchestrator.processChanges();

    // Run layer tweakers to update any dynamic elements
    orchestrator.observeLayerGroups([&](LayerGroupBase& layerGroup) {
        if (layerGroup.getLayerTweaker()) {
            layerGroup.getLayerTweaker()->execute(layerGroup, renderTree, parameters);
        }
    });

    // Give the layers a chance to do setup
    // orchestrator.observeLayerGroups([&](LayerGroup& layerGroup) { layerGroup.preRender(orchestrator, parameters);
    // });
#endif

    // - UPLOAD PASS -------------------------------------------------------------------------------
    // Uploads all required buffers and images before we do any actual rendering.
    {
        const auto uploadPass = parameters.encoder->createUploadPass("upload");

        // Update all clipping IDs + upload buckets.
        for (const RenderItem& item : sourceRenderItems) {
            item.upload(*uploadPass);
        }
        for (const RenderItem& item : layerRenderItems) {
            item.upload(*uploadPass);
        }
        staticData->upload(*uploadPass);
        renderTree.getLineAtlas().upload(*uploadPass);
        renderTree.getPatternAtlas().upload(*uploadPass);

#if MLN_DRAWABLE_RENDERER
        // Give the layers a chance to upload
        orchestrator.observeLayerGroups([&](LayerGroupBase& layerGroup) { layerGroup.upload(*uploadPass); });
#endif
    }

    // - 3D PASS
    // -------------------------------------------------------------------------------------
    // Renders any 3D layers bottom-to-top to unique FBOs with texture
    // attachments, but share the same depth rbo between them.
    const auto common3DPass = [&] {
        if (parameters.staticData.has3D) {
            parameters.staticData.backendSize = parameters.backend.getDefaultRenderable().getSize();
            
            const auto debugGroup(parameters.encoder->createDebugGroup("common-3d"));
            parameters.pass = RenderPass::Pass3D;
            
            if (!parameters.staticData.depthRenderbuffer ||
                parameters.staticData.depthRenderbuffer->getSize() != parameters.staticData.backendSize) {
                parameters.staticData.depthRenderbuffer =
                parameters.context.createRenderbuffer<gfx::RenderbufferPixelType::Depth>(
                                                                                         parameters.staticData.backendSize);
            }
            parameters.staticData.depthRenderbuffer->setShouldClear(true);
        }
    };

#if MLN_DRAWABLE_RENDERER
    const auto drawable3DPass = [&] {
        const auto debugGroup(parameters.encoder->createDebugGroup("drawables-3d"));
        assert(parameters.pass == RenderPass::Pass3D);
        parameters.currentLayer = 0;

        // draw layer groups, opaque pass
        orchestrator.observeLayerGroups([&](LayerGroupBase& layerGroup) {
            layerGroup.render(orchestrator, parameters);
            parameters.currentLayer++;
        });
    };
#endif // MLN_DRAWABLE_RENDERER

#if MLN_LEGACY_RENDERER
    const auto renderLayer3DPass = [&] {
        const auto debugGroup(parameters.encoder->createDebugGroup("3d"));
        int32_t i = static_cast<int32_t>(layerRenderItems.size()) - 1;
        for (auto it = layerRenderItems.begin(); it != layerRenderItems.end() && i >= 0; ++it, --i) {
            parameters.currentLayer = i;
            const RenderItem& renderItem = it->get();
            if (renderItem.hasRenderPass(parameters.pass)) {
                const auto layerDebugGroup(parameters.encoder->createDebugGroup(renderItem.getName().c_str()));
                renderItem.render(parameters);
            }
        }
    };
#endif // MLN_LEGACY_RENDERER

    const auto commonClearPass = [&] {
        // - CLEAR
        // -------------------------------------------------------------------------------------
        // Renders the backdrop of the OpenGL view. This also paints in areas where
        // we don't have any tiles whatsoever.
        {
            std::optional<Color> color;
            if (parameters.debugOptions & MapDebugOptions::Overdraw) {
                color = Color::black();
            } else if (!backend.contextIsShared()) {
                color = renderTreeParameters.backgroundColor;
            }
            parameters.renderPass = parameters.encoder->createRenderPass(
                                                                         "main buffer", {parameters.backend.getDefaultRenderable(), color, 1.0f, 0});
        }
    };

    // Actually render the layers
#if MLN_DRAWABLE_RENDERER
    // Drawables
    const auto drawableOpaquePass = [&] {
        const auto debugGroup(parameters.renderPass->createDebugGroup("drawables-opaque"));
        parameters.pass = RenderPass::Opaque;
        parameters.currentLayer = 0;
        parameters.depthRangeSize = 1 - (orchestrator.numLayerGroups() + 2) * parameters.numSublayers *
                                            PaintParameters::depthEpsilon;

        // draw layer groups, opaque pass
        orchestrator.observeLayerGroups([&](LayerGroupBase& layerGroup) {
            layerGroup.render(orchestrator, parameters);
            parameters.currentLayer++;
        });
    };

    const auto drawableTranslucentPass = [&] {
        const auto debugGroup(parameters.renderPass->createDebugGroup("drawables-translucent"));
        parameters.pass = RenderPass::Translucent;
        parameters.currentLayer = static_cast<int32_t>(orchestrator.numLayerGroups()) - 1;
        parameters.depthRangeSize = 1 - (orchestrator.numLayerGroups() + 2) * parameters.numSublayers *
                                            PaintParameters::depthEpsilon;

        // draw layer groups, translucent pass
        orchestrator.observeLayerGroups([&](LayerGroupBase& layerGroup) {
            layerGroup.render(orchestrator, parameters);
            if (parameters.currentLayer != 0) {
                parameters.currentLayer--;
            }
        });
    };
#endif

#if MLN_LEGACY_RENDERER
    // Render everything top-to-bottom by using reverse iterators. Render opaque objects first.
    const auto renderLayerOpaquePass = [&] {
        const auto debugGroup(parameters.renderPass->createDebugGroup("opaque"));
        parameters.pass = RenderPass::Opaque;
        parameters.depthRangeSize = 1 - (layerRenderItems.size() + 2) * parameters.numSublayers *
                                            PaintParameters::depthEpsilon;

        uint32_t i = 0;
        for (auto it = layerRenderItems.rbegin(); it != layerRenderItems.rend(); ++it, ++i) {
            parameters.currentLayer = i;
            const RenderItem& renderItem = it->get();
            if (renderItem.hasRenderPass(parameters.pass)) {
                const auto layerDebugGroup(parameters.renderPass->createDebugGroup(renderItem.getName().c_str()));
                renderItem.render(parameters);
            }
        }
    };

    // Make a second pass, rendering translucent objects. This time, we render bottom-to-top.
    const auto renderLayerTranslucentPass = [&] {
        const auto debugGroup(parameters.renderPass->createDebugGroup("translucent"));
        parameters.pass = RenderPass::Translucent;
        parameters.depthRangeSize = 1 - (layerRenderItems.size() + 2) * parameters.numSublayers *
                                            PaintParameters::depthEpsilon;

        int32_t i = static_cast<int32_t>(layerRenderItems.size()) - 1;
        for (auto it = layerRenderItems.begin(); it != layerRenderItems.end() && i >= 0; ++it, --i) {
            parameters.currentLayer = i;
            const RenderItem& renderItem = it->get();
            if (renderItem.hasRenderPass(parameters.pass)) {
                const auto layerDebugGroup(parameters.renderPass->createDebugGroup(renderItem.getName().c_str()));
                renderItem.render(parameters);
            }
        }
    };
#endif // MLN_LEGACY_RENDERER

#if (MLN_DRAWABLE_RENDERER && !MLN_LEGACY_RENDERER)
    if (parameters.staticData.has3D) {
        common3DPass();
        drawable3DPass();
    }
    commonClearPass();
    drawableOpaquePass();
    drawableTranslucentPass();
#elif (MLN_LEGACY_RENDERER && !MLN_DRAWABLE_RENDERER)
    if (parameters.staticData.has3D) {
        common3DPass();
        renderLayer3DPass();
    }
    commonClearPass();
    renderLayerOpaquePass();
    renderLayerTranslucentPass();
#elif (MLN_DRAWABLE_RENDERER && MLN_LEGACY_RENDERER)
#if MLN_RENDERER_SPLIT_VIEW
    [[maybe_unused]] const auto W = backend.getDefaultRenderable().getSize().width;
    [[maybe_unused]] const auto H = backend.getDefaultRenderable().getSize().height;
    [[maybe_unused]] const auto halfW = static_cast<platform::GLsizei>(backend.getDefaultRenderable().getSize().width *
                                                                       0.5f);
    [[maybe_unused]] const auto halfH = static_cast<platform::GLsizei>(backend.getDefaultRenderable().getSize().height *
                                                                       0.5f);
    platform::glEnable(GL_SCISSOR_TEST);
#if MLN_RENDERER_QUAD_SPLIT_VIEW
    // Drawable LayerGroups on the left
    // Opaque only on top
    platform::glScissor(0, 0, halfW, H);
    if (parameters.staticData.has3D) {
        common3DPass();
        drawable3DPass();
        parameters.clearStencil();
    }
    commonClearPass();
    drawableOpaquePass();

    // Composite (Opaque+Translucent) on bottom
    platform::glScissor(0, 0, halfW, halfH);
    drawableTranslucentPass();

    // RenderLayers on the right
    // Opaque only on top
    platform::glScissor(halfW, 0, halfW, H);
    if (parameters.staticData.has3D) {
        parameters.clearStencil();
        common3DPass();
        renderLayer3DPass();
        parameters.clearStencil();
    }
    commonClearPass();
    // Clipping masks were drawn only on the other side
    parameters.clearTileClippingMasks();
    renderLayerOpaquePass();

    // Composite (Opaque+Translucent) on bottom
    platform::glScissor(halfW, 0, halfW, halfH);
    renderLayerTranslucentPass();
#else // MLN_RENDERER_QUAD_SPLIT_VIEW
    // Drawable LayerGroups on the left
    platform::glScissor(0, 0, halfW, H);
    if (parameters.staticData.has3D) {
        common3DPass();
        drawable3DPass();
        parameters.clearStencil();
    }
    commonClearPass();
    parameters.clearTileClippingMasks();
    drawableOpaquePass();
    drawableTranslucentPass();

    // RenderLayers on the right
    platform::glScissor(halfW, 0, W, H);
    if (parameters.staticData.has3D) {
        parameters.clearStencil();
        common3DPass();
        renderLayer3DPass();
        parameters.clearStencil();
    }
    commonClearPass();
    parameters.clearTileClippingMasks();
    renderLayerOpaquePass();
    renderLayerTranslucentPass();
#endif // MLN_RENDERER_QUAD_SPLIT_VIEW
    // Reset viewport
    platform::glScissor(0, 0, W, H);
    platform::glDisable(GL_SCISSOR_TEST);
#else  // if MLN_RENDERER_SPLIT_VIEW
    if (parameters.staticData.has3D) {
        common3DPass();
        renderLayer3DPass();
        drawable3DPass();
        parameters.clearStencil();
    }
    commonClearPass();
    // Do RenderLayers first, drawables last
    renderLayerOpaquePass();
    renderLayerTranslucentPass();

    drawableOpaquePass();
    drawableTranslucentPass();
#endif // MLN_RENDERER_SPLIT_VIEW
#else
    static_assert(0, "Must define one of (MLN_DRAWABLE_RENDERER, MLN_LEGACY_RENDERER)");
#endif

#if MLN_LEGACY_RENDERER
    // - DEBUG PASS
    // --------------------------------------------------------------------------------
    // Renders debug overlays.
    {
        const auto debugGroup(parameters.renderPass->createDebugGroup("debug"));

        // Finalize the rendering, e.g. by calling debug render calls per tile.
        // This guarantees that we have at least one function per tile called.
        // When only rendering layers via the stylesheet, it's possible that we
        // don't ever visit a tile during rendering.
        for (const RenderItem& renderItem : sourceRenderItems) {
            renderItem.render(parameters);
        }
    }

#if !defined(NDEBUG)
    if (parameters.debugOptions & MapDebugOptions::StencilClip) {
        // Render tile clip boundaries, using stencil buffer to calculate fill color.
        parameters.context.visualizeStencilBuffer();
    } else if (parameters.debugOptions & MapDebugOptions::DepthBuffer) {
        // Render the depth buffer.
        parameters.context.visualizeDepthBuffer(parameters.depthRangeSize);
    }
#endif
#endif // MLN_LEGACY_RENDERER

#if MLN_DRAWABLE_RENDERER
    //     Give the layers a chance to do cleanup
    orchestrator.observeLayerGroups(
        [&](LayerGroupBase& layerGroup) { layerGroup.postRender(orchestrator, parameters); });
#endif

    // Ends the RenderPass
    parameters.renderPass.reset();
    const bool isMapModeContinuous = renderTreeParameters.mapMode == MapMode::Continuous;
    if (isMapModeContinuous) {
        parameters.encoder->present(parameters.backend.getDefaultRenderable());
    }

    // CommandEncoder destructor submits render commands.
    parameters.encoder.reset();

    observer->onDidFinishRenderingFrame(
        renderTreeParameters.loaded ? RendererObserver::RenderMode::Full : RendererObserver::RenderMode::Partial,
        renderTreeParameters.needsRepaint,
        renderTreeParameters.placementChanged);

    if (!renderTreeParameters.loaded) {
        renderState = RenderState::Partial;
    } else if (renderState != RenderState::Fully) {
        renderState = RenderState::Fully;
        observer->onDidFinishRenderingMap();
    }
}

void Renderer::Impl::reduceMemoryUse() {
    assert(gfx::BackendScope::exists());
    backend.getContext().reduceMemoryUsage();
}

} // namespace mbgl
