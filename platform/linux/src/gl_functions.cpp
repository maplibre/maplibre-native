#include <mbgl/platform/gl_functions.hpp>

#define GL_GLEXT_PROTOTYPES

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

namespace mbgl {
namespace platform {

/* OpenGL ES 2.0 */

void (*const glActiveTexture)(GLenum) = ::glActiveTexture;
void (*const glAttachShader)(GLuint, GLuint) = ::glAttachShader;
void (*const glBindAttribLocation)(GLuint, GLuint, const GLchar*) = ::glBindAttribLocation;
void (*const glBindBuffer)(GLenum, GLuint) = ::glBindBuffer;
void (*const glBindFramebuffer)(GLenum, GLuint) = ::glBindFramebuffer;
void (*const glBindRenderbuffer)(GLenum, GLuint) = ::glBindRenderbuffer;
void (*const glBindTexture)(GLenum, GLuint) = ::glBindTexture;
void (*const glBlendColor)(GLfloat, GLfloat, GLfloat, GLfloat) = ::glBlendColor;
void (*const glBlendEquation)(GLenum) = ::glBlendEquation;
void (*const glBlendEquationSeparate)(GLenum, GLenum) = ::glBlendEquationSeparate;
void (*const glBlendFunc)(GLenum, GLenum) = ::glBlendFunc;
void (*const glBlendFuncSeparate)(GLenum, GLenum, GLenum, GLenum) = ::glBlendFuncSeparate;
void (*const glBufferData)(GLenum, GLsizeiptr, const void*, GLenum) = ::glBufferData;
void (*const glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const void*) = ::glBufferSubData;
GLenum (*const glCheckFramebufferStatus)(GLenum) = ::glCheckFramebufferStatus;
void (*const glClear)(GLbitfield) = ::glClear;
void (*const glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat) = ::glClearColor;
void (*const glClearDepthf)(GLfloat) = ::glClearDepthf;
void (*const glClearStencil)(GLint) = ::glClearStencil;
void (*const glColorMask)(GLboolean, GLboolean, GLboolean, GLboolean) = ::glColorMask;
void (*const glCompileShader)(GLuint) = ::glCompileShader;
void (*const glCompressedTexImage2D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void*) =
    ::glCompressedTexImage2D;
void (*const glCompressedTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*) =
    ::glCompressedTexSubImage2D;
void (*const glCopyTexImage2D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint) = ::glCopyTexImage2D;
void (*const glCopyTexSubImage2D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) = ::glCopyTexSubImage2D;
GLuint (*const glCreateProgram)() = ::glCreateProgram;
GLuint (*const glCreateShader)(GLenum) = ::glCreateShader;
void (*const glCullFace)(GLenum) = ::glCullFace;
void (*const glDeleteBuffers)(GLsizei, const GLuint*) = ::glDeleteBuffers;
void (*const glDeleteFramebuffers)(GLsizei, const GLuint*) = ::glDeleteFramebuffers;
void (*const glDeleteProgram)(GLuint) = ::glDeleteProgram;
void (*const glDeleteRenderbuffers)(GLsizei, const GLuint*) = ::glDeleteRenderbuffers;
void (*const glDeleteShader)(GLuint) = ::glDeleteShader;
void (*const glDeleteTextures)(GLsizei, const GLuint*) = ::glDeleteTextures;
void (*const glDepthFunc)(GLenum) = ::glDepthFunc;
void (*const glDepthMask)(GLboolean) = ::glDepthMask;
void (*const glDepthRangef)(GLfloat, GLfloat) = ::glDepthRangef;
void (*const glDetachShader)(GLuint, GLuint) = ::glDetachShader;
void (*const glDisable)(GLenum) = ::glDisable;
void (*const glDisableVertexAttribArray)(GLuint) = ::glDisableVertexAttribArray;
void (*const glDrawArrays)(GLenum, GLint, GLsizei) = ::glDrawArrays;
void (*const glDrawElements)(GLenum, GLsizei, GLenum, const void*) = ::glDrawElements;
void (*const glEnable)(GLenum) = ::glEnable;
void (*const glEnableVertexAttribArray)(GLuint) = ::glEnableVertexAttribArray;
void (*const glFinish)() = ::glFinish;
void (*const glFlush)() = ::glFlush;
void (*const glFramebufferRenderbuffer)(GLenum, GLenum, GLenum, GLuint) = ::glFramebufferRenderbuffer;
void (*const glFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint) = ::glFramebufferTexture2D;
void (*const glFrontFace)(GLenum) = ::glFrontFace;
void (*const glGenBuffers)(GLsizei, GLuint*) = ::glGenBuffers;
void (*const glGenerateMipmap)(GLenum) = ::glGenerateMipmap;
void (*const glGenFramebuffers)(GLsizei, GLuint*) = ::glGenFramebuffers;
void (*const glGenRenderbuffers)(GLsizei, GLuint*) = ::glGenRenderbuffers;
void (*const glGenTextures)(GLsizei, GLuint*) = ::glGenTextures;
void (*const glGetActiveAttrib)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*) = ::glGetActiveAttrib;
void (*const glGetActiveUniform)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*) = ::glGetActiveUniform;
void (*const glGetAttachedShaders)(GLuint, GLsizei, GLsizei*, GLuint*) = ::glGetAttachedShaders;
GLint (*const glGetAttribLocation)(GLuint, const GLchar*) = ::glGetAttribLocation;
void (*const glGetBooleanv)(GLenum, GLboolean*) = ::glGetBooleanv;
void (*const glGetBufferParameteriv)(GLenum, GLenum, GLint*) = ::glGetBufferParameteriv;
GLenum (*const glGetError)() = ::glGetError;
void (*const glGetFloatv)(GLenum, GLfloat*) = ::glGetFloatv;
void (*const glGetFramebufferAttachmentParameteriv)(GLenum,
                                                    GLenum,
                                                    GLenum,
                                                    GLint*) = ::glGetFramebufferAttachmentParameteriv;
