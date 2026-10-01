#if MLN_RENDER_BACKEND_OPENGL

#include <mln/test/util.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gl/context.hpp>
#include <mln/gl/defines.hpp>
#include <mln/gl/headless_backend.hpp>
#include <mln/gl/uniform_buffer_gl.hpp>
#include <mln/platform/gl_functions.hpp>

#include <algorithm>
#include <cstring>
#include <vector>

using namespace mln;
using namespace mln::platform;

TEST(GLUniformBuffer, PageBoundaryAndUpdates) {
    gl::HeadlessBackend backend{{32, 32}};
    gfx::BackendScope scope{backend};
    gl::Context context{backend};

    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxSize);
    const auto error = glGetError();
    if (error == GL_INVALID_ENUM || (error == GL_NO_ERROR && maxSize < 8208)) {
        GTEST_SKIP() << "Uniform block sizes unsupported by this GL context";
    }
    ASSERT_EQ(error, GLenum(GL_NO_ERROR));

    // Small control, exactly one allocator page, and a dedicated larger buffer.
    for (const size_t size : {16u, 8192u, 8208u}) {
        SCOPED_TRACE(size);
        std::vector<uint8_t> data(size, 13);
        auto buffer = context.createUniformBuffer(data.data(), size);
        const auto& glBuffer = static_cast<const gl::UniformBufferGL&>(*buffer);
        for (uint8_t revision = 0; revision < 3; ++revision) {
            if (revision) {
                std::fill(data.begin(), data.end(), revision);
                buffer->update(data.data(), size);
            }
            // Verify initial allocation and both updates in actual GPU storage.
            glBindBuffer(GL_UNIFORM_BUFFER, glBuffer.getID());
            const auto* mapped = glMapBufferRange(
                GL_UNIFORM_BUFFER, glBuffer.getManagedBuffer().getBindingOffset(), size, GL_MAP_READ_BIT);
            ASSERT_EQ(glGetError(), GLenum(GL_NO_ERROR));
            ASSERT_NE(mapped, nullptr);
            EXPECT_EQ(std::memcmp(mapped, data.data(), size), 0);
            EXPECT_EQ(glUnmapBuffer(GL_UNIFORM_BUFFER), GLboolean(GL_TRUE));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            ASSERT_EQ(glGetError(), GLenum(GL_NO_ERROR));
        }
    }
}

#endif
