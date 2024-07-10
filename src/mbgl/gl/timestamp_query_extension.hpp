#pragma once

#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/gl/extension.hpp>

#include <functional>

#define GL_QUERY_COUNTER_BITS 0x8864
#define GL_CURRENT_QUERY 0x8865
#define GL_QUERY_RESULT 0x8866
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_TIME_ELAPSED 0x88BF
#define GL_TIMESTAMP 0x8E28
#define GL_GPU_DISJOINT 0x8FBB

namespace mbgl {
namespace gl {
namespace extension {

using namespace platform;

void glGenQueries(GLsizei n, GLuint *ids);
void glDeleteQueries(GLsizei n, const GLuint *ids);
GLboolean glIsQuery(GLuint id);
void glBeginQuery(GLenum target, GLuint id);
void glEndQuery(GLenum target);
void glQueryCounter(GLuint id, GLenum target);
void glGetQueryiv(GLenum target, GLenum pname, GLint *params);
void glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params);
void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params);
void glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64 *params);
void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 *params);
void glGetInteger64v(GLenum pname, GLint64 *data);

using GlContexsLoader = std::function<ProcAddress(std::initializer_list<std::pair<const char *, const char *>>)>;

void loadTimeStampQueryExtension(const GlContexsLoader &loadExtension);

} // namespace extension
} // namespace gl
} // namespace mbgl