void (*const glGetIntegerv)(GLenum, GLint*) = ::glGetIntegerv;
void (*const glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*) = ::glGetProgramInfoLog;
void (*const glGetProgramiv)(GLuint, GLenum, GLint*) = ::glGetProgramiv;
void (*const glGetRenderbufferParameteriv)(GLenum, GLenum, GLint*) = ::glGetRenderbufferParameteriv;
void (*const glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*) = ::glGetShaderInfoLog;
void (*const glGetShaderiv)(GLuint, GLenum, GLint*) = ::glGetShaderiv;
void (*const glGetShaderPrecisionFormat)(GLenum, GLenum, GLint*, GLint*) = ::glGetShaderPrecisionFormat;
void (*const glGetShaderSource)(GLuint, GLsizei, GLsizei*, GLchar*) = ::glGetShaderSource;
const GLubyte* (*const glGetString)(GLenum) = ::glGetString;
void (*const glGetTexParameterfv)(GLenum, GLenum, GLfloat*) = ::glGetTexParameterfv;
void (*const glGetTexParameteriv)(GLenum, GLenum, GLint*) = ::glGetTexParameteriv;
void (*const glGetUniformfv)(GLuint, GLint, GLfloat*) = ::glGetUniformfv;
void (*const glGetUniformiv)(GLuint, GLint, GLint*) = ::glGetUniformiv;
GLint (*const glGetUniformLocation)(GLuint, const GLchar*) = ::glGetUniformLocation;
void (*const glGetVertexAttribfv)(GLuint, GLenum, GLfloat*) = ::glGetVertexAttribfv;
void (*const glGetVertexAttribiv)(GLuint, GLenum, GLint*) = ::glGetVertexAttribiv;
void (*const glGetVertexAttribPointerv)(GLuint, GLenum, void**) = ::glGetVertexAttribPointerv;
void (*const glHint)(GLenum, GLenum) = ::glHint;
GLboolean (*const glIsBuffer)(GLuint) = ::glIsBuffer;
GLboolean (*const glIsEnabled)(GLenum) = ::glIsEnabled;
GLboolean (*const glIsFramebuffer)(GLuint) = ::glIsFramebuffer;
GLboolean (*const glIsProgram)(GLuint) = ::glIsProgram;
GLboolean (*const glIsRenderbuffer)(GLuint) = ::glIsRenderbuffer;
GLboolean (*const glIsShader)(GLuint) = ::glIsShader;
GLboolean (*const glIsTexture)(GLuint) = ::glIsTexture;
void (*const glLineWidth)(GLfloat) = ::glLineWidth;
void (*const glLinkProgram)(GLuint) = ::glLinkProgram;
void (*const glPixelStorei)(GLenum, GLint) = ::glPixelStorei;
void (*const glPolygonOffset)(GLfloat, GLfloat) = ::glPolygonOffset;
void (*const glReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*) = ::glReadPixels;
void (*const glReleaseShaderCompiler)() = ::glReleaseShaderCompiler;
void (*const glRenderbufferStorage)(GLenum, GLenum, GLsizei, GLsizei) = ::glRenderbufferStorage;
void (*const glSampleCoverage)(GLfloat, GLboolean) = ::glSampleCoverage;
void (*const glScissor)(GLint, GLint, GLsizei, GLsizei) = ::glScissor;
void (*const glShaderBinary)(GLsizei, const GLuint*, GLenum, const GLvoid*, GLsizei) = ::glShaderBinary;
void (*const glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*) = ::glShaderSource;
void (*const glStencilFunc)(GLenum, GLint, GLuint) = ::glStencilFunc;
void (*const glStencilFuncSeparate)(GLenum, GLenum, GLint, GLuint) = ::glStencilFuncSeparate;
void (*const glStencilMask)(GLuint) = ::glStencilMask;
void (*const glStencilMaskSeparate)(GLenum, GLuint) = ::glStencilMaskSeparate;
void (*const glStencilOp)(GLenum, GLenum, GLenum) = ::glStencilOp;
void (*const glStencilOpSeparate)(GLenum, GLenum, GLenum, GLenum) = ::glStencilOpSeparate;
void (*const glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*) = ::glTexImage2D;
void (*const glTexParameterf)(GLenum, GLenum, GLfloat) = ::glTexParameterf;
void (*const glTexParameterfv)(GLenum, GLenum, const GLfloat*) = ::glTexParameterfv;
void (*const glTexParameteri)(GLenum, GLenum, GLint) = ::glTexParameteri;
void (*const glTexParameteriv)(GLenum, GLenum, const GLint*) = ::glTexParameteriv;
void (*const glTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*) =
    ::glTexSubImage2D;
