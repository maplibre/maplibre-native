#if MLN_RENDER_BACKEND_OPENGL
#include <mln/test/util.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/gl/context.hpp>
#include <mln/style/layers/custom_layer.hpp>
#include <mln/gl/defines.hpp>
#include <mln/gl/headless_backend.hpp>
#include <mln/gl/renderable_resource.hpp>
#include <mln/gl/uniform_buffer_gl.hpp>
#include <mln/gfx/types.hpp>
#include <mln/gfx/upload_pass.hpp>
#include <mln/gl/upload_pass.hpp>
#include <mln/map/map.hpp>
#include <mln/map/map_options.hpp>
#include <mln/platform/gl_functions.hpp>
#include <mln/renderer/renderer.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/style/layers/background_layer.hpp>
#include <mln/style/layers/fill_layer.hpp>
#include <mln/style/style.hpp>
#include <mln/util/io.hpp>
#include <mln/util/mat4.hpp>
#include <mln/util/run_loop.hpp>

#include <array>
#include <cstring>
#include <vector>

using namespace mln;
using namespace mln::style;
using namespace mln::platform;

static const GLchar* vertexShaderSource = R"MBGL_SHADER(
#ifdef GL_ES
precision mediump float;
#endif
attribute vec2 a_pos;
void main() {
    gl_Position = vec4(a_pos, 0, 1);
}
)MBGL_SHADER";

static const GLchar* fragmentShaderSource = R"MBGL_SHADER(
#ifdef GL_ES
precision mediump float;
#endif
void main() {
    gl_FragColor = vec4(0, 1, 0, 1);
}
)MBGL_SHADER";

struct Shader {
    Shader(const GLchar* vertex, const GLchar* fragment) {
        program = MBGL_CHECK_ERROR(glCreateProgram());
        vertexShader = MBGL_CHECK_ERROR(glCreateShader(GL_VERTEX_SHADER));
        fragmentShader = MBGL_CHECK_ERROR(glCreateShader(GL_FRAGMENT_SHADER));
        MBGL_CHECK_ERROR(glShaderSource(vertexShader, 1, &vertex, nullptr));
        MBGL_CHECK_ERROR(glCompileShader(vertexShader));
        MBGL_CHECK_ERROR(glAttachShader(program, vertexShader));
        MBGL_CHECK_ERROR(glShaderSource(fragmentShader, 1, &fragment, nullptr));
        MBGL_CHECK_ERROR(glCompileShader(fragmentShader));
        MBGL_CHECK_ERROR(glAttachShader(program, fragmentShader));
        MBGL_CHECK_ERROR(glLinkProgram(program));
        a_pos = MBGL_CHECK_ERROR(glGetAttribLocation(program, "a_pos"));
    }

    ~Shader() {
        MBGL_CHECK_ERROR(glDetachShader(program, vertexShader));
        MBGL_CHECK_ERROR(glDetachShader(program, fragmentShader));
        MBGL_CHECK_ERROR(glDeleteShader(vertexShader));
        MBGL_CHECK_ERROR(glDeleteShader(fragmentShader));
        MBGL_CHECK_ERROR(glDeleteProgram(program));
    }

    GLuint program = 0;
    GLuint vertexShader = 0;
    GLuint fragmentShader = 0;
    GLuint a_pos = 0;
};

struct Buffer {
    Buffer(std::vector<GLfloat> data) {
        MBGL_CHECK_ERROR(glGenBuffers(1, &buffer));
        MBGL_CHECK_ERROR(glBindBuffer(GL_ARRAY_BUFFER, buffer));
        MBGL_CHECK_ERROR(glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), data.data(), GL_STATIC_DRAW));
    }

    ~Buffer() { MBGL_CHECK_ERROR(glDeleteBuffers(1, &buffer)); }

    GLuint buffer = 0;
};

