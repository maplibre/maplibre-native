#pragma once

#include <cstddef>
#include <cstdint>

// Pointers to OpenGL ES functions. They must be
// initialized by the platform at linking time.

#ifndef NDEBUG
#define MBGL_CHECK_ERROR(cmd)                                           \
    ([&]() {                                                            \
        struct __MBGL_CHECK_ERROR {                                     \
            ~__MBGL_CHECK_ERROR() noexcept(false) {                     \
                mbgl::platform::glCheckError(#cmd, __FILE__, __LINE__); \
            }                                                           \
        } __MBGL_CHECK_ERROR;                                           \
        return cmd;                                                     \
    }())
#else
#define MBGL_CHECK_ERROR(cmd) (cmd)
#endif

struct __GLsync;

namespace mbgl {
namespace platform {

using GLbitfield = unsigned int;
using GLboolean = unsigned char;
using GLchar = char;
using GLdouble = double;
using GLenum = unsigned int;
using GLfloat = float;
using GLint = int;
using GLsizei = int;
using GLubyte = unsigned char;
using GLuint = unsigned int;
using GLvoid = void;
using GLint64 = int64_t;
using GLuint64 = uint64_t;
using GLsync = ::__GLsync*;

#if defined(_WIN32)
using GLintptr = long long;
using GLsizeiptr = long long;
#else
using GLintptr = long;
using GLsizeiptr = long;
#endif

/* OpenGL ES 2.0 */

/// Pointer to glActiveTexture OpenGL function.
extern void (*const glActiveTexture)(GLenum);
/// Pointer to glAttachShader OpenGL function.
extern void (*const glAttachShader)(GLuint, GLuint);
/// Pointer to glBindAttribLocation OpenGL function.
extern void (*const glBindAttribLocation)(GLuint, GLuint, const GLchar*);
/// Pointer to glBindBuffer OpenGL function.
extern void (*const glBindBuffer)(GLenum, GLuint);
/// Pointer to glBindFramebuffer OpenGL function.
extern void (*const glBindFramebuffer)(GLenum, GLuint);
/// Pointer to glBindRenderbuffer OpenGL function.
extern void (*const glBindRenderbuffer)(GLenum, GLuint);
/// Pointer to glBindTexture OpenGL function.
extern void (*const glBindTexture)(GLenum, GLuint);
/// Pointer to glBlendColor OpenGL function.
extern void (*const glBlendColor)(GLfloat, GLfloat, GLfloat, GLfloat);
/// Pointer to glBlendEquation OpenGL function.
extern void (*const glBlendEquation)(GLenum);
/// Pointer to glBlendEquationSeparate OpenGL function.
extern void (*const glBlendEquationSeparate)(GLenum, GLenum);
/// Pointer to glBlendFunc OpenGL function.
extern void (*const glBlendFunc)(GLenum, GLenum);
/// Pointer to glBlendFuncSeparate OpenGL function.
extern void (*const glBlendFuncSeparate)(GLenum, GLenum, GLenum, GLenum);
/// Pointer to glBufferData OpenGL function.
extern void (*const glBufferData)(GLenum, GLsizeiptr, const void*, GLenum);
/// Pointer to glBufferSubData OpenGL function.
extern void (*const glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const void*);
/// Pointer to glCheckFramebufferStatus OpenGL function.
extern GLenum (*const glCheckFramebufferStatus)(GLenum);
/// Pointer to glClear OpenGL function.
extern void (*const glClear)(GLbitfield);
/// Pointer to glClearColor OpenGL function.
extern void (*const glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat);
/// Pointer to glClearDepthf OpenGL function.
extern void (*const glClearDepthf)(GLfloat);
/// Pointer to glClearStencil OpenGL function.
extern void (*const glClearStencil)(GLint);
/// Pointer to glColorMask OpenGL function.
extern void (*const glColorMask)(GLboolean, GLboolean, GLboolean, GLboolean);
/// Pointer to glCompileShader OpenGL function.
extern void (*const glCompileShader)(GLuint);
/// Pointer to glCompressedTexImage2D OpenGL function.
extern void (*const glCompressedTexImage2D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void*);
/// Pointer to glCompressedTexSubImage2D OpenGL function.
extern void (*const glCompressedTexSubImage2D)(
    GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*);
/// Pointer to glCopyTexImage2D OpenGL function.
extern void (*const glCopyTexImage2D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
/// Pointer to glCopyTexSubImage2D OpenGL function.
extern void (*const glCopyTexSubImage2D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
/// Pointer to glCreateProgram OpenGL function.
extern GLuint (*const glCreateProgram)();
/// Pointer to glCreateShader OpenGL function.
extern GLuint (*const glCreateShader)(GLenum);
/// Pointer to glCullFace OpenGL function.
extern void (*const glCullFace)(GLenum);
/// Pointer to glDeleteBuffers OpenGL function.
extern void (*const glDeleteBuffers)(GLsizei, const GLuint*);
/// Pointer to glDeleteFramebuffers OpenGL function.
extern void (*const glDeleteFramebuffers)(GLsizei, const GLuint*);
/// Pointer to glDeleteProgram OpenGL function.
extern void (*const glDeleteProgram)(GLuint);
/// Pointer to glDeleteRenderbuffers OpenGL function.
extern void (*const glDeleteRenderbuffers)(GLsizei, const GLuint*);
/// Pointer to glDeleteShader OpenGL function.
extern void (*const glDeleteShader)(GLuint);
/// Pointer to glDeleteTextures OpenGL function.
extern void (*const glDeleteTextures)(GLsizei, const GLuint*);
/// Pointer to glDepthFunc OpenGL function.
extern void (*const glDepthFunc)(GLenum);
/// Pointer to glDepthMask OpenGL function.
extern void (*const glDepthMask)(GLboolean);
/// Pointer to glDepthRangef OpenGL function.
extern void (*const glDepthRangef)(GLfloat, GLfloat);
/// Pointer to glDetachShader OpenGL function.
extern void (*const glDetachShader)(GLuint, GLuint);
/// Pointer to glDisable OpenGL function.
extern void (*const glDisable)(GLenum);
/// Pointer to glDisableVertexAttribArray OpenGL function.
extern void (*const glDisableVertexAttribArray)(GLuint);
/// Pointer to glDrawArrays OpenGL function.
extern void (*const glDrawArrays)(GLenum, GLint, GLsizei);
/// Pointer to glDrawElements OpenGL function.
extern void (*const glDrawElements)(GLenum, GLsizei, GLenum, const void*);
/// Pointer to glEnable OpenGL function.
extern void (*const glEnable)(GLenum);
/// Pointer to glEnableVertexAttribArray OpenGL function.
extern void (*const glEnableVertexAttribArray)(GLuint);
/// Pointer to glFinish OpenGL function.
extern void (*const glFinish)();
/// Pointer to glFlush OpenGL function.
extern void (*const glFlush)();
/// Pointer to glFramebufferRenderbuffer OpenGL function.
extern void (*const glFramebufferRenderbuffer)(GLenum, GLenum, GLenum, GLuint);
/// Pointer to glFramebufferTexture2D OpenGL function.
extern void (*const glFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint);
/// Pointer to glFrontFace OpenGL function.
extern void (*const glFrontFace)(GLenum);
/// Pointer to glGenBuffers OpenGL function.
extern void (*const glGenBuffers)(GLsizei, GLuint*);
/// Pointer to glGenerateMipmap OpenGL function.
extern void (*const glGenerateMipmap)(GLenum);
/// Pointer to glGenFramebuffers OpenGL function.
extern void (*const glGenFramebuffers)(GLsizei, GLuint*);
/// Pointer to glGenRenderbuffers OpenGL function.
extern void (*const glGenRenderbuffers)(GLsizei, GLuint*);
/// Pointer to glGenTextures OpenGL function.
extern void (*const glGenTextures)(GLsizei, GLuint*);
/// Pointer to glGetActiveAttrib OpenGL function.
extern void (*const glGetActiveAttrib)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
/// Pointer to glGetActiveUniform OpenGL function.
extern void (*const glGetActiveUniform)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
/// Pointer to glGetAttachedShaders OpenGL function.
extern void (*const glGetAttachedShaders)(GLuint, GLsizei, GLsizei*, GLuint*);
/// Pointer to glGetAttribLocation OpenGL function.
extern GLint (*const glGetAttribLocation)(GLuint, const GLchar*);
/// Pointer to glGetBooleanv OpenGL function.
extern void (*const glGetBooleanv)(GLenum, GLboolean*);
/// Pointer to glGetBufferParameteriv OpenGL function.
extern void (*const glGetBufferParameteriv)(GLenum, GLenum, GLint*);
/// Pointer to glGetError OpenGL function.
extern GLenum (*const glGetError)();
/// Pointer to glGetFloatv OpenGL function.
extern void (*const glGetFloatv)(GLenum, GLfloat*);
/// Pointer to glGetFramebufferAttachmentParameteriv OpenGL function.
extern void (*const glGetFramebufferAttachmentParameteriv)(GLenum, GLenum, GLenum, GLint*);
/// Pointer to glGetIntegerv OpenGL function.
extern void (*const glGetIntegerv)(GLenum, GLint*);
/// Pointer to glGetProgramInfoLog OpenGL function.
extern void (*const glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
/// Pointer to glGetProgramiv OpenGL function.
extern void (*const glGetProgramiv)(GLuint, GLenum, GLint*);
/// Pointer to glGetRenderbufferParameteriv OpenGL function.
extern void (*const glGetRenderbufferParameteriv)(GLenum, GLenum, GLint*);
/// Pointer to glGetShaderInfoLog OpenGL function.
extern void (*const glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
/// Pointer to glGetShaderiv OpenGL function.
extern void (*const glGetShaderiv)(GLuint, GLenum, GLint*);
/// Pointer to glGetShaderPrecisionFormat OpenGL function.
extern void (*const glGetShaderPrecisionFormat)(GLenum, GLenum, GLint*, GLint*);
/// Pointer to glGetShaderSource OpenGL function.
extern void (*const glGetShaderSource)(GLuint, GLsizei, GLsizei*, GLchar*);
/// Pointer to glGetString OpenGL function.
extern const GLubyte* (*const glGetString)(GLenum);
/// Pointer to glGetTexParameterfv OpenGL function.
extern void (*const glGetTexParameterfv)(GLenum, GLenum, GLfloat*);
/// Pointer to glGetTexParameteriv OpenGL function.
extern void (*const glGetTexParameteriv)(GLenum, GLenum, GLint*);
/// Pointer to glGetUniformfv OpenGL function.
extern void (*const glGetUniformfv)(GLuint, GLint, GLfloat*);
/// Pointer to glGetUniformiv OpenGL function.
extern void (*const glGetUniformiv)(GLuint, GLint, GLint*);
/// Pointer to glGetUniformLocation OpenGL function.
extern GLint (*const glGetUniformLocation)(GLuint, const GLchar*);
/// Pointer to glGetVertexAttribfv OpenGL function.
extern void (*const glGetVertexAttribfv)(GLuint, GLenum, GLfloat*);
/// Pointer to glGetVertexAttribiv OpenGL function.
extern void (*const glGetVertexAttribiv)(GLuint, GLenum, GLint*);
/// Pointer to glGetVertexAttribPointerv OpenGL function.
extern void (*const glGetVertexAttribPointerv)(GLuint, GLenum, void**);
/// Pointer to glHint OpenGL function.
extern void (*const glHint)(GLenum, GLenum);
/// Pointer to glIsBuffer OpenGL function.
extern GLboolean (*const glIsBuffer)(GLuint);
/// Pointer to glIsEnabled OpenGL function.
extern GLboolean (*const glIsEnabled)(GLenum);
/// Pointer to glIsFramebuffer OpenGL function.
extern GLboolean (*const glIsFramebuffer)(GLuint);
/// Pointer to glIsProgram OpenGL function.
extern GLboolean (*const glIsProgram)(GLuint);
/// Pointer to glIsRenderbuffer OpenGL function.
extern GLboolean (*const glIsRenderbuffer)(GLuint);
/// Pointer to glIsShader OpenGL function.
extern GLboolean (*const glIsShader)(GLuint);
/// Pointer to glIsTexture OpenGL function.
extern GLboolean (*const glIsTexture)(GLuint);
/// Pointer to glLineWidth OpenGL function.
extern void (*const glLineWidth)(GLfloat);
/// Pointer to glLinkProgram OpenGL function.
extern void (*const glLinkProgram)(GLuint);
/// Pointer to glPixelStorei OpenGL function.
extern void (*const glPixelStorei)(GLenum, GLint);
/// Pointer to glPolygonOffset OpenGL function.
extern void (*const glPolygonOffset)(GLfloat, GLfloat);
/// Pointer to glReadPixels OpenGL function.
extern void (*const glReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*);
/// Pointer to glReleaseShaderCompiler OpenGL function.
extern void (*const glReleaseShaderCompiler)();
/// Pointer to glRenderbufferStorage OpenGL function.
extern void (*const glRenderbufferStorage)(GLenum, GLenum, GLsizei, GLsizei);
/// Pointer to glSampleCoverage OpenGL function.
extern void (*const glSampleCoverage)(GLfloat, GLboolean);
/// Pointer to glScissor OpenGL function.
extern void (*const glScissor)(GLint, GLint, GLsizei, GLsizei);
/// Pointer to glShaderBinary OpenGL function.
extern void (*const glShaderBinary)(GLsizei, const GLuint*, GLenum, const GLvoid*, GLsizei);
/// Pointer to glShaderSource OpenGL function.
extern void (*const glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*);
/// Pointer to glStencilFunc OpenGL function.
extern void (*const glStencilFunc)(GLenum, GLint, GLuint);
/// Pointer to glStencilFuncSeparate OpenGL function.
extern void (*const glStencilFuncSeparate)(GLenum, GLenum, GLint, GLuint);
/// Pointer to glStencilMask OpenGL function.
extern void (*const glStencilMask)(GLuint);
/// Pointer to glStencilMaskSeparate OpenGL function.
extern void (*const glStencilMaskSeparate)(GLenum, GLuint);
/// Pointer to glStencilOp OpenGL function.
extern void (*const glStencilOp)(GLenum, GLenum, GLenum);
/// Pointer to glStencilOpSeparate OpenGL function.
extern void (*const glStencilOpSeparate)(GLenum, GLenum, GLenum, GLenum);
/// Pointer to glTexImage2D OpenGL function.
extern void (*const glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
/// Pointer to glTexParameterf OpenGL function.
extern void (*const glTexParameterf)(GLenum, GLenum, GLfloat);
/// Pointer to glTexParameterfv OpenGL function.
extern void (*const glTexParameterfv)(GLenum, GLenum, const GLfloat*);
/// Pointer to glTexParameteri OpenGL function.
extern void (*const glTexParameteri)(GLenum, GLenum, GLint);
/// Pointer to glTexParameteriv OpenGL function.
extern void (*const glTexParameteriv)(GLenum, GLenum, const GLint*);
/// Pointer to glTexSubImage2D OpenGL function.
extern void (*const glTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*);
/// Pointer to glUniform1f OpenGL function.
extern void (*const glUniform1f)(GLint, GLfloat);
/// Pointer to glUniform1fv OpenGL function.
extern void (*const glUniform1fv)(GLint, GLsizei, const GLfloat*);
/// Pointer to glUniform1i OpenGL function.
extern void (*const glUniform1i)(GLint, GLint);
/// Pointer to glUniform1iv OpenGL function.
extern void (*const glUniform1iv)(GLint, GLsizei, const GLint*);
/// Pointer to glUniform2f OpenGL function.
extern void (*const glUniform2f)(GLint, GLfloat, GLfloat);
/// Pointer to glUniform2fv OpenGL function.
extern void (*const glUniform2fv)(GLint, GLsizei, const GLfloat*);
/// Pointer to glUniform2i OpenGL function.
extern void (*const glUniform2i)(GLint, GLint, GLint);
/// Pointer to glUniform2iv OpenGL function.
extern void (*const glUniform2iv)(GLint, GLsizei, const GLint*);
/// Pointer to glUniform3f OpenGL function.
extern void (*const glUniform3f)(GLint, GLfloat, GLfloat, GLfloat);
/// Pointer to glUniform3fv OpenGL function.
extern void (*const glUniform3fv)(GLint, GLsizei, const GLfloat*);
/// Pointer to glUniform3i OpenGL function.
extern void (*const glUniform3i)(GLint, GLint, GLint, GLint);
/// Pointer to glUniform3iv OpenGL function.
extern void (*const glUniform3iv)(GLint, GLsizei, const GLint*);
/// Pointer to glUniform4f OpenGL function.
extern void (*const glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
/// Pointer to glUniform4fv OpenGL function.
extern void (*const glUniform4fv)(GLint, GLsizei, const GLfloat*);
/// Pointer to glUniform4i OpenGL function.
extern void (*const glUniform4i)(GLint, GLint, GLint, GLint, GLint);
/// Pointer to glUniform4iv OpenGL function.
extern void (*const glUniform4iv)(GLint, GLsizei, const GLint*);
/// Pointer to glUniformMatrix2fv OpenGL function.
extern void (*const glUniformMatrix2fv)(GLint, GLsizei, GLboolean, const GLfloat*);
/// Pointer to glUniformMatrix3fv OpenGL function.
extern void (*const glUniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat*);
/// Pointer to glUniformMatrix4fv OpenGL function.
extern void (*const glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*);
/// Pointer to glUseProgram OpenGL function.
extern void (*const glUseProgram)(GLuint);
/// Pointer to glValidateProgram OpenGL function.
extern void (*const glValidateProgram)(GLuint);
/// Pointer to glVertexAttrib1f OpenGL function.
extern void (*const glVertexAttrib1f)(GLuint, GLfloat);
/// Pointer to glVertexAttrib1fv OpenGL function.
extern void (*const glVertexAttrib1fv)(GLuint, const GLfloat*);
/// Pointer to glVertexAttrib2f OpenGL function.
extern void (*const glVertexAttrib2f)(GLuint, GLfloat, GLfloat);
/// Pointer to glVertexAttrib2fv OpenGL function.
extern void (*const glVertexAttrib2fv)(GLuint, const GLfloat*);
/// Pointer to glVertexAttrib3f OpenGL function.
extern void (*const glVertexAttrib3f)(GLuint, GLfloat, GLfloat, GLfloat);
/// Pointer to glVertexAttrib3fv OpenGL function.
extern void (*const glVertexAttrib3fv)(GLuint, const GLfloat*);
/// Pointer to glVertexAttrib4f OpenGL function.
extern void (*const glVertexAttrib4f)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
/// Pointer to glVertexAttrib4fv OpenGL function.
extern void (*const glVertexAttrib4fv)(GLuint, const GLfloat*);
/// Pointer to glVertexAttribPointer OpenGL function.
extern void (*const glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
/// Pointer to glViewport OpenGL function.
extern void (*const glViewport)(GLint, GLint, GLsizei, GLsizei);

/* OpenGL ES 3.0 */

/// Pointer to glReadBuffer OpenGL function.
extern void (*const glReadBuffer)(GLenum);
/// Pointer to glDrawRangeElements OpenGL function.
extern void (*const glDrawRangeElements)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid*);
/// Pointer to glTexImage3D OpenGL function.
extern void (*const glTexImage3D)(
    GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*);
/// Pointer to glTexSubImage3D OpenGL function.
extern void (*const glTexSubImage3D)(
    GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*);
/// Pointer to glCopyTexSubImage3D OpenGL function.
extern void (*const glCopyTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
/// Pointer to glCompressedTexImage3D OpenGL function.
extern void (*const glCompressedTexImage3D)(
    GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*);
/// Pointer to glCompressedTexSubImage3D OpenGL function.
extern void (*const glCompressedTexSubImage3D)(
    GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid*);
/// Pointer to glGenQueries OpenGL function.
extern void (*const glGenQueries)(GLsizei, GLuint*);
/// Pointer to glDeleteQueries OpenGL function.
extern void (*const glDeleteQueries)(GLsizei, const GLuint*);
/// Pointer to glIsQuery OpenGL function.
extern GLboolean (*const glIsQuery)(GLuint);
/// Pointer to glBeginQuery OpenGL function.
extern void (*const glBeginQuery)(GLenum, GLuint);
/// Pointer to glEndQuery OpenGL function.
extern void (*const glEndQuery)(GLenum);
/// Pointer to glGetQueryiv OpenGL function.
extern void (*const glGetQueryiv)(GLenum, GLenum, GLint*);
/// Pointer to glGetQueryObjectuiv OpenGL function.
extern void (*const glGetQueryObjectuiv)(GLuint, GLenum, GLuint*);
/// Pointer to glUnmapBuffer OpenGL function.
extern GLboolean (*const glUnmapBuffer)(GLenum);
/// Pointer to glGetBufferPointerv OpenGL function.
extern void (*const glGetBufferPointerv)(GLenum, GLenum, GLvoid**);
/// Pointer to glDrawBuffers OpenGL function.
extern void (*const glDrawBuffers)(GLsizei, const GLenum*);
/// Pointer to glUniformMatrix2x3fv OpenGL function.
extern void (*const glUniformMatrix2x3fv)(GLint, GLsizei, GLboolean, const GLfloat*);
/// Pointer to glUniformMatrix3x2fv OpenGL function.
extern void (*const glUniformMatrix3x2fv)(GLint, GLsizei, GLboolean, const GLfloat*);
/// Pointer to glUniformMatrix2x4fv OpenGL function.
extern void (*const glUniformMatrix2x4fv)(GLint, GLsizei, GLboolean, const GLfloat*);
/// Pointer to glUniformMatrix4x2fv OpenGL function.
extern void (*const glUniformMatrix4x2fv)(GLint, GLsizei, GLboolean, const GLfloat*);
/// Pointer to glUniformMatrix3x4fv OpenGL function.
extern void (*const glUniformMatrix3x4fv)(GLint, GLsizei, GLboolean, const GLfloat*);
/// Pointer to glUniformMatrix4x3fv OpenGL function.
extern void (*const glUniformMatrix4x3fv)(GLint, GLsizei, GLboolean, const GLfloat*);
/// Pointer to glBlitFramebuffer OpenGL function.
extern void (*const glBlitFramebuffer)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
/// Pointer to glRenderbufferStorageMultisample OpenGL function.
extern void (*const glRenderbufferStorageMultisample)(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
/// Pointer to glFramebufferTextureLayer OpenGL function.
extern void (*const glFramebufferTextureLayer)(GLenum, GLenum, GLuint, GLint, GLint);
/// Pointer to glMapBufferRange OpenGL function.
extern GLvoid* (*const glMapBufferRange)(GLenum, GLintptr, GLsizeiptr, GLbitfield);
/// Pointer to glMapBufferRange OpenGL function.
extern void (*const glFlushMappedBufferRange)(GLenum, GLintptr, GLsizeiptr);
/// Pointer to glBindVertexArray OpenGL function.
extern void (*const glBindVertexArray)(GLuint);
/// Pointer to glDeleteVertexArrays OpenGL function.
extern void (*const glDeleteVertexArrays)(GLsizei, const GLuint*);
/// Pointer to glGenVertexArrays OpenGL function.
extern void (*const glGenVertexArrays)(GLsizei, GLuint*);
/// Pointer to glIsVertexArray OpenGL function.
extern GLboolean (*const glIsVertexArray)(GLuint);
/// Pointer to glGetIntegeri_v OpenGL function.
extern void (*const glGetIntegeri_v)(GLenum, GLuint, GLint*);
/// Pointer to glBeginTransformFeedback OpenGL function.
extern void (*const glBeginTransformFeedback)(GLenum);
/// Pointer to glEndTransformFeedback OpenGL function.
extern void (*const glEndTransformFeedback)();
/// Pointer to glBindBufferRange OpenGL function.
extern void (*const glBindBufferRange)(GLenum, GLuint, GLuint, GLintptr, GLsizeiptr);
/// Pointer to glBindBufferBase OpenGL function.
extern void (*const glBindBufferBase)(GLenum, GLuint, GLuint);
/// Pointer to glTransformFeedbackVaryings OpenGL function.
extern void (*const glTransformFeedbackVaryings)(GLuint, GLsizei, const GLchar* const*, GLenum);
/// Pointer to glGetTransformFeedbackVarying OpenGL function.
extern void (*const glGetTransformFeedbackVarying)(GLuint, GLuint, GLsizei, GLsizei*, GLsizei*, GLenum*, GLchar*);
/// Pointer to glVertexAttribIPointer OpenGL function.
extern void (*const glVertexAttribIPointer)(GLuint, GLint, GLenum, GLsizei, const GLvoid*);
/// Pointer to glGetVertexAttribIiv OpenGL function.
extern void (*const glGetVertexAttribIiv)(GLuint, GLenum, GLint*);
/// Pointer to glGetVertexAttribIiv OpenGL function.
extern void (*const glGetVertexAttribIuiv)(GLuint, GLenum, GLuint*);
/// Pointer to glVertexAttribI4i OpenGL function.
extern void (*const glVertexAttribI4i)(GLuint, GLint, GLint, GLint, GLint);
/// Pointer to glVertexAttribI4ui OpenGL function.
extern void (*const glVertexAttribI4ui)(GLuint, GLuint, GLuint, GLuint, GLuint);
/// Pointer to glVertexAttribI4iv OpenGL function.
extern void (*const glVertexAttribI4iv)(GLuint, const GLint*);
/// Pointer to glVertexAttribI4uiv OpenGL function.
extern void (*const glVertexAttribI4uiv)(GLuint, const GLuint*);
/// Pointer to glGetUniformuiv OpenGL function.
extern void (*const glGetUniformuiv)(GLuint, GLint, GLuint*);
/// Pointer to glGetFragDataLocation OpenGL function.
extern GLint (*const glGetFragDataLocation)(GLuint, const GLchar*);
/// Pointer to glUniform1ui OpenGL function.
extern void (*const glUniform1ui)(GLint, GLuint);
/// Pointer to glUniform2ui OpenGL function.
extern void (*const glUniform2ui)(GLint, GLuint, GLuint);
/// Pointer to glUniform3ui OpenGL function.
extern void (*const glUniform3ui)(GLint, GLuint, GLuint, GLuint);
/// Pointer to glUniform4ui OpenGL function.
extern void (*const glUniform4ui)(GLint, GLuint, GLuint, GLuint, GLuint);
/// Pointer to glUniform1uiv OpenGL function.
extern void (*const glUniform1uiv)(GLint, GLsizei, const GLuint*);
/// Pointer to glUniform2uiv OpenGL function.
extern void (*const glUniform2uiv)(GLint, GLsizei, const GLuint*);
/// Pointer to glUniform3uiv OpenGL function.
extern void (*const glUniform3uiv)(GLint, GLsizei, const GLuint*);
/// Pointer to glUniform4uiv OpenGL function.
extern void (*const glUniform4uiv)(GLint, GLsizei, const GLuint*);
/// Pointer to glClearBufferiv OpenGL function.
extern void (*const glClearBufferiv)(GLenum, GLint, const GLint*);
/// Pointer to glClearBufferuiv OpenGL function.
extern void (*const glClearBufferuiv)(GLenum, GLint, const GLuint*);
/// Pointer to glClearBufferfv OpenGL function.
extern void (*const glClearBufferfv)(GLenum, GLint, const GLfloat*);
/// Pointer to glClearBufferfi OpenGL function.
extern void (*const glClearBufferfi)(GLenum, GLint, GLfloat, GLint);
/// Pointer to glGetStringi OpenGL function.
extern const GLubyte* (*const glGetStringi)(GLenum, GLuint);
/// Pointer to glCopyBufferSubData OpenGL function.
extern void (*const glCopyBufferSubData)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr);
/// Pointer to glGetUniformIndices OpenGL function.
extern void (*const glGetUniformIndices)(GLuint, GLsizei, const GLchar* const*, GLuint*);
/// Pointer to glGetActiveUniformsiv OpenGL function.
extern void (*const glGetActiveUniformsiv)(GLuint, GLsizei, const GLuint*, GLenum, GLint*);
/// Pointer to glGetUniformBlockIndex OpenGL function.
extern GLuint (*const glGetUniformBlockIndex)(GLuint, const GLchar*);
/// Pointer to glGetActiveUniformBlockiv OpenGL function.
extern void (*const glGetActiveUniformBlockiv)(GLuint, GLuint, GLenum, GLint*);
/// Pointer to glGetActiveUniformBlockName OpenGL function.
extern void (*const glGetActiveUniformBlockName)(GLuint, GLuint, GLsizei, GLsizei*, GLchar*);
/// Pointer to glUniformBlockBinding OpenGL function.
extern void (*const glUniformBlockBinding)(GLuint, GLuint, GLuint);
/// Pointer to glDrawArraysInstanced OpenGL function.
extern void (*const glDrawArraysInstanced)(GLenum, GLint, GLsizei, GLsizei);
/// Pointer to glDrawElementsInstanced OpenGL function.
extern void (*const glDrawElementsInstanced)(GLenum, GLsizei, GLenum, const GLvoid*, GLsizei);
/// Pointer to glFenceSync OpenGL function.
extern GLsync (*const glFenceSync)(GLenum, GLbitfield);
/// Pointer to glIsSync OpenGL function.
extern GLboolean (*const glIsSync)(GLsync);
/// Pointer to glDeleteSync OpenGL function.
extern void (*const glDeleteSync)(GLsync);
/// Pointer to glClientWaitSync OpenGL function.
extern GLenum (*const glClientWaitSync)(GLsync, GLbitfield, GLuint64);
/// Pointer to glWaitSync OpenGL function.
extern void (*const glWaitSync)(GLsync, GLbitfield, GLuint64);
/// Pointer to glGetInteger64v OpenGL function.
extern void (*const glGetInteger64v)(GLenum, GLint64*);
/// Pointer to glGetSynciv OpenGL function.
extern void (*const glGetSynciv)(GLsync, GLenum, GLsizei, GLsizei*, GLint*);
/// Pointer to glGetInteger64i_v OpenGL function.
extern void (*const glGetInteger64i_v)(GLenum, GLuint, GLint64*);
/// Pointer to glGetBufferParameteri64v OpenGL function.
extern void (*const glGetBufferParameteri64v)(GLenum, GLenum, GLint64*);
/// Pointer to glGenSamplers OpenGL function.
extern void (*const glGenSamplers)(GLsizei, GLuint*);
/// Pointer to glDeleteSamplers OpenGL function.
extern void (*const glDeleteSamplers)(GLsizei, const GLuint*);
/// Pointer to glIsSampler OpenGL function.
extern GLboolean (*const glIsSampler)(GLuint);
/// Pointer to glBindSampler OpenGL function.
extern void (*const glBindSampler)(GLuint, GLuint);
/// Pointer to glSamplerParameteri OpenGL function.
extern void (*const glSamplerParameteri)(GLuint, GLenum, GLint);
/// Pointer to glSamplerParameteriv OpenGL function.
extern void (*const glSamplerParameteriv)(GLuint, GLenum, const GLint*);
/// Pointer to glSamplerParameterf OpenGL function.
extern void (*const glSamplerParameterf)(GLuint, GLenum, GLfloat);
/// Pointer to glSamplerParameterfv OpenGL function.
extern void (*const glSamplerParameterfv)(GLuint, GLenum, const GLfloat*);
/// Pointer to glGetSamplerParameteriv OpenGL function.
extern void (*const glGetSamplerParameteriv)(GLuint, GLenum, GLint*);
/// Pointer to glGetSamplerParameterfv OpenGL function.
extern void (*const glGetSamplerParameterfv)(GLuint, GLenum, GLfloat*);
/// Pointer to glVertexAttribDivisor OpenGL function.
extern void (*const glVertexAttribDivisor)(GLuint, GLuint);
/// Pointer to glBindTransformFeedback OpenGL function.
extern void (*const glBindTransformFeedback)(GLenum, GLuint);
/// Pointer to glDeleteTransformFeedbacks OpenGL function.
extern void (*const glDeleteTransformFeedbacks)(GLsizei, const GLuint*);
/// Pointer to glGenTransformFeedbacks OpenGL function.
extern void (*const glGenTransformFeedbacks)(GLsizei, GLuint*);
/// Pointer to glIsTransformFeedback OpenGL function.
extern GLboolean (*const glIsTransformFeedback)(GLuint);
/// Pointer to glPauseTransformFeedback OpenGL function.
extern void (*const glPauseTransformFeedback)();
/// Pointer to glResumeTransformFeedback OpenGL function.
extern void (*const glResumeTransformFeedback)();
/// Pointer to glGetProgramBinary OpenGL function.
extern void (*const glGetProgramBinary)(GLuint, GLsizei, GLsizei*, GLenum*, GLvoid*);
/// Pointer to glProgramBinary OpenGL function.
extern void (*const glProgramBinary)(GLuint, GLenum, const GLvoid*, GLsizei);
/// Pointer to glProgramParameteri OpenGL function.
extern void (*const glProgramParameteri)(GLuint, GLenum, GLint);
/// Pointer to glInvalidateFramebuffer OpenGL function.
extern void (*const glInvalidateFramebuffer)(GLenum, GLsizei, const GLenum*);
/// Pointer to glInvalidateSubFramebuffer OpenGL function.
extern void (*const glInvalidateSubFramebuffer)(GLenum, GLsizei, const GLenum*, GLint, GLint, GLsizei, GLsizei);
/// Pointer to glTexStorage2D OpenGL function.
extern void (*const glTexStorage2D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
/// Pointer to glTexStorage3D OpenGL function.
extern void (*const glTexStorage3D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei);
/// Pointer to glGetInternalformativ OpenGL function.
extern void (*const glGetInternalformativ)(GLenum, GLenum, GLenum, GLsizei, GLint*);

#ifndef NDEBUG
/// Check for GL errors and print on the console.
void glCheckError(const char* cmd, const char* file, int line);
#endif

} // namespace platform
} // namespace mbgl
