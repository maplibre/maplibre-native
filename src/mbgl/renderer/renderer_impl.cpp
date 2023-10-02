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
#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/renderer/render_target.hpp>

#include <limits>
#endif // MLN_DRAWABLE_RENDERER

#if !MLN_RENDER_BACKEND_METAL
#include <mbgl/gl/defines.hpp>
#if MLN_DRAWABLE_RENDERER
#include <mbgl/gl/drawable_gl.hpp>
#endif // MLN_DRAWABLE_RENDERER
#endif // !MLN_RENDER_BACKEND_METAL

namespace mbgl {

using namespace style;

namespace {

RendererObserver& nullObserver() {
    static RendererObserver observer;
    return observer;
}

} // namespace

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

void Renderer::Impl::render(const RenderTree& renderTree,
                            [[maybe_unused]] const std::shared_ptr<UpdateParameters>& updateParameters) {
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

    const auto& renderTreeParameters = renderTree.getParameters();
    staticData->has3D = renderTreeParameters.has3D;
    staticData->backendSize = backend.getDefaultRenderable().getSize();

    if (renderState == RenderState::Never) {
        observer->onWillStartRenderingMap();
    }

    observer->onWillStartRenderingFrame();

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
                               renderTree.getPatternAtlas(),
                               frameCount};

    parameters.symbolFadeChange = renderTreeParameters.symbolFadeChange;
    parameters.opaquePassCutoff = renderTreeParameters.opaquePassCutOff;
    const auto& sourceRenderItems = renderTree.getSourceRenderItems();
    const auto& layerRenderItems = renderTree.getLayerRenderItems();

    // - UPLOAD PASS -------------------------------------------------------------------------------
    // Uploads all required buffers and images before we do any actual rendering.
    {
        const auto uploadPass = parameters.encoder->createUploadPass("upload",
                                                                     parameters.backend.getDefaultRenderable());
#if !defined(NDEBUG)
        const auto debugGroup = uploadPass->createDebugGroup("upload");
#endif

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
    }

#if MLN_DRAWABLE_RENDERER
    // - LAYER GROUP UPDATE ------------------------------------------------------------------------
    // Updates all layer groups and process changes
    if (staticData && staticData->shaders) {
        orchestrator.updateLayers(
            *staticData->shaders, context, renderTreeParameters.transformParams.state, updateParameters, renderTree);
    }

    orchestrator.processChanges();

    // Upload layer groups
    {
        const auto uploadPass = parameters.encoder->createUploadPass("layerGroup-upload",
                                                                     parameters.backend.getDefaultRenderable());
#if !defined(NDEBUG)
        const auto debugGroup = uploadPass->createDebugGroup("layerGroup-upload");
#endif

        // Tweakers are run in the upload pass so they can set up uniforms.
        orchestrator.visitLayerGroups(
            [&](LayerGroupBase& layerGroup) { layerGroup.runTweakers(renderTree, parameters); });
    }

    // Update the debug layer groups
    orchestrator.updateDebugLayerGroups(renderTree, parameters);

    // Give the layers a chance to do setup
    // orchestrator.visitLayerGroups([&](LayerGroup& layerGroup) { layerGroup.preRender(orchestrator, parameters);
    // });

    // Upload layer groups
    {
        const auto uploadPass = parameters.encoder->createUploadPass("layerGroup-upload",
                                                                     parameters.backend.getDefaultRenderable());

        // Give the layers a chance to upload
        orchestrator.visitLayerGroups([&](LayerGroupBase& layerGroup) { layerGroup.upload(*uploadPass); });

        // Give the render targets a chance to upload
        orchestrator.visitRenderTargets([&](RenderTarget& renderTarget) { renderTarget.upload(*uploadPass); });

        // Upload the Debug layer group
        orchestrator.visitDebugLayerGroups([&](LayerGroupBase& layerGroup) { layerGroup.upload(*uploadPass); });
    }
#endif

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

