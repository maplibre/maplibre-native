#if MLN_RENDER_BACKEND_OPENGL
#include <mln/test/util.hpp>

#include <mln/geometry/line_atlas.hpp>
#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/command_encoder.hpp>
#include <mln/gfx/index_vector.hpp>
#include <mln/gfx/render_pass.hpp>
#include <mln/gfx/upload_pass.hpp>
#include <mln/gl/context.hpp>
#include <mln/gl/drawable_gl.hpp>
#include <mln/gl/headless_backend.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/pattern_atlas.hpp>
#include <mln/renderer/render_static_data.hpp>
#include <mln/shaders/gl/shader_program_gl.hpp>
#include <mln/shaders/program_parameters.hpp>
#include <mln/shaders/segment.hpp>

using namespace mln;

// A symbol bucket replaces its index buffer when it sorts its symbols again,
// and drawables that share the index vector replace it for each other.
TEST(DrawableGL, DrawsWithReplacedIndexBuffer) {
    const Size size{16, 16};
    gl::HeadlessBackend backend{size};
    gfx::BackendScope scope{backend};
    auto& context = static_cast<gl::Context&>(backend.getContext());
    auto& renderable = backend.getDefaultRenderable();

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

    // Indices 0-2 cover the target and indices 3-5 miss it.
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
        auto pass = parameters.encoder->createUploadPass("upload", renderable);
        drawable.upload(*pass);
    };
    upload(first);
    upload(second);

    // Replace the buffer twice through the other drawable. Once the first
    // buffer is deleted, the second replacement can take its GL name.
    for (int replacement = 0; replacement < 2; ++replacement) {
        for (std::size_t i = 0; i < 3; ++i) {
            indexes->at(i) = static_cast<uint16_t>(i + 3);
        }
        upload(second);
        context.performCleanup();
    }

    upload(first);
    {
        auto pass = parameters.encoder->createRenderPass("draw", {renderable, Color::black(), 1.0f, 0});
        first.draw(parameters);
    }
    const auto image = backend.readStillImage();
    EXPECT_EQ(0, image.data[(8 * size.width + 8) * 4 + 1]);
}

#endif
