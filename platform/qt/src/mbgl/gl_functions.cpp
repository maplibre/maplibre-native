#include <mbgl/platform/gl_functions.hpp>

#include <QOpenGLContext>
#include <QOpenGLFunctions>

namespace mbgl {
namespace platform {

/* OpenGL ES 2.0 */

void (* const glActiveTexture)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glActiveTexture(args...);
};

void (* const glAttachShader)(GLuint, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glAttachShader(args...);
};

void (* const glBindAttribLocation)(GLuint, GLuint, const GLchar *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBindAttribLocation(args...);
};

void (* const glBindBuffer)(GLenum, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBindBuffer(args...);
};

void (* const glBindFramebuffer)(GLenum, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBindFramebuffer(args...);
};

void (* const glBindRenderbuffer)(GLenum, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBindRenderbuffer(args...);
};

void (* const glBindTexture)(GLenum, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBindTexture(args...);
};

void (* const glBlendColor)(GLfloat, GLfloat, GLfloat, GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBlendColor(args...);
};

void (* const glBlendEquation)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBlendEquation(args...);
};

void (* const glBlendEquationSeparate)(GLenum, GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBlendEquationSeparate(args...);
};

void (* const glBlendFunc)(GLenum, GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBlendFunc(args...);
};

void (* const glBlendFuncSeparate)(GLenum, GLenum, GLenum, GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBlendFuncSeparate(args...);
};

void (* const glBufferData)(GLenum, GLsizeiptr, const void *, GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBufferData(args...);
};

void (* const glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const void *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBufferSubData(args...);
};

GLenum (* const glCheckFramebufferStatus)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glCheckFramebufferStatus(args...);
};

void (* const glClear)(GLbitfield) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glClear(args...);
};

void (* const glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glClearColor(args...);
};

void (* const glClearDepthf)(GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glClearDepthf(args...);
};

void (* const glClearStencil)(GLint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glClearStencil(args...);
};

void (* const glColorMask)(GLboolean, GLboolean, GLboolean, GLboolean) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glColorMask(args...);
};

void (* const glCompileShader)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glCompileShader(args...);
};

void (* const glCompressedTexImage2D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glCompressedTexImage2D(args...);
};

void (* const glCompressedTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glCompressedTexSubImage2D(args...);
};

void (* const glCopyTexImage2D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glCopyTexImage2D(args...);
};

void (* const glCopyTexSubImage2D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glCopyTexSubImage2D(args...);
};

GLuint (* const glCreateProgram)() = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glCreateProgram(args...);
};

GLuint (* const glCreateShader)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glCreateShader(args...);
};

void (* const glCullFace)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glCullFace(args...);
};

void (* const glDeleteBuffers)(GLsizei, const GLuint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDeleteBuffers(args...);
};

void (* const glDeleteFramebuffers)(GLsizei, const GLuint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDeleteFramebuffers(args...);
};

void (* const glDeleteProgram)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDeleteProgram(args...);
};

void (* const glDeleteRenderbuffers)(GLsizei, const GLuint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDeleteRenderbuffers(args...);
};

void (* const glDeleteShader)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDeleteShader(args...);
};

void (* const glDeleteTextures)(GLsizei, const GLuint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDeleteTextures(args...);
};

void (* const glDepthFunc)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDepthFunc(args...);
};

void (* const glDepthMask)(GLboolean) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDepthMask(args...);
};

void (* const glDepthRangef)(GLfloat, GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDepthRangef(args...);
};

void (* const glDetachShader)(GLuint, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDetachShader(args...);
};

void (* const glDisable)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDisable(args...);
};

void (* const glDisableVertexAttribArray)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDisableVertexAttribArray(args...);
};

void (* const glDrawArrays)(GLenum, GLint, GLsizei) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDrawArrays(args...);
};

void (* const glDrawElements)(GLenum, GLsizei, GLenum, const void *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDrawElements(args...);
};

void (* const glEnable)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glEnable(args...);
};

void (* const glEnableVertexAttribArray)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glEnableVertexAttribArray(args...);
};

void (* const glFinish)() = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glFinish(args...);
};

void (* const glFlush)() = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glFlush(args...);
};

