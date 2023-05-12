#include <mbgl/renderer/renderer_impl.hpp>

#include <mbgl/geometry/line_atlas.hpp>
#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/gl/drawable_gl.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/renderer/pattern_atlas.hpp>
#include <mbgl/renderer/renderer_observer.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/logging.hpp>

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

        // Initialize shaders for drawables
        const auto programParameters = ProgramParameters{pixelRatio, false};
        backend.initShaders(*staticData->shaders, programParameters);

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

    // Run changes
    orchestrator.processChanges();

    // Collect drawables
    std::vector<gfx::DrawablePtr> drawables(orchestrator.getDrawables().size());
    std::transform(orchestrator.getDrawables().begin(),
                   orchestrator.getDrawables().end(),
                   drawables.begin(),
                   std::bind(&RenderOrchestrator::DrawableMap::value_type::second, std::placeholders::_1));

    // Run tweakers to update any dynamic elements
    for (auto& drawable : drawables) {
        for (auto& tweaker : drawable->getTweakers()) {
            tweaker->execute(*drawable, parameters);
        }
    }

    // Give the layers a chance to do setup
    orchestrator.observeLayerGroups([&](LayerGroup& layerGroup) { layerGroup.preRender(orchestrator, parameters); });

    // Sort the drawables
    std::sort(drawables.begin(), drawables.end(), gfx::DrawablePtrLessByLayer(/*descending=*/true));

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

        for (const auto& drawPtr : drawables) {
            auto& drawable = *drawPtr;

            auto& drawableGL = static_cast<gl::DrawableGL&>(drawable);
            auto& shader = drawable.getShader();

            // Generate a vertex array object for the drawable state, if necessary
            if (shader && (!drawableGL.getVertexArray().isValid() || drawableGL.getVertexAttributes().isDirty())) {
                const auto usage = gfx::BufferUsageType::StaticDraw;

                // Build index buffer
                const auto& indexData = drawable.getIndexData();
                auto indexBuffer = gfx::IndexBuffer{
                    indexData.size(),
                    uploadPass->createIndexBufferResource(
                        indexData.data(), indexData.size() * sizeof(indexData[0]), usage)};

                // Apply drawable values to shader defaults
                const auto& defaults = shader->getVertexAttributes();
                const auto& overrides = drawable.getVertexAttributes();
                const auto vertexCount = drawable.getVertexCount();
                std::unique_ptr<gfx::VertexBufferResource> vertexBuffer;
                auto bindings = uploadPass->buildAttributeBindings(
                    vertexCount, defaults, overrides, usage, vertexBuffer);

                auto& glContext = static_cast<gl::Context&>(context);
                auto vertexArray = glContext.createVertexArray();
                vertexArray.bind(glContext, indexBuffer, bindings);
                glContext.bindVertexArray = gl::value::BindVertexArray::Default;

                drawableGL.setVertexArray(std::move(vertexArray), std::move(vertexBuffer), std::move(indexBuffer));
            }
        }

        // Give the layers a chance to upload
        orchestrator.observeLayerGroups([&](LayerGroup& layerGroup) { layerGroup.upload(context, *uploadPass); });
    }

    // - 3D PASS
    // -------------------------------------------------------------------------------------
    // Renders any 3D layers bottom-to-top to unique FBOs with texture
    // attachments, but share the same depth rbo between them.
    if (parameters.staticData.has3D) {
        parameters.staticData.backendSize = parameters.backend.getDefaultRenderable().getSize();

        const auto debugGroup(parameters.encoder->createDebugGroup("3d"));
        parameters.pass = RenderPass::Pass3D;

        if (!parameters.staticData.depthRenderbuffer ||
            parameters.staticData.depthRenderbuffer->getSize() != parameters.staticData.backendSize) {
            parameters.staticData.depthRenderbuffer =
                parameters.context.createRenderbuffer<gfx::RenderbufferPixelType::Depth>(
                    parameters.staticData.backendSize);
        }
        parameters.staticData.depthRenderbuffer->setShouldClear(true);

        int32_t i = static_cast<int32_t>(layerRenderItems.size()) - 1;
        for (auto it = layerRenderItems.begin(); it != layerRenderItems.end() && i >= 0; ++it, --i) {
            parameters.currentLayer = i;
            const RenderItem& renderItem = it->get();
            if (renderItem.hasRenderPass(parameters.pass)) {
                const auto layerDebugGroup(parameters.encoder->createDebugGroup(renderItem.getName().c_str()));
                renderItem.render(parameters);
            }
        }
    }

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

    // Draw Drawables
    const auto drawDrawables = [&](const mbgl::RenderPass renderPass) {
        // TODO: figure out how to make drawables participate in the depth system
        // Maybe make it a function mapping draw priority to the depth range (always (0,1)?)
        parameters.depthRangeSize = 1 - (1 + 2) * parameters.numSublayers * parameters.depthEpsilon;
        const auto debugGroup(parameters.renderPass->createDebugGroup("drawables"));

        // TODO: Render tile masks
        // for (auto& layer : renderLayers) {
        //    parameters.renderTileClippingMasks(renderTiles);
        //}

        for (const auto& drawPtr : drawables) {
            auto& drawable = *drawPtr;

            if (!drawable.hasRenderPass(renderPass)) {
                continue;
            }
            if (drawable.hasRenderPass(RenderPass::Opaque) && parameters.currentLayer >= parameters.opaquePassCutoff) {
                continue;
            }
            if (!context.setupDraw(parameters, drawable)) {
                continue;
            }

            mat4 matrix = drawable.getMatrix();
            if (drawable.getTileID()) {
                const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
                const auto tileMat = parameters.matrixForTile(tileID);
                matrix::multiply(matrix, drawable.getMatrix(), tileMat);
                matrix = tileMat;
            }

            gfx::DrawableUBO drawableUBO;
            drawableUBO.matrix = util::cast<float>(matrix);
            auto uniformBuffer = context.createUniformBuffer(&drawableUBO, sizeof(drawableUBO));
            drawable.mutableUniformBuffers().addOrReplace("DrawableUBO", uniformBuffer);

            drawable.draw(parameters);
        }
    };

    // draw layer groups, opaque pass
    orchestrator.observeLayerGroups([&](LayerGroup& layerGroup) { layerGroup.render(orchestrator, parameters); });

    parameters.pass = RenderPass::Opaque;
    parameters.currentLayer = 0;
    parameters.opaquePassCutoff = 1;
    drawDrawables(parameters.pass);

    // Actually render the layers
    parameters.opaquePassCutoff = renderTreeParameters.opaquePassCutOff;
    parameters.depthRangeSize = 1 - (layerRenderItems.size() + 2) * parameters.numSublayers * parameters.depthEpsilon;

    // - OPAQUE PASS -------------------------------------------------------------------------------
    // Render everything top-to-bottom by using reverse iterators. Render opaque objects first.
    {
        const auto debugGroup(parameters.renderPass->createDebugGroup("opaque"));

        uint32_t i = 0;
        for (auto it = layerRenderItems.rbegin(); it != layerRenderItems.rend(); ++it, ++i) {
            parameters.currentLayer = i;
            const RenderItem& renderItem = it->get();
            if (renderItem.getName() == "background") {
                continue;
            }
            if (renderItem.hasRenderPass(parameters.pass)) {
                const auto layerDebugGroup(parameters.renderPass->createDebugGroup(renderItem.getName().c_str()));
                renderItem.render(parameters);
            }
        }
    }

    parameters.pass = RenderPass::Translucent;
    parameters.currentLayer = 1;
    parameters.opaquePassCutoff = 1;
    drawDrawables(parameters.pass);

    // draw layer groups, translucent pass
    orchestrator.observeLayerGroups([&](LayerGroup& layerGroup) { layerGroup.render(orchestrator, parameters); });

    parameters.opaquePassCutoff = renderTreeParameters.opaquePassCutOff;

    // - TRANSLUCENT PASS --------------------------------------------------------------------------
    // Make a second pass, rendering translucent objects. This time, we render bottom-to-top.
    {
        const auto debugGroup(parameters.renderPass->createDebugGroup("translucent"));

        int32_t i = static_cast<int32_t>(layerRenderItems.size()) - 1;
        for (auto it = layerRenderItems.begin(); it != layerRenderItems.end() && i >= 0; ++it, --i) {
            parameters.currentLayer = i;
            const RenderItem& renderItem = it->get();
            if (renderItem.hasRenderPass(parameters.pass)) {
                const auto layerDebugGroup(parameters.renderPass->createDebugGroup(renderItem.getName().c_str()));
                renderItem.render(parameters);
            }
        }
    }

    // - DEBUG PASS --------------------------------------------------------------------------------
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

    // Give the layers a chance to do cleanup
    orchestrator.observeLayerGroups([&](LayerGroup& layerGroup) { layerGroup.postRender(orchestrator, parameters); });

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