        // draw layer groups, 3D pass
        const auto maxLayerIndex = orchestrator.maxLayerIndex();
        orchestrator.visitLayerGroups([&](LayerGroupBase& layerGroup) {
            layerGroup.render(orchestrator, parameters);
            parameters.currentLayer = maxLayerIndex - layerGroup.getLayerIndex();
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

#if MLN_DRAWABLE_RENDERER
    const auto drawableTargetsPass = [&] {
        // draw render targets
        orchestrator.visitRenderTargets(
            [&](RenderTarget& renderTarget) { renderTarget.render(orchestrator, renderTree, parameters); });
    };
#endif

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
        const auto maxLayerIndex = orchestrator.maxLayerIndex();
        parameters.pass = RenderPass::Opaque;
        parameters.currentLayer = 0;
        parameters.depthRangeSize = 1 - (maxLayerIndex + 3) * parameters.numSublayers * PaintParameters::depthEpsilon;

        // draw layer groups, opaque pass
        orchestrator.visitLayerGroups([&](LayerGroupBase& layerGroup) {
            parameters.currentLayer = layerGroup.getLayerIndex();
            layerGroup.render(orchestrator, parameters);
        });
    };

    const auto drawableTranslucentPass = [&] {
        const auto debugGroup(parameters.renderPass->createDebugGroup("drawables-translucent"));
        const auto maxLayerIndex = orchestrator.maxLayerIndex();
        parameters.pass = RenderPass::Translucent;
        parameters.depthRangeSize = 1 - (maxLayerIndex + 3) * parameters.numSublayers * PaintParameters::depthEpsilon;

        // draw layer groups, translucent pass
        orchestrator.visitLayerGroups([&](LayerGroupBase& layerGroup) {
            parameters.currentLayer = maxLayerIndex - layerGroup.getLayerIndex();
            layerGroup.render(orchestrator, parameters);
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

#if MLN_DRAWABLE_RENDERER
    const auto drawableDebugOverlays = [&] {
        // Renders debug overlays.
        {
            const auto debugGroup(parameters.renderPass->createDebugGroup("debug"));
            orchestrator.visitDebugLayerGroups([&](LayerGroupBase& layerGroup) {
                layerGroup.visitDrawables([&](gfx::Drawable& drawable) { drawable.draw(parameters); });
            });
        }
    };
#endif // MLN_DRAWABLE_RENDERER

#if MLN_LEGACY_RENDERER
    const auto renderDebugOverlays = [&] {
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
    };
#endif // MLN_LEGACY_RENDERER

#if (MLN_DRAWABLE_RENDERER && !MLN_LEGACY_RENDERER)
    if (parameters.staticData.has3D) {
        common3DPass();
        drawable3DPass();
    }
    drawableTargetsPass();
    commonClearPass();
    drawableOpaquePass();
    drawableTranslucentPass();
    drawableDebugOverlays();
#elif (MLN_LEGACY_RENDERER && !MLN_DRAWABLE_RENDERER)
    if (parameters.staticData.has3D) {
        common3DPass();
        renderLayer3DPass();
    }
    commonClearPass();
    renderLayerOpaquePass();
    renderLayerTranslucentPass();
    renderDebugOverlays();
#else
    static_assert(0, "Must define one of (MLN_DRAWABLE_RENDERER, MLN_LEGACY_RENDERER)");
#endif // MLN_LEGACY_RENDERER

#if MLN_DRAWABLE_RENDERER
    //     Give the layers a chance to do cleanup
    orchestrator.visitLayerGroups([&](LayerGroupBase& layerGroup) { layerGroup.postRender(orchestrator, parameters); });
#endif

    // Ends the RenderPass
    parameters.renderPass.reset();
    parameters.encoder->present(parameters.backend.getDefaultRenderable());

    // CommandEncoder destructor submits render commands.
    parameters.encoder.reset();

    observer->onDidFinishRenderingFrame(
        renderTreeParameters.loaded ? RendererObserver::RenderMode::Full : RendererObserver::RenderMode::Partial,
        renderTreeParameters.needsRepaint,
        renderTreeParameters.placementChanged,
        renderTree.getElapsedTime());

    if (!renderTreeParameters.loaded) {
        renderState = RenderState::Partial;
    } else if (renderState != RenderState::Fully) {
        renderState = RenderState::Fully;
        observer->onDidFinishRenderingMap();
    }

    frameCount += 1;
}

void Renderer::Impl::reduceMemoryUse() {
    assert(gfx::BackendScope::exists());
    backend.getContext().reduceMemoryUsage();
}

} // namespace mbgl