void (*const glUniform1f)(GLint, GLfloat) = ::glUniform1f;
void (*const glUniform1fv)(GLint, GLsizei, const GLfloat*) = ::glUniform1fv;
void (*const glUniform1i)(GLint, GLint) = ::glUniform1i;
void (*const glUniform1iv)(GLint, GLsizei, const GLint*) = ::glUniform1iv;
void (*const glUniform2f)(GLint, GLfloat, GLfloat) = ::glUniform2f;
void (*const glUniform2fv)(GLint, GLsizei, const GLfloat*) = ::glUniform2fv;
void (*const glUniform2i)(GLint, GLint, GLint) = ::glUniform2i;
void (*const glUniform2iv)(GLint, GLsizei, const GLint*) = ::glUniform2iv;
void (*const glUniform3f)(GLint, GLfloat, GLfloat, GLfloat) = ::glUniform3f;
void (*const glUniform3fv)(GLint, GLsizei, const GLfloat*) = ::glUniform3fv;
void (*const glUniform3i)(GLint, GLint, GLint, GLint) = ::glUniform3i;
void (*const glUniform3iv)(GLint, GLsizei, const GLint*) = ::glUniform3iv;
void (*const glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat) = ::glUniform4f;
void (*const glUniform4fv)(GLint, GLsizei, const GLfloat*) = ::glUniform4fv;
void (*const glUniform4i)(GLint, GLint, GLint, GLint, GLint) = ::glUniform4i;
void (*const glUniform4iv)(GLint, GLsizei, const GLint*) = ::glUniform4iv;
void (*const glUniformMatrix2fv)(GLint, GLsizei, GLboolean, const GLfloat*) = ::glUniformMatrix2fv;
void (*const glUniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat*) = ::glUniformMatrix3fv;
void (*const glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*) = ::glUniformMatrix4fv;
void (*const glUseProgram)(GLuint) = ::glUseProgram;
void (*const glValidateProgram)(GLuint) = ::glValidateProgram;
void (*const glVertexAttrib1f)(GLuint, GLfloat) = ::glVertexAttrib1f;
void (*const glVertexAttrib1fv)(GLuint, const GLfloat*) = ::glVertexAttrib1fv;
void (*const glVertexAttrib2f)(GLuint, GLfloat, GLfloat) = ::glVertexAttrib2f;
void (*const glVertexAttrib2fv)(GLuint, const GLfloat*) = ::glVertexAttrib2fv;
void (*const glVertexAttrib3f)(GLuint, GLfloat, GLfloat, GLfloat) = ::glVertexAttrib3f;
void (*const glVertexAttrib3fv)(GLuint, const GLfloat*) = ::glVertexAttrib3fv;
void (*const glVertexAttrib4f)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat) = ::glVertexAttrib4f;
void (*const glVertexAttrib4fv)(GLuint, const GLfloat*) = ::glVertexAttrib4fv;
void (*const glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*) = ::glVertexAttribPointer;
void (*const glViewport)(GLint, GLint, GLsizei, GLsizei) = ::glViewport;