void (* const glFramebufferRenderbuffer)(GLenum, GLenum, GLenum, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glFramebufferRenderbuffer(args...);
};

void (* const glFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glFramebufferTexture2D(args...);
};

void (* const glFrontFace)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glFrontFace(args...);
};

void (* const glGenBuffers)(GLsizei, GLuint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGenBuffers(args...);
};

void (* const glGenerateMipmap)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGenerateMipmap(args...);
};

void (* const glGenFramebuffers)(GLsizei, GLuint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGenFramebuffers(args...);
};

void (* const glGenRenderbuffers)(GLsizei, GLuint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGenRenderbuffers(args...);
};

void (* const glGenTextures)(GLsizei, GLuint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGenTextures(args...);
};

void (* const glGetActiveAttrib)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetActiveAttrib(args...);
};

void (* const glGetActiveUniform)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetActiveUniform(args...);
};

void (* const glGetAttachedShaders)(GLuint, GLsizei, GLsizei *, GLuint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetAttachedShaders(args...);
};

GLint (* const glGetAttribLocation)(GLuint, const GLchar *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetAttribLocation(args...);
};

void (* const glGetBooleanv)(GLenum, GLboolean *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetBooleanv(args...);
};

void (* const glGetBufferParameteriv)(GLenum, GLenum, GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetBufferParameteriv(args...);
};

GLenum (* const glGetError)() = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetError(args...);
};

void (* const glGetFloatv)(GLenum, GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetFloatv(args...);
};

void (* const glGetFramebufferAttachmentParameteriv)(GLenum, GLenum, GLenum, GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetFramebufferAttachmentParameteriv(args...);
};

void (* const glGetIntegerv)(GLenum, GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetIntegerv(args...);
};

void (* const glGetProgramInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetProgramInfoLog(args...);
};

void (* const glGetProgramiv)(GLuint, GLenum, GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetProgramiv(args...);
};

void (* const glGetRenderbufferParameteriv)(GLenum, GLenum, GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetRenderbufferParameteriv(args...);
};

void (* const glGetShaderInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetShaderInfoLog(args...);
};

void (* const glGetShaderiv)(GLuint, GLenum, GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetShaderiv(args...);
};

void (* const glGetShaderPrecisionFormat)(GLenum, GLenum, GLint *, GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetShaderPrecisionFormat(args...);
};

void (* const glGetShaderSource)(GLuint, GLsizei, GLsizei *, GLchar *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetShaderSource(args...);
};

const GLubyte *(* const glGetString)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetString(args...);
};

void (* const glGetTexParameterfv)(GLenum, GLenum, GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetTexParameterfv(args...);
};

void (* const glGetTexParameteriv)(GLenum, GLenum, GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetTexParameteriv(args...);
};

void (* const glGetUniformfv)(GLuint, GLint, GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetUniformfv(args...);
};

void (* const glGetUniformiv)(GLuint, GLint, GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetUniformiv(args...);
};

GLint (* const glGetUniformLocation)(GLuint, const GLchar *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetUniformLocation(args...);
};

void (* const glGetVertexAttribfv)(GLuint, GLenum, GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetVertexAttribfv(args...);
};

void (* const glGetVertexAttribiv)(GLuint, GLenum, GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetVertexAttribiv(args...);
};

void (* const glGetVertexAttribPointerv)(GLuint, GLenum, void **) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetVertexAttribPointerv(args...);
};

void (* const glHint)(GLenum, GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glHint(args...);
};

GLboolean (* const glIsBuffer)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glIsBuffer(args...);
};

GLboolean (* const glIsEnabled)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glIsEnabled(args...);
};

GLboolean (* const glIsFramebuffer)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glIsFramebuffer(args...);
};

GLboolean (* const glIsProgram)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glIsProgram(args...);
};

GLboolean (* const glIsRenderbuffer)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glIsRenderbuffer(args...);
};

GLboolean (* const glIsShader)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glIsShader(args...);
};

GLboolean (* const glIsTexture)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glIsTexture(args...);
};

void (* const glLineWidth)(GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glLineWidth(args...);
};

void (* const glLinkProgram)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glLinkProgram(args...);
};

void (* const glPixelStorei)(GLenum, GLint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glPixelStorei(args...);
};

