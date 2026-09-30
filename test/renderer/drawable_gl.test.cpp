#include <mln/test/util.hpp>

#if MLN_RENDER_BACKEND_OPENGL

#include <mln/geometry/line_atlas.hpp>
#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/command_encoder.hpp>
#include <mln/gfx/headless_backend.hpp>
#include <mln/gfx/index_vector.hpp>
#include <mln/gfx/render_pass.hpp>
#include <mln/gfx/upload_pass.hpp>
#include <mln/gl/drawable_gl.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/pattern_atlas.hpp>
#include <mln/renderer/render_static_data.hpp>
#include <mln/shaders/gl/shader_program_gl.hpp>
#include <mln/shaders/program_parameters.hpp>
#include <mln/shaders/segment.hpp>
#include <mln/util/run_loop.hpp>

using namespace mln;

TEST(DrawableGL, SharedIndexBufferReplacement) {
    if (gfx::Backend::GetType() != gfx::Backend::Type::OpenGL) {
        GTEST_SKIP() << "Requires the OpenGL backend";
    }
    util::RunLoop runLoop;
    const Size size{16, 16};
    auto target = gfx::HeadlessBackend::Create(size);
    auto& backend = *target->getRendererBackend();
    gfx::BackendScope scope{backend};
    auto& context = static_cast<gl::Context&>(backend.getContext());
    backend.getDefaultRenderable();

    TransformState state;
    state.setSize(size);
    TransformParameters transform{state};
    EvaluatedLight light;
    RenderStaticData staticData{std::make_unique<gfx::ShaderRegistry>()};
    LineAtlas lineAtlas;
    PatternAtlas patternAtlas;
    PaintParameters parameters{context,
                               1,
                               backend,
                               light,
                               MapMode::Static,
                               MapDebugOptions::NoDebug,
                               Clock::now(),
                               transform,
                               staticData,
                               lineAtlas,
                               patternAtlas,
                               0,
                               0,
                               1,
                               0,
                               TileLodMode::Default,
                               size,
                               {},
                               false};

    // The first triangle covers the target; the second is entirely outside it.
    // gl_VertexID makes the rendered result depend only on the selected indices.
    auto shader = gl::ShaderProgramGL::create(context,
                                              ProgramParameters{1, false},
                                              "",
                                              {},
                                              {},
                                              {},
                                              R"(
            const vec2 positions[6] = vec2[6](
                vec2(-1, -1), vec2(3, -1), vec2(-1, 3),
                vec2(2, 2), vec2(3, 2), vec2(2, 3));
            void main() { gl_Position = vec4(positions[gl_VertexID], 0, 1); }
        )",
                                              "void main() { fragColor = vec4(0, 1, 0, 1); }");
    ASSERT_TRUE(shader);

    auto indexes = std::make_shared<gfx::IndexVector<gfx::Triangles>>();
    indexes->emplace_back(0, 1, 2);
    const SegmentBase segment{0, 0, 6, 3};
    gl::DrawableGL first{"first"};
    gl::DrawableGL second{"second"};
    for (auto* drawable : {&first, &second}) {
        drawable->setShader(shader);
        drawable->setEnableDepth(false);
        drawable->setColorMode(gfx::ColorMode::unblended());
        drawable->updateVertexAttributes(
            context.createVertexAttributeArray(), 6, gfx::Triangles{}, indexes, &segment, 1);
    }

    const auto upload = [&](gl::DrawableGL& drawable) {
        auto pass = parameters.encoder->createUploadPass("upload", *target);
        drawable.upload(*pass);
    };
    const auto expectPixel = [&](gl::DrawableGL& drawable, uint8_t green) {
        SCOPED_TRACE(drawable.getName());
        auto pass = parameters.encoder->createRenderPass("draw", {*target, Color::black(), 1.0f, 0});
        drawable.draw(parameters);
        pass.reset();
        const auto image = target->readStillImage();
        const auto offset = (8 * size.width + 8) * 4;
        EXPECT_EQ(image.data[offset], 0);
        EXPECT_EQ(image.data[offset + 1], green);
        EXPECT_EQ(image.data[offset + 2], 0);
        EXPECT_EQ(image.data[offset + 3], 255);
    };

    upload(first);
    upload(second);
    expectPixel(first, 255);
    expectPixel(second, 255);

    // Upload through only one drawable. Both existing VAOs must use the new
    // shared buffer, including the drawable that did not receive an upload.
    for (std::size_t i = 0; i < 3; ++i) {
        indexes->at(i) = static_cast<uint16_t>(i + 3);
    }
    upload(second);
    context.performCleanup();
    expectPixel(first, 0);
    expectPixel(second, 0);

    // Replace it again through the other drawable, then repeat unchanged draws.
    for (std::size_t i = 0; i < 3; ++i) {
        indexes->at(i) = static_cast<uint16_t>(i);
    }
    upload(first);
    context.performCleanup();
    for (int i = 0; i < 2; ++i) {
        expectPixel(second, 255);
        expectPixel(first, 255);
    }
}

#endif