/* OpenGL ES 3.0 */

void (*const glReadBuffer)(GLenum) = ::glReadBuffer;
void (*const glDrawRangeElements)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid*) = ::glDrawRangeElements;
void (*const glTexImage3D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*) =
    ::glTexImage3D;
void (*const glTexSubImage3D)(
    GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*) = ::glTexSubImage3D;
void (*const glCopyTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) =
    ::glCopyTexSubImage3D;
void (*const glCompressedTexImage3D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*) =
    ::glCompressedTexImage3D;
void (*const glCompressedTexSubImage3D)(
    GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid*) =
    ::glCompressedTexSubImage3D;
void (*const glGenQueries)(GLsizei, GLuint*) = ::glGenQueries;
void (*const glDeleteQueries)(GLsizei, const GLuint*) = ::glDeleteQueries;
GLboolean (*const glIsQuery)(GLuint) = ::glIsQuery;
void (*const glBeginQuery)(GLenum, GLuint) = ::glBeginQuery;
void (*const glEndQuery)(GLenum) = ::glEndQuery;
void (*const glGetQueryiv)(GLenum, GLenum, GLint*) = ::glGetQueryiv;
void (*const glGetQueryObjectuiv)(GLuint, GLenum, GLuint*) = ::glGetQueryObjectuiv;
GLboolean (*const glUnmapBuffer)(GLenum) = ::glUnmapBuffer;
void (*const glGetBufferPointerv)(GLenum, GLenum, GLvoid**) = ::glGetBufferPointerv;
void (*const glDrawBuffers)(GLsizei, const GLenum*) = ::glDrawBuffers;
void (*const glUniformMatrix2x3fv)(GLint, GLsizei, GLboolean, const GLfloat*) = ::glUniformMatrix2x3fv;
void (*const glUniformMatrix3x2fv)(GLint, GLsizei, GLboolean, const GLfloat*) = ::glUniformMatrix3x2fv;
void (*const glUniformMatrix2x4fv)(GLint, GLsizei, GLboolean, const GLfloat*) = ::glUniformMatrix2x4fv;
void (*const glUniformMatrix4x2fv)(GLint, GLsizei, GLboolean, const GLfloat*) = ::glUniformMatrix4x2fv;
void (*const glUniformMatrix3x4fv)(GLint, GLsizei, GLboolean, const GLfloat*) = ::glUniformMatrix3x4fv;
void (*const glUniformMatrix4x3fv)(GLint, GLsizei, GLboolean, const GLfloat*) = ::glUniformMatrix4x3fv;
void (*const glBlitFramebuffer)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum) =
    ::glBlitFramebuffer;
void (*const glRenderbufferStorageMultisample)(GLenum, GLsizei, GLenum, GLsizei, GLsizei) =
    ::glRenderbufferStorageMultisample;
