#include <mbgl/gl/layer_group_gl.hpp>

#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gl/drawable_gl.hpp>
#include <mbgl/gl/upload_pass.hpp>
#include <mbgl/gl/vertex_array.hpp>
#include <mbgl/gl/vertex_attribute_gl.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {
namespace gl {

TileLayerGroupGL::TileLayerGroupGL(int32_t layerIndex_, std::size_t initialCapacity)
    : TileLayerGroup(layerIndex_, initialCapacity) {}

void TileLayerGroupGL::upload(gfx::Context& context, gfx::UploadPass& uploadPass) {
    observeDrawables([&](gfx::Drawable& drawable) {
        auto& drawableGL = static_cast<gl::DrawableGL&>(drawable);
        const auto& shader = drawable.getShader();

        // Generate a vertex array object for the drawable state, if necessary
        if (shader && (!drawableGL.getVertexArray().isValid() || drawableGL.getVertexAttributes().isDirty())) {
            const auto usage = gfx::BufferUsageType::StaticDraw;

            // Build index buffer
            auto indexData = drawable.getLineIndexData();
            indexData.insert(
                indexData.end(), drawable.getTriangleIndexData().begin(), drawable.getTriangleIndexData().end());
            auto indexBuffer = gfx::IndexBuffer{
                indexData.size(),
                uploadPass.createIndexBufferResource(indexData.data(), indexData.size() * sizeof(indexData[0]), usage)};

            // Apply drawable values to shader defaults
            const auto& defaults = shader->getVertexAttributes();
            const auto& overrides = drawable.getVertexAttributes();
            const auto vertexCount = drawable.getVertexCount();
            std::unique_ptr<gfx::VertexBufferResource> vertexBuffer;
            auto bindings = uploadPass.buildAttributeBindings(vertexCount, defaults, overrides, usage, vertexBuffer);

            auto& glContext = static_cast<gl::Context&>(context);
            auto vertexArray = glContext.createVertexArray();
            vertexArray.bind(glContext, indexBuffer, bindings);
            glContext.bindVertexArray = gl::value::BindVertexArray::Default;

            drawableGL.setVertexArray(std::move(vertexArray), std::move(vertexBuffer), std::move(indexBuffer));
        }
    });
}

void TileLayerGroupGL::render(RenderOrchestrator&, PaintParameters& parameters) {
    auto& context = parameters.context;

    // TODO: Render tile masks
    // for (auto& layer : renderLayers) {
    //    parameters.renderTileClippingMasks(renderTiles);
    //}

    observeDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.hasRenderPass(parameters.pass)) {
            return;
        }
        if (drawable.hasRenderPass(RenderPass::Opaque) && parameters.currentLayer >= parameters.opaquePassCutoff) {
            return;
        }
        if (!context.setupDraw(parameters, drawable)) {
            return;
        }

        std::string label;
        if (const auto& tileID = drawable.getTileID()) {
            label = drawable.getName() + "/" + util::toString(*tileID);
        }
        const auto labelPtr = (label.empty() ? drawable.getName() : label).c_str();
        const auto debugGroup = parameters.encoder->createDebugGroup(labelPtr);

        drawable.draw(parameters);
    });
}

} // namespace gl
} // namespace mbgl