TEST(GLContextMode, Shared) {
    if (gfx::Backend::GetType() != gfx::Backend::Type::OpenGL) {
        return;
    }

    util::RunLoop loop;

    HeadlessFrontend frontend{1, gfx::HeadlessBackend::SwapBehaviour::NoFlush, gfx::ContextMode::Shared};

    Map map(frontend,
            MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()),
            ResourceOptions().withCachePath(":memory:").withAssetPath("test/fixtures/api/assets"));
    map.getStyle().loadJSON(util::read_file("test/fixtures/api/water.json"));
    map.jumpTo(CameraOptions().withCenter(LatLng{37.8, -122.5}).withZoom(10.0));

    // Set transparent background layer.
    auto layer = map.getStyle().getLayer("background");
    ASSERT_STREQ("background", layer->getTypeInfo()->type);
    static_cast<BackgroundLayer*>(layer)->setBackgroundColor({{1.0f, 0.0f, 0.0f, 0.5f}});

    {
        // Custom rendering outside of GL Native render loop.
        gfx::BackendScope scope{*frontend.getBackend()};
        frontend.getBackend()->getDefaultRenderable().getResource<gl::RenderableResource>().bind();

        Shader paintShader(vertexShaderSource, fragmentShaderSource);
        Buffer triangleBuffer({0, 0.5, 0.5, -0.5, -0.5, -0.5});
        MBGL_CHECK_ERROR(glUseProgram(paintShader.program));
        MBGL_CHECK_ERROR(glBindBuffer(GL_ARRAY_BUFFER, triangleBuffer.buffer));
        MBGL_CHECK_ERROR(glEnableVertexAttribArray(paintShader.a_pos));
        MBGL_CHECK_ERROR(glVertexAttribPointer(paintShader.a_pos, 2, GL_FLOAT, GL_FALSE, 0, nullptr));
        MBGL_CHECK_ERROR(glDrawArrays(GL_TRIANGLE_STRIP, 0, 3));

        frontend.getRenderer()->reduceMemoryUse();
        ASSERT_TRUE(static_cast<gl::Context&>(frontend.getBackend()->getContext()).empty());
    }

    test::checkImage("test/fixtures/shared_context", frontend.render(map).image, 0.5, 0.1);
}

TEST(GLContext, BufferAllocationIgnoresEarlierErrors) {
    gl::HeadlessBackend backend{{32, 32}};
    gfx::BackendScope scope{backend};

    gl::Context context{backend};
    auto commandEncoder = context.createCommandEncoder();
    auto uploadPass = commandEncoder->createUploadPass("upload", backend.getDefaultRenderable());
    auto& glUploadPass = static_cast<gl::UploadPass&>(*uploadPass);

    const std::array<float, 6> vertices = {{0.0f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f}};
    const std::array<uint16_t, 3> indices = {{0, 1, 2}};

    glBindBuffer(0, 0);
    EXPECT_NO_THROW(glUploadPass.createVertexBufferResource(
        vertices.data(), vertices.size() * sizeof(float), gfx::BufferUsageType::StaticDraw, false));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    glBindBuffer(0, 0);
    EXPECT_NO_THROW(glUploadPass.createIndexBufferResource(
        indices.data(), indices.size() * sizeof(uint16_t), gfx::BufferUsageType::StaticDraw, false));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST(GLContext, UniformBufferCloneCopiesLargeBuffer) {
    gl::HeadlessBackend backend{{32, 32}};
    gfx::BackendScope scope{backend};

    gl::Context context{backend};

    constexpr std::size_t size = 16384;
    std::vector<uint8_t> data(size);
    for (std::size_t i = 0; i < size; ++i) {
        data[i] = static_cast<uint8_t>(i % 251);
    }

    auto buffer = context.createUniformBuffer(data.data(), size, /*persistent=*/false, /*ssbo=*/false);
    ASSERT_TRUE(buffer);
    auto& glBuffer = static_cast<gl::UniformBufferGL&>(*buffer);
    ASSERT_EQ(glGetError(), GL_NO_ERROR);

    auto clone = glBuffer.clone();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    MBGL_CHECK_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, clone.getID()));
    const auto* contents = static_cast<const uint8_t*>(
        MBGL_CHECK_ERROR(glMapBufferRange(GL_COPY_READ_BUFFER, 0, size, GL_MAP_READ_BIT)));
    ASSERT_NE(contents, nullptr);
    EXPECT_EQ(0, std::memcmp(contents, data.data(), size));
    MBGL_CHECK_ERROR(glUnmapBuffer(GL_COPY_READ_BUFFER));
    MBGL_CHECK_ERROR(glBindBuffer(GL_COPY_READ_BUFFER, 0));
}

#endif