void (*const glFramebufferTextureLayer)(GLenum, GLenum, GLuint, GLint, GLint) = ::glFramebufferTextureLayer;
GLvoid* (*const glMapBufferRange)(GLenum, GLintptr, GLsizeiptr, GLbitfield) = ::glMapBufferRange;
void (*const glFlushMappedBufferRange)(GLenum, GLintptr, GLsizeiptr) = ::glFlushMappedBufferRange;
void (*const glBindVertexArray)(GLuint) = ::glBindVertexArray;
void (*const glDeleteVertexArrays)(GLsizei, const GLuint*) = ::glDeleteVertexArrays;
void (*const glGenVertexArrays)(GLsizei, GLuint*) = ::glGenVertexArrays;
GLboolean (*const glIsVertexArray)(GLuint) = ::glIsVertexArray;
void (*const glGetIntegeri_v)(GLenum, GLuint, GLint*) = ::glGetIntegeri_v;
void (*const glBeginTransformFeedback)(GLenum) = ::glBeginTransformFeedback;
void (*const glEndTransformFeedback)() = ::glEndTransformFeedback;
void (*const glBindBufferRange)(GLenum, GLuint, GLuint, GLintptr, GLsizeiptr) = ::glBindBufferRange;
void (*const glBindBufferBase)(GLenum, GLuint, GLuint) = ::glBindBufferBase;
void (*const glTransformFeedbackVaryings)(GLuint,
                                          GLsizei,
                                          const GLchar* const*,
                                          GLenum) = ::glTransformFeedbackVaryings;
void (*const glGetTransformFeedbackVarying)(GLuint, GLuint, GLsizei, GLsizei*, GLsizei*, GLenum*, GLchar*) =
    ::glGetTransformFeedbackVarying;