void (* const glPolygonOffset)(GLfloat, GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glPolygonOffset(args...);
};

void (* const glReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glReadPixels(args...);
};

void (* const glReleaseShaderCompiler)() = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glReleaseShaderCompiler(args...);
};

void (* const glRenderbufferStorage)(GLenum, GLenum, GLsizei, GLsizei) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glRenderbufferStorage(args...);
};

void (* const glSampleCoverage)(GLfloat, GLboolean) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glSampleCoverage(args...);
};

void (* const glScissor)(GLint, GLint, GLsizei, GLsizei) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glScissor(args...);
};

void (* const glShaderBinary)(GLsizei, const GLuint *, GLenum, const GLvoid *, GLsizei) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glShaderBinary(args...);
};

void (* const glShaderSource)(GLuint, GLsizei, const GLchar * const*, const GLint *) = [](GLuint shader, GLsizei count, const GLchar * const * string, const GLint *length) {
    return QOpenGLContext::currentContext()->functions()->glShaderSource(shader, count, const_cast<const GLchar **>(string), length);
};

void (* const glStencilFunc)(GLenum, GLint, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glStencilFunc(args...);
};

void (* const glStencilFuncSeparate)(GLenum, GLenum, GLint, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glStencilFuncSeparate(args...);
};

void (* const glStencilMask)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glStencilMask(args...);
};

void (* const glStencilMaskSeparate)(GLenum, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glStencilMaskSeparate(args...);
};

void (* const glStencilOp)(GLenum, GLenum, GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glStencilOp(args...);
};

void (* const glStencilOpSeparate)(GLenum, GLenum, GLenum, GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glStencilOpSeparate(args...);
};

void (* const glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glTexImage2D(args...);
};

void (* const glTexParameterf)(GLenum, GLenum, GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glTexParameterf(args...);
};

void (* const glTexParameterfv)(GLenum, GLenum, const GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glTexParameterfv(args...);
};

void (* const glTexParameteri)(GLenum, GLenum, GLint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glTexParameteri(args...);
};

void (* const glTexParameteriv)(GLenum, GLenum, const GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glTexParameteriv(args...);
};

void (* const glTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glTexSubImage2D(args...);
};

void (* const glUniform1f)(GLint, GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform1f(args...);
};

void (* const glUniform1fv)(GLint, GLsizei, const GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform1fv(args...);
};

void (* const glUniform1i)(GLint, GLint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform1i(args...);
};

void (* const glUniform1iv)(GLint, GLsizei, const GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform1iv(args...);
};

void (* const glUniform2f)(GLint, GLfloat, GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform2f(args...);
};

void (* const glUniform2fv)(GLint, GLsizei, const GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform2fv(args...);
};

void (* const glUniform2i)(GLint, GLint, GLint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform2i(args...);
};

void (* const glUniform2iv)(GLint, GLsizei, const GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform2iv(args...);
};

void (* const glUniform3f)(GLint, GLfloat, GLfloat, GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform3f(args...);
};

void (* const glUniform3fv)(GLint, GLsizei, const GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform3fv(args...);
};

void (* const glUniform3i)(GLint, GLint, GLint, GLint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform3i(args...);
};

void (* const glUniform3iv)(GLint, GLsizei, const GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform3iv(args...);
};

void (* const glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform4f(args...);
};

void (* const glUniform4fv)(GLint, GLsizei, const GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform4fv(args...);
};

void (* const glUniform4i)(GLint, GLint, GLint, GLint, GLint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform4i(args...);
};

void (* const glUniform4iv)(GLint, GLsizei, const GLint *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform4iv(args...);
};

void (* const glUniformMatrix2fv)(GLint, GLsizei, GLboolean, const GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniformMatrix2fv(args...);
};

void (* const glUniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniformMatrix3fv(args...);
};

void (* const glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniformMatrix4fv(args...);
};

void (* const glUseProgram)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUseProgram(args...);
};

void (* const glValidateProgram)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glValidateProgram(args...);
};

void (* const glVertexAttrib1f)(GLuint, GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttrib1f(args...);
};

void (* const glVertexAttrib1fv)(GLuint, const GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttrib1fv(args...);
};

