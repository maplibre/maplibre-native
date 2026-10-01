#if MLN_RENDER_BACKEND_OPENGL
#include <mln/test/util.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gl/context.hpp>
#include <mln/gl/headless_backend.hpp>
#include <mln/platform/gl_functions.hpp>
#include <mln/shaders/gl/shader_program_gl.hpp>
#include <mln/shaders/program_parameters.hpp>

using namespace mln;

TEST(GLShaderProgram, AttributeIDsFollowReflectedNames) {
    gl::HeadlessBackend backend{{64, 64}};
    gfx::BackendScope scope{backend};
    gl::Context context{backend};
    // Descriptor order and attribute IDs do not determine linked locations.
    const auto shader = gl::ShaderProgramGL::create(context,
                                                    ProgramParameters(1, false),
                                                    "a_pos",
                                                    {},
                                                    {},
                                                    {{"a_pos", 0}, {"a_first", 4}, {"a_second", 7}},
                                                    R"GLSL(
in vec2 a_pos;
layout(location=2) in vec2 a_first;
layout(location=1) in vec2 a_second;
void main() { gl_Position = vec4(a_pos + a_first * 0.01 + a_second * 0.02, 0, 1); }
)GLSL",
                                                    "void main() { fragColor = vec4(1); }");

    const auto& attributes = shader->getVertexAttributes();
    ASSERT_TRUE(attributes.get(4));
    ASSERT_TRUE(attributes.get(7));
    EXPECT_EQ(platform::glGetAttribLocation(shader->getGLProgramID(), "a_first"), attributes.get(4)->getIndex());
    EXPECT_EQ(platform::glGetAttribLocation(shader->getGLProgramID(), "a_second"), attributes.get(7)->getIndex());
}

#endif