void (*const glVertexAttribIPointer)(GLuint, GLint, GLenum, GLsizei, const GLvoid*) = ::glVertexAttribIPointer;
void (*const glGetVertexAttribIiv)(GLuint, GLenum, GLint*) = ::glGetVertexAttribIiv;
void (*const glGetVertexAttribIuiv)(GLuint, GLenum, GLuint*) = ::glGetVertexAttribIuiv;
void (*const glVertexAttribI4i)(GLuint, GLint, GLint, GLint, GLint) = ::glVertexAttribI4i;
void (*const glVertexAttribI4ui)(GLuint, GLuint, GLuint, GLuint, GLuint) = ::glVertexAttribI4ui;
void (*const glVertexAttribI4iv)(GLuint, const GLint*) = ::glVertexAttribI4iv;
void (*const glVertexAttribI4uiv)(GLuint, const GLuint*) = ::glVertexAttribI4uiv;
void (*const glGetUniformuiv)(GLuint, GLint, GLuint*) = ::glGetUniformuiv;
GLint (*const glGetFragDataLocation)(GLuint, const GLchar*) = ::glGetFragDataLocation;
void (*const glUniform1ui)(GLint, GLuint) = ::glUniform1ui;
void (*const glUniform2ui)(GLint, GLuint, GLuint) = ::glUniform2ui;
void (*const glUniform3ui)(GLint, GLuint, GLuint, GLuint) = ::glUniform3ui;
void (*const glUniform4ui)(GLint, GLuint, GLuint, GLuint, GLuint) = ::glUniform4ui;
void (*const glUniform1uiv)(GLint, GLsizei, const GLuint*) = ::glUniform1uiv;
void (*const glUniform2uiv)(GLint, GLsizei, const GLuint*) = ::glUniform2uiv;
void (*const glUniform3uiv)(GLint, GLsizei, const GLuint*) = ::glUniform3uiv;
void (*const glUniform4uiv)(GLint, GLsizei, const GLuint*) = ::glUniform4uiv;
void (*const glClearBufferiv)(GLenum, GLint, const GLint*) = ::glClearBufferiv;
void (*const glClearBufferuiv)(GLenum, GLint, const GLuint*) = ::glClearBufferuiv;
void (*const glClearBufferfv)(GLenum, GLint, const GLfloat*) = ::glClearBufferfv;
void (*const glClearBufferfi)(GLenum, GLint, GLfloat, GLint) = ::glClearBufferfi;
const GLubyte* (*const glGetStringi)(GLenum, GLuint) = ::glGetStringi;
void (*const glCopyBufferSubData)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr) = ::glCopyBufferSubData;
void (*const glGetUniformIndices)(GLuint, GLsizei, const GLchar* const*, GLuint*) = ::glGetUniformIndices;
void (*const glGetActiveUniformsiv)(GLuint, GLsizei, const GLuint*, GLenum, GLint*) = ::glGetActiveUniformsiv;
GLuint (*const glGetUniformBlockIndex)(GLuint, const GLchar*) = ::glGetUniformBlockIndex;
void (*const glGetActiveUniformBlockiv)(GLuint, GLuint, GLenum, GLint*) = ::glGetActiveUniformBlockiv;
void (*const glGetActiveUniformBlockName)(GLuint, GLuint, GLsizei, GLsizei*, GLchar*) = ::glGetActiveUniformBlockName;
void (*const glUniformBlockBinding)(GLuint, GLuint, GLuint) = ::glUniformBlockBinding;
void (*const glDrawArraysInstanced)(GLenum, GLint, GLsizei, GLsizei) = ::glDrawArraysInstanced;
void (*const glDrawElementsInstanced)(GLenum, GLsizei, GLenum, const GLvoid*, GLsizei) = ::glDrawElementsInstanced;
GLsync (*const glFenceSync)(GLenum, GLbitfield) = ::glFenceSync;
GLboolean (*const glIsSync)(GLsync) = ::glIsSync;
void (*const glDeleteSync)(GLsync) = ::glDeleteSync;
GLenum (*const glClientWaitSync)(GLsync, GLbitfield, GLuint64) = ::glClientWaitSync;
void (*const glWaitSync)(GLsync, GLbitfield, GLuint64) = ::glWaitSync;
void (*const glGetInteger64v)(GLenum, GLint64*) = ::glGetInteger64v;
void (*const glGetSynciv)(GLsync, GLenum, GLsizei, GLsizei*, GLint*) = ::glGetSynciv;
void (*const glGetInteger64i_v)(GLenum, GLuint, GLint64*) = ::glGetInteger64i_v;
void (*const glGetBufferParameteri64v)(GLenum, GLenum, GLint64*) = ::glGetBufferParameteri64v;
void (*const glGenSamplers)(GLsizei, GLuint*) = ::glGenSamplers;
void (*const glDeleteSamplers)(GLsizei, const GLuint*) = ::glDeleteSamplers;
GLboolean (*const glIsSampler)(GLuint) = ::glIsSampler;
void (*const glBindSampler)(GLuint, GLuint) = ::glBindSampler;
void (*const glSamplerParameteri)(GLuint, GLenum, GLint) = ::glSamplerParameteri;
void (*const glSamplerParameteriv)(GLuint, GLenum, const GLint*) = ::glSamplerParameteriv;
void (*const glSamplerParameterf)(GLuint, GLenum, GLfloat) = ::glSamplerParameterf;
void (*const glSamplerParameterfv)(GLuint, GLenum, const GLfloat*) = ::glSamplerParameterfv;
void (*const glGetSamplerParameteriv)(GLuint, GLenum, GLint*) = ::glGetSamplerParameteriv;
void (*const glGetSamplerParameterfv)(GLuint, GLenum, GLfloat*) = ::glGetSamplerParameterfv;
void (*const glVertexAttribDivisor)(GLuint, GLuint) = ::glVertexAttribDivisor;
void (*const glBindTransformFeedback)(GLenum, GLuint) = ::glBindTransformFeedback;
void (*const glDeleteTransformFeedbacks)(GLsizei, const GLuint*) = ::glDeleteTransformFeedbacks;
void (*const glGenTransformFeedbacks)(GLsizei, GLuint*) = ::glGenTransformFeedbacks;
GLboolean (*const glIsTransformFeedback)(GLuint) = ::glIsTransformFeedback;
void (*const glPauseTransformFeedback)() = ::glPauseTransformFeedback;
void (*const glResumeTransformFeedback)() = ::glResumeTransformFeedback;
void (*const glGetProgramBinary)(GLuint, GLsizei, GLsizei*, GLenum*, GLvoid*) = ::glGetProgramBinary;
void (*const glProgramBinary)(GLuint, GLenum, const GLvoid*, GLsizei) = ::glProgramBinary;
void (*const glProgramParameteri)(GLuint, GLenum, GLint) = ::glProgramParameteri;
void (*const glInvalidateFramebuffer)(GLenum, GLsizei, const GLenum*) = ::glInvalidateFramebuffer;
void (*const glInvalidateSubFramebuffer)(GLenum, GLsizei, const GLenum*, GLint, GLint, GLsizei, GLsizei) =
    ::glInvalidateSubFramebuffer;
void (*const glTexStorage2D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei) = ::glTexStorage2D;
void (*const glTexStorage3D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei) = ::glTexStorage3D;
void (*const glGetInternalformativ)(GLenum, GLenum, GLenum, GLsizei, GLint*) = ::glGetInternalformativ;

} // namespace platform
} // namespace mbgl
