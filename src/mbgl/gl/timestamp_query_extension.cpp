
#include <mbgl/gl/timestamp_query_extension.hpp>

#include <cassert>
#include <memory>

namespace mbgl {
namespace gl {
namespace extension {

using namespace platform;

namespace {

struct TimestampQueryLoader {
    TimestampQueryLoader(const GlContexsLoader &loadExtension)
        : glGenQueries(loadExtension({{"GL_EXT_disjoint_timer_query", "glGenQueries"}})),
          glDeleteQueries(loadExtension({{"GL_EXT_disjoint_timer_query", "glDeleteQueries"}})),
          glIsQuery(loadExtension({{"GL_EXT_disjoint_timer_query", "glIsQuery"}})),
          glBeginQuery(loadExtension({{"GL_EXT_disjoint_timer_query", "glBeginQuery"}})),
          glEndQuery(loadExtension({{"GL_EXT_disjoint_timer_query", "glEndQuery"}})),
          glQueryCounter(loadExtension({{"GL_EXT_disjoint_timer_query", "glQueryCounter"}})),
          glGetQueryiv(loadExtension({{"GL_EXT_disjoint_timer_query", "glGetQueryiv"}})),
          glGetQueryObjectiv(loadExtension({{"GL_EXT_disjoint_timer_query", "glGetQueryObjectiv"}})),
          glGetQueryObjectuiv(loadExtension({{"GL_EXT_disjoint_timer_query", "glGetQueryObjectuiv"}})),
          glGetQueryObjecti64v(loadExtension({{"GL_EXT_disjoint_timer_query", "glGetQueryObjecti64v"}})),
          glGetQueryObjectui64v(loadExtension({{"GL_EXT_disjoint_timer_query", "glGetQueryObjectui64v"}})),
          glGetInteger64v(loadExtension({{"GL_EXT_disjoint_timer_query", "glGetInteger64v"}})) {}

    ExtensionFunction<void(GLsizei n, GLuint *ids)> glGenQueries;
    ExtensionFunction<void(GLsizei n, const GLuint *ids)> glDeleteQueries;
    ExtensionFunction<GLboolean(GLuint id)> glIsQuery;
    ExtensionFunction<void(GLenum target, GLuint id)> glBeginQuery;
    ExtensionFunction<void(GLenum target)> glEndQuery;
    ExtensionFunction<void(GLuint id, GLenum target)> glQueryCounter;
    ExtensionFunction<void(GLenum target, GLenum pname, GLint *params)> glGetQueryiv;
    ExtensionFunction<void(GLuint id, GLenum pname, GLint *params)> glGetQueryObjectiv;
    ExtensionFunction<void(GLuint id, GLenum pname, GLuint *params)> glGetQueryObjectuiv;
    ExtensionFunction<void(GLuint id, GLenum pname, GLint64 *params)> glGetQueryObjecti64v;
    ExtensionFunction<void(GLuint id, GLenum pname, GLuint64 *params)> glGetQueryObjectui64v;
    ExtensionFunction<void(GLenum pname, GLint64 *data)> glGetInteger64v;
};

std::unique_ptr<TimestampQueryLoader> &singleton() {
    static std::unique_ptr<TimestampQueryLoader> loader;
    return loader;
}

} // namespace

void glGenQueries(GLsizei n, GLuint *ids) {
    assert(singleton() && singleton()->glGenQueries);
    return singleton()->glGenQueries(n, ids);
}

void glDeleteQueries(GLsizei n, const GLuint *ids) {
    assert(singleton() && singleton()->glDeleteQueries);
    return singleton()->glDeleteQueries(n, ids);
}

GLboolean glIsQuery(GLuint id) {
    assert(singleton() && singleton()->glIsQuery);
    return singleton()->glIsQuery(id);
}

void glBeginQuery(GLenum target, GLuint id) {
    assert(singleton() && singleton()->glBeginQuery);
    return singleton()->glBeginQuery(target, id);
}

void glEndQuery(GLenum target) {
    assert(singleton() && singleton()->glEndQuery);
    return singleton()->glEndQuery(target);
}

void glQueryCounter(GLuint id, GLenum target) {
    assert(singleton() && singleton()->glQueryCounter);
    return singleton()->glQueryCounter(id, target);
}

void glGetQueryiv(GLenum target, GLenum pname, GLint *params) {
    assert(singleton() && singleton()->glGetQueryiv);
    return singleton()->glGetQueryiv(target, pname, params);
}

void glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params) {
    assert(singleton() && singleton()->glGetQueryObjectiv);
    return singleton()->glGetQueryObjectiv(id, pname, params);
}

void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params) {
    assert(singleton() && singleton()->glGetQueryObjectuiv);
    return singleton()->glGetQueryObjectuiv(id, pname, params);
}

void glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64 *params) {
    assert(singleton() && singleton()->glGetQueryObjecti64v);
    return singleton()->glGetQueryObjecti64v(id, pname, params);
}

void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 *params) {
    assert(singleton() && singleton()->glGetQueryObjectui64v);
    return singleton()->glGetQueryObjectui64v(id, pname, params);
}

void glGetInteger64v(GLenum pname, GLint64 *data) {
    assert(singleton() && singleton()->glGetInteger64v);
    return singleton()->glGetInteger64v(pname, data);
}

void loadTimeStampQueryExtension(const GlContexsLoader &loadExtension) {
    auto &loader = singleton();
    loader = std::make_unique<TimestampQueryLoader>(loadExtension);
}

} // namespace extension
} // namespace gl
} // namespace mbgl
