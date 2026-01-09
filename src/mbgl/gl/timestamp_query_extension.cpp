
#include <mbgl/gl/timestamp_query_extension.hpp>

#include <cassert>
#include <memory>

namespace mbgl {
namespace gl {
namespace extension {

using namespace platform;

namespace {

constexpr const char *const extName = "GL_EXT_disjoint_timer_query";

struct TimestampQueryLoader {
    TimestampQueryLoader(const GlContexsLoader &loadExtension)
        : glGenQueries(loadExtension({{extName, "glGenQueries"}, {extName, "glGenQueriesEXT"}})),
          glDeleteQueries(loadExtension({{extName, "glDeleteQueries"}, {extName, "glDeleteQueriesEXT"}})),
          glIsQuery(loadExtension({{extName, "glIsQuery"}, {extName, "glIsQueryEXT"}})),
          glBeginQuery(loadExtension({{extName, "glBeginQuery"}, {extName, "glBeginQueryEXT"}})),
          glEndQuery(loadExtension({{extName, "glEndQuery"}, {extName, "glEndQueryEXT"}})),
          glQueryCounter(loadExtension({{extName, "glQueryCounter"}, {extName, "glQueryCounterEXT"}})),
          glGetQueryiv(loadExtension({{extName, "glGetQueryiv"}, {extName, "glGetQueryivEXT"}})),
          glGetQueryObjectiv(loadExtension({{extName, "glGetQueryObjectiv"}, {extName, "glGetQueryObjectivEXT"}})),
          glGetQueryObjectuiv(loadExtension({{extName, "glGetQueryObjectuiv"}, {extName, "glGetQueryObjectuivEXT"}})),
          glGetQueryObjecti64v(
              loadExtension({{extName, "glGetQueryObjecti64v"}, {extName, "glGetQueryObjecti64vEXT"}})),
          glGetQueryObjectui64v(
              loadExtension({{extName, "glGetQueryObjectui64v"}, {extName, "glGetQueryObjectui64vEXT"}})),
          glGetInteger64v(loadExtension({{extName, "glGetInteger64v"}, {extName, "glGetInteger64vEXT"}})) {}

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
    singleton()->glGenQueries(n, ids);
}

void glDeleteQueries(GLsizei n, const GLuint *ids) {
    assert(singleton() && singleton()->glDeleteQueries);
    singleton()->glDeleteQueries(n, ids);
}

GLboolean glIsQuery(GLuint id) {
    assert(singleton() && singleton()->glIsQuery);
    return singleton()->glIsQuery(id);
}

void glBeginQuery(GLenum target, GLuint id) {
    assert(singleton() && singleton()->glBeginQuery);
    singleton()->glBeginQuery(target, id);
}

void glEndQuery(GLenum target) {
    assert(singleton() && singleton()->glEndQuery);
    singleton()->glEndQuery(target);
}

void glQueryCounter(GLuint id, GLenum target) {
    assert(singleton() && singleton()->glQueryCounter);
    singleton()->glQueryCounter(id, target);
}

void glGetQueryiv(GLenum target, GLenum pname, GLint *params) {
    assert(singleton() && singleton()->glGetQueryiv);
    singleton()->glGetQueryiv(target, pname, params);
}

void glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params) {
    assert(singleton() && singleton()->glGetQueryObjectiv);
    singleton()->glGetQueryObjectiv(id, pname, params);
}

void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params) {
    assert(singleton() && singleton()->glGetQueryObjectuiv);
    singleton()->glGetQueryObjectuiv(id, pname, params);
}

void glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64 *params) {
    assert(singleton() && singleton()->glGetQueryObjecti64v);
    singleton()->glGetQueryObjecti64v(id, pname, params);
}

void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 *params) {
    assert(singleton() && singleton()->glGetQueryObjectui64v);
    singleton()->glGetQueryObjectui64v(id, pname, params);
}

void glGetInteger64v(GLenum pname, GLint64 *data) {
    assert(singleton() && singleton()->glGetInteger64v);
    singleton()->glGetInteger64v(pname, data);
}

void loadTimeStampQueryExtension(const GlContexsLoader &loadExtension) {
    auto &loader = singleton();
    loader = std::make_unique<TimestampQueryLoader>(loadExtension);
}

} // namespace extension
} // namespace gl
} // namespace mbgl