void (* const glVertexAttrib2f)(GLuint, GLfloat, GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttrib2f(args...);
};

void (* const glVertexAttrib2fv)(GLuint, const GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttrib2fv(args...);
};

void (* const glVertexAttrib3f)(GLuint, GLfloat, GLfloat, GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttrib3f(args...);
};

void (* const glVertexAttrib3fv)(GLuint, const GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttrib3fv(args...);
};

void (* const glVertexAttrib4f)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttrib4f(args...);
};

void (* const glVertexAttrib4fv)(GLuint, const GLfloat *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttrib4fv(args...);
};

void (* const glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttribPointer(args...);
};

void (* const glViewport)(GLint, GLint, GLsizei, GLsizei) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glViewport(args...);
};

/* OpenGL ES 3.0 */

void (* const glReadBuffer)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glReadBuffer(args...);
};
void (* const glDrawRangeElements)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDrawRangeElements(args...);
};
void (* const glTexImage3D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glTexImage3D(args...);
};
void (* const glTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glTexSubImage3D(args...);
};
void (* const glCopyTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glCopyTexSubImage3D(args...);
};
void (* const glCompressedTexImage3D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glCompressedTexImage3D(args...);
};
void (* const glCompressedTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glCompressedTexSubImage3D(args...);
};
void (* const glGenQueries)(GLsizei, GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGenQueries(args...);
};
void (* const glDeleteQueries)(GLsizei, const GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDeleteQueries(args...);
};
GLboolean (* const glIsQuery)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glIsQuery(args...);
};
void (* const glBeginQuery)(GLenum, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBeginQuery(args...);
};
void (* const glEndQuery)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glEndQuery(args...);
};
void (* const glGetQueryiv)(GLenum, GLenum, GLint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetQueryiv(args...);
};
void (* const glGetQueryObjectuiv)(GLuint, GLenum, GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetQueryObjectuiv(args...);
};
GLboolean (* const glUnmapBuffer)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUnmapBuffer(args...);
};
void (* const glGetBufferPointerv)(GLenum, GLenum, GLvoid**) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetBufferPointerv(args...);
};
void (* const glDrawBuffers)(GLsizei, const GLenum*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDrawBuffers(args...);
};
void (* const glUniformMatrix2x3fv)(GLint, GLsizei, GLboolean, const GLfloat*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniformMatrix2x3fv(args...);
};
void (* const glUniformMatrix3x2fv)(GLint, GLsizei, GLboolean, const GLfloat*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniformMatrix3x2fv(args...);
};
void (* const glUniformMatrix2x4fv)(GLint, GLsizei, GLboolean, const GLfloat*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniformMatrix2x4fv(args...);
};
void (* const glUniformMatrix4x2fv)(GLint, GLsizei, GLboolean, const GLfloat*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniformMatrix4x2fv(args...);
};
void (* const glUniformMatrix3x4fv)(GLint, GLsizei, GLboolean, const GLfloat*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniformMatrix3x4fv(args...);
};
void (* const glUniformMatrix4x3fv)(GLint, GLsizei, GLboolean, const GLfloat*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniformMatrix4x3fv(args...);
};
void (* const glBlitFramebuffer)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBlitFramebuffer(args...);
};
void (* const glRenderbufferStorageMultisample)(GLenum, GLsizei, GLenum, GLsizei, GLsizei) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glRenderbufferStorageMultisample(args...);
};
void (* const glFramebufferTextureLayer)(GLenum, GLenum, GLuint, GLint, GLint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glFramebufferTextureLayer(args...);
};
GLvoid* (* const glMapBufferRange)(GLenum, GLintptr, GLsizeiptr, GLbitfield) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glMapBufferRange(args...);
};
void (* const glFlushMappedBufferRange)(GLenum, GLintptr, GLsizeiptr) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glFlushMappedBufferRange(args...);
};
void (* const glBindVertexArray)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBindVertexArray(args...);
};
void (* const glDeleteVertexArrays)(GLsizei, const GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDeleteVertexArrays(args...);
};
void (* const glGenVertexArrays)(GLsizei, GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGenVertexArrays(args...);
};
GLboolean (* const glIsVertexArray)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glIsVertexArray(args...);
};
void (* const glGetIntegeri_v)(GLenum, GLuint, GLint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetIntegeri_v(args...);
};
void (* const glBeginTransformFeedback)(GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBeginTransformFeedback(args...);
};
void (* const glEndTransformFeedback)() = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glEndTransformFeedback(args...);
};
void (* const glBindBufferRange)(GLenum, GLuint, GLuint, GLintptr, GLsizeiptr) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBindBufferRange(args...);
};
void (* const glBindBufferBase)(GLenum, GLuint, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBindBufferBase(args...);
};
void (* const glTransformFeedbackVaryings)(GLuint, GLsizei, const GLchar* const, GLenum) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glTransformFeedbackVaryings(args...);
};
void (* const glGetTransformFeedbackVarying)(GLuint, GLuint, GLsizei, GLsizei*, GLsizei*, GLenum*, GLchar*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetTransformFeedbackVarying(args...);
};
void (* const glVertexAttribIPointer)(GLuint, GLint, GLenum, GLsizei, const GLvoid*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttribIPointer(args...);
};
void (* const glGetVertexAttribIiv)(GLuint, GLenum, GLint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetVertexAttribIiv(args...);
};
void (* const glGetVertexAttribIuiv)(GLuint, GLenum, GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetVertexAttribIuiv(args...);
};
void (* const glVertexAttribI4i)(GLuint, GLint, GLint, GLint, GLint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttribI4i(args...);
};
void (* const glVertexAttribI4ui)(GLuint, GLuint, GLuint, GLuint, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttribI4ui(args...);
};
void (* const glVertexAttribI4iv)(GLuint, const GLint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttribI4iv(args...);
};
void (* const glVertexAttribI4uiv)(GLuint, const GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttribI4uiv(args...);
};
void (* const glGetUniformuiv)(GLuint, GLint, GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetUniformuiv(args...);
};
GLint (* const glGetFragDataLocation)(GLuint, const GLchar) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetFragDataLocation(args...);
};
void (* const glUniform1ui)(GLint, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform1ui(args...);
};
void (* const glUniform2ui)(GLint, GLuint, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform2ui(args...);
};
void (* const glUniform3ui)(GLint, GLuint, GLuint, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform3ui(args...);
};
void (* const glUniform4ui)(GLint, GLuint, GLuint, GLuint, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform4ui(args...);
};
void (* const glUniform1uiv)(GLint, GLsizei, const GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform1uiv(args...);
};
void (* const glUniform2uiv)(GLint, GLsizei, const GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform2uiv(args...);
};
void (* const glUniform3uiv)(GLint, GLsizei, const GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform3uiv(args...);
};
void (* const glUniform4uiv)(GLint, GLsizei, const GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniform4uiv(args...);
};
void (* const glClearBufferiv)(GLenum, GLint, const GLint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glClearBufferiv(args...);
};
void (* const glClearBufferuiv)(GLenum, GLint, const GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glClearBufferuiv(args...);
};
void (* const glClearBufferfv)(GLenum, GLint, const GLfloat*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glClearBufferfv(args...);
};
void (* const glClearBufferfi)(GLenum, GLint, GLfloat, GLint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glClearBufferfi(args...);
};
const GLubyte* (* const glGetStringi)(GLenum, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetStringi(args...);
};
void (* const glCopyBufferSubData)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glCopyBufferSubData(args...);
};
void (* const glGetUniformIndices)(GLuint, GLsizei, const GLchar* const, GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetUniformIndices(args...);
};
void (* const glGetActiveUniformsiv)(GLuint, GLsizei, const GLuint*, GLenum, GLint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetActiveUniformsiv(args...);
};
GLuint (* const glGetUniformBlockIndex)(GLuint, const GLchar*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetUniformBlockIndex(args...);
};
void (* const glGetActiveUniformBlockiv)(GLuint, GLuint, GLenum, GLint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetActiveUniformBlockiv(args...);
};
void (* const glGetActiveUniformBlockName)(GLuint, GLuint, GLsizei, GLsizei*, GLchar*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetActiveUniformBlockName(args...);
};
void (* const glUniformBlockBinding)(GLuint, GLuint, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glUniformBlockBinding(args...);
};
void (* const glDrawArraysInstanced)(GLenum, GLint, GLsizei, GLsizei) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDrawArraysInstanced(args...);
};
void (* const glDrawElementsInstanced)(GLenum, GLsizei, GLenum, const GLvoid*, GLsizei) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDrawElementsInstanced(args...);
};
GLsync (* const glFenceSync)(GLenum, GLbitfield) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glFenceSync(args...);
};
GLboolean (* const glIsSync)(GLsync) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glIsSync(args...);
};
void (* const glDeleteSync)(GLsync) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDeleteSync(args...);
};
GLenum (* const glClientWaitSync)(GLsync, GLbitfield, GLuint64) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glClientWaitSync(args...);
};
void (* const glWaitSync)(GLsync, GLbitfield, GLuint64) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glWaitSync(args...);
};
void (* const glGetInteger64v)(GLenum, GLint64*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetInteger64v(args...);
};
void (* const glGetSynciv)(GLsync, GLenum, GLsizei, GLsizei*, GLint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetSynciv(args...);
};
void (* const glGetInteger64i_v)(GLenum, GLuint, GLint64*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetInteger64i_v(args...);
};
void (* const glGetBufferParameteri64v)(GLenum, GLenum, GLint64*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetBufferParameteri64v(args...);
};
void (* const glGenSamplers)(GLsizei, GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGenSamplers(args...);
};
void (* const glDeleteSamplers)(GLsizei, const GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDeleteSamplers(args...);
};
GLboolean (* const glIsSampler)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glIsSampler(args...);
};
void (* const glBindSampler)(GLuint, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBindSampler(args...);
};
void (* const glSamplerParameteri)(GLuint, GLenum, GLint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glSamplerParameteri(args...);
};
void (* const glSamplerParameteriv)(GLuint, GLenum, const GLint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glSamplerParameteriv(args...);
};
void (* const glSamplerParameterf)(GLuint, GLenum, GLfloat) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glSamplerParameterf(args...);
};
void (* const glSamplerParameterfv)(GLuint, GLenum, const GLfloat*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glSamplerParameterfv(args...);
};
void (* const glGetSamplerParameteriv)(GLuint, GLenum, GLint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetSamplerParameteriv(args...);
};
void (* const glGetSamplerParameterfv)(GLuint, GLenum, GLfloat*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetSamplerParameterfv(args...);
};
void (* const glVertexAttribDivisor)(GLuint, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glVertexAttribDivisor(args...);
};
void (* const glBindTransformFeedback)(GLenum, GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glBindTransformFeedback(args...);
};
void (* const glDeleteTransformFeedbacks)(GLsizei, const GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glDeleteTransformFeedbacks(args...);
};
void (* const glGenTransformFeedbacks)(GLsizei, GLuint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGenTransformFeedbacks(args...);
};
GLboolean (* const glIsTransformFeedback)(GLuint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glIsTransformFeedback(args...);
};
void (* const glPauseTransformFeedback)() = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glPauseTransformFeedback(args...);
};
void (* const glResumeTransformFeedback)() = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glResumeTransformFeedback(args...);
};
void (* const glGetProgramBinary)(GLuint, GLsizei, GLsizei*, GLenum*, GLvoid*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetProgramBinary(args...);
};
void (* const glProgramBinary)(GLuint, GLenum, const GLvoid*, GLsizei) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glProgramBinary(args...);
};
void (* const glProgramParameteri)(GLuint, GLenum, GLint) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glProgramParameteri(args...);
};
void (* const glInvalidateFramebuffer)(GLenum, GLsizei, const GLenum*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glInvalidateFramebuffer(args...);
};
void (* const glInvalidateSubFramebuffer)(GLenum, GLsizei, const GLenum*, GLint, GLint, GLsizei, GLsizei) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glInvalidateSubFramebuffer(args...);
};
void (* const glTexStorage2D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glTexStorage2D(args...);
};
void (* const glTexStorage3D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glTexStorage3D(args...);
};
void (* const glGetInternalformativ)(GLenum, GLenum, GLenum, GLsizei, GLint*) = [](auto... args) {
    return QOpenGLContext::currentContext()->functions()->glGetInternalformativ(args...);
};

}  // namespace platform
}  // namespace mbgl