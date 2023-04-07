#pragma once

#include <mbgl/platform/gl_functions.hpp>

#ifndef _gl_functions_wgl_
#define _gl_functions_wgl_ 1

#ifndef WINAPI
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#endif
#endif

#define GL_GLEXT_PROTOTYPES

#ifdef MBGL_USE_GLES2
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <GL/wgl.h>

#ifdef __cplusplus
extern "C" {
#endif

PFNGLACTIVETEXTUREPROC wgl_glActiveTexture = NULL;
PFNGLATTACHSHADERPROC wgl_glAttachShader = NULL;
PFNGLBINDATTRIBLOCATIONPROC wgl_glBindAttribLocation = NULL;
PFNGLBINDBUFFERPROC wgl_glBindBuffer = NULL;
PFNGLBINDFRAMEBUFFERPROC wgl_glBindFramebuffer = NULL;
PFNGLBINDRENDERBUFFERPROC wgl_glBindRenderbuffer = NULL;
PFNGLBINDTEXTUREPROC wgl_glBindTexture = NULL;
PFNGLBLENDCOLORPROC wgl_glBlendColor = NULL;
PFNGLBLENDEQUATIONPROC wgl_glBlendEquation = NULL;
PFNGLBLENDEQUATIONSEPARATEPROC wgl_glBlendEquationSeparate = NULL;
PFNGLBLENDFUNCPROC wgl_glBlendFunc = NULL;
PFNGLBLENDFUNCSEPARATEPROC wgl_glBlendFuncSeparate = NULL;
PFNGLBUFFERDATAPROC wgl_glBufferData = NULL;
PFNGLBUFFERSUBDATAPROC wgl_glBufferSubData = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC wgl_glCheckFramebufferStatus = NULL;
PFNGLCLEARPROC wgl_glClear = NULL;
PFNGLCLEARCOLORPROC wgl_glClearColor = NULL;
PFNGLCLEARDEPTHFPROC wgl_glClearDepthf = NULL;
PFNGLCLEARSTENCILPROC wgl_glClearStencil = NULL;
PFNGLCOLORMASKPROC wgl_glColorMask = NULL;
PFNGLCOMPILESHADERPROC wgl_glCompileShader = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DPROC wgl_glCompressedTexImage2D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC wgl_glCompressedTexSubImage2D = NULL;
PFNGLCOPYTEXIMAGE2DPROC wgl_glCopyTexImage2D = NULL;
PFNGLCOPYTEXSUBIMAGE2DPROC wgl_glCopyTexSubImage2D = NULL;
PFNGLCREATEPROGRAMPROC wgl_glCreateProgram = NULL;
PFNGLCREATESHADERPROC wgl_glCreateShader = NULL;
PFNGLCULLFACEPROC wgl_glCullFace = NULL;
PFNGLDELETEBUFFERSPROC wgl_glDeleteBuffers = NULL;
PFNGLDELETEFRAMEBUFFERSPROC wgl_glDeleteFramebuffers = NULL;
PFNGLDELETEPROGRAMPROC wgl_glDeleteProgram = NULL;
PFNGLDELETERENDERBUFFERSPROC wgl_glDeleteRenderbuffers = NULL;
PFNGLDELETESHADERPROC wgl_glDeleteShader = NULL;
PFNGLDELETETEXTURESPROC wgl_glDeleteTextures = NULL;
PFNGLDEPTHFUNCPROC wgl_glDepthFunc = NULL;
PFNGLDEPTHMASKPROC wgl_glDepthMask = NULL;
PFNGLDEPTHRANGEFPROC wgl_glDepthRangef = NULL;
PFNGLDETACHSHADERPROC wgl_glDetachShader = NULL;
PFNGLDISABLEPROC wgl_glDisable = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC wgl_glDisableVertexAttribArray = NULL;
PFNGLDRAWARRAYSPROC wgl_glDrawArrays = NULL;
PFNGLDRAWELEMENTSPROC wgl_glDrawElements = NULL;
PFNGLENABLEPROC wgl_glEnable = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC wgl_glEnableVertexAttribArray = NULL;
PFNGLFINISHPROC wgl_glFinish = NULL;
PFNGLFLUSHPROC wgl_glFlush = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC wgl_glFramebufferRenderbuffer = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC wgl_glFramebufferTexture2D = NULL;
PFNGLFRONTFACEPROC wgl_glFrontFace = NULL;
PFNGLGENBUFFERSPROC wgl_glGenBuffers = NULL;
PFNGLGENERATEMIPMAPPROC wgl_glGenerateMipmap = NULL;
PFNGLGENFRAMEBUFFERSPROC wgl_glGenFramebuffers = NULL;
PFNGLGENRENDERBUFFERSPROC wgl_glGenRenderbuffers = NULL;
PFNGLGENTEXTURESPROC wgl_glGenTextures = NULL;
PFNGLGETACTIVEATTRIBPROC wgl_glGetActiveAttrib = NULL;
PFNGLGETACTIVEUNIFORMPROC wgl_glGetActiveUniform = NULL;
PFNGLGETATTACHEDSHADERSPROC wgl_glGetAttachedShaders = NULL;
PFNGLGETATTRIBLOCATIONPROC wgl_glGetAttribLocation = NULL;
PFNGLGETBOOLEANVPROC wgl_glGetBooleanv = NULL;
PFNGLGETBUFFERPARAMETERIVPROC wgl_glGetBufferParameteriv = NULL;
PFNGLGETERRORPROC wgl_glGetError = NULL;
PFNGLGETFLOATVPROC wgl_glGetFloatv = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC wgl_glGetFramebufferAttachmentParameteriv = NULL;
PFNGLGETINTEGERVPROC wgl_glGetIntegerv = NULL;
PFNGLGETPROGRAMINFOLOGPROC wgl_glGetProgramInfoLog = NULL;
PFNGLGETPROGRAMIVPROC wgl_glGetProgramiv = NULL;
PFNGLGETRENDERBUFFERPARAMETERIVPROC wgl_glGetRenderbufferParameteriv = NULL;
PFNGLGETSHADERINFOLOGPROC wgl_glGetShaderInfoLog = NULL;
PFNGLGETSHADERIVPROC wgl_glGetShaderiv = NULL;
PFNGLGETSHADERSOURCEPROC wgl_glGetShaderSource = NULL;
PFNGLGETSTRINGPROC wgl_glGetString = NULL;
PFNGLGETTEXPARAMETERFVPROC wgl_glGetTexParameterfv = NULL;
PFNGLGETTEXPARAMETERIVPROC wgl_glGetTexParameteriv = NULL;
PFNGLGETUNIFORMFVPROC wgl_glGetUniformfv = NULL;
PFNGLGETUNIFORMIVPROC wgl_glGetUniformiv = NULL;
PFNGLGETUNIFORMLOCATIONPROC wgl_glGetUniformLocation = NULL;
PFNGLGETVERTEXATTRIBFVPROC wgl_glGetVertexAttribfv = NULL;
PFNGLGETVERTEXATTRIBIVPROC wgl_glGetVertexAttribiv = NULL;
PFNGLGETVERTEXATTRIBPOINTERVPROC wgl_glGetVertexAttribPointerv = NULL;
PFNGLHINTPROC wgl_glHint = NULL;
PFNGLISBUFFERPROC wgl_glIsBuffer = NULL;
PFNGLISENABLEDPROC wgl_glIsEnabled = NULL;
PFNGLISFRAMEBUFFERPROC wgl_glIsFramebuffer = NULL;
PFNGLISPROGRAMPROC wgl_glIsProgram = NULL;
PFNGLISRENDERBUFFERPROC wgl_glIsRenderbuffer = NULL;
PFNGLISSHADERPROC wgl_glIsShader = NULL;
PFNGLISTEXTUREPROC wgl_glIsTexture = NULL;
PFNGLLINEWIDTHPROC wgl_glLineWidth = NULL;
PFNGLLINKPROGRAMPROC wgl_glLinkProgram = NULL;
PFNGLPIXELSTOREIPROC wgl_glPixelStorei = NULL;
PFNGLPOLYGONOFFSETPROC wgl_glPolygonOffset = NULL;
PFNGLREADPIXELSPROC wgl_glReadPixels = NULL;
PFNGLRENDERBUFFERSTORAGEPROC wgl_glRenderbufferStorage = NULL;
PFNGLSAMPLECOVERAGEPROC wgl_glSampleCoverage = NULL;
PFNGLSCISSORPROC wgl_glScissor = NULL;
PFNGLSHADERSOURCEPROC wgl_glShaderSource = NULL;
PFNGLSTENCILFUNCPROC wgl_glStencilFunc = NULL;
PFNGLSTENCILFUNCSEPARATEPROC wgl_glStencilFuncSeparate = NULL;
PFNGLSTENCILMASKPROC wgl_glStencilMask = NULL;
PFNGLSTENCILMASKSEPARATEPROC wgl_glStencilMaskSeparate = NULL;
PFNGLSTENCILOPPROC wgl_glStencilOp = NULL;
PFNGLSTENCILOPSEPARATEPROC wgl_glStencilOpSeparate = NULL;
PFNGLTEXIMAGE2DPROC wgl_glTexImage2D = NULL;
PFNGLTEXPARAMETERFPROC wgl_glTexParameterf = NULL;
PFNGLTEXPARAMETERFVPROC wgl_glTexParameterfv = NULL;
PFNGLTEXPARAMETERIPROC wgl_glTexParameteri = NULL;
PFNGLTEXPARAMETERIVPROC wgl_glTexParameteriv = NULL;
PFNGLTEXSUBIMAGE2DPROC wgl_glTexSubImage2D = NULL;
PFNGLUNIFORM1FPROC wgl_glUniform1f = NULL;
PFNGLUNIFORM1FVPROC wgl_glUniform1fv = NULL;
PFNGLUNIFORM1IPROC wgl_glUniform1i = NULL;
PFNGLUNIFORM1IVPROC wgl_glUniform1iv = NULL;
PFNGLUNIFORM2FPROC wgl_glUniform2f = NULL;
PFNGLUNIFORM2FVPROC wgl_glUniform2fv = NULL;
PFNGLUNIFORM2IPROC wgl_glUniform2i = NULL;
PFNGLUNIFORM2IVPROC wgl_glUniform2iv = NULL;
PFNGLUNIFORM3FPROC wgl_glUniform3f = NULL;
PFNGLUNIFORM3FVPROC wgl_glUniform3fv = NULL;
PFNGLUNIFORM3IPROC wgl_glUniform3i = NULL;
PFNGLUNIFORM3IVPROC wgl_glUniform3iv = NULL;
PFNGLUNIFORM4FPROC wgl_glUniform4f = NULL;
PFNGLUNIFORM4FVPROC wgl_glUniform4fv = NULL;
PFNGLUNIFORM4IPROC wgl_glUniform4i = NULL;
PFNGLUNIFORM4IVPROC wgl_glUniform4iv = NULL;
PFNGLUNIFORMMATRIX2FVPROC wgl_glUniformMatrix2fv = NULL;
PFNGLUNIFORMMATRIX3FVPROC wgl_glUniformMatrix3fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC wgl_glUniformMatrix4fv = NULL;
PFNGLUSEPROGRAMPROC wgl_glUseProgram = NULL;
PFNGLVALIDATEPROGRAMPROC wgl_glValidateProgram = NULL;
PFNGLVERTEXATTRIB1FPROC wgl_glVertexAttrib1f = NULL;
PFNGLVERTEXATTRIB1FVPROC wgl_glVertexAttrib1fv = NULL;
PFNGLVERTEXATTRIB2FPROC wgl_glVertexAttrib2f = NULL;
PFNGLVERTEXATTRIB2FVPROC wgl_glVertexAttrib2fv = NULL;
PFNGLVERTEXATTRIB3FPROC wgl_glVertexAttrib3f = NULL;
PFNGLVERTEXATTRIB3FVPROC wgl_glVertexAttrib3fv = NULL;
PFNGLVERTEXATTRIB4FPROC wgl_glVertexAttrib4f = NULL;
PFNGLVERTEXATTRIB4FVPROC wgl_glVertexAttrib4fv = NULL;
PFNGLVERTEXATTRIBPOINTERPROC wgl_glVertexAttribPointer = NULL;
PFNGLVIEWPORTPROC wgl_glViewport = NULL;

PFNWGLCHOOSEPIXELFORMATARBPROC wgl_wglChoosePixelFormatARB = NULL;
PFNWGLCREATECONTEXTATTRIBSARBPROC wgl_wglCreateContextAttribsARB = NULL;
PFNWGLGETEXTENSIONSSTRINGARBPROC wgl_wglGetExtensionsStringARB = NULL;
PFNWGLGETEXTENSIONSSTRINGEXTPROC wgl_wglGetExtensionsStringEXT = NULL;

HMODULE opengl32 = NULL;

typedef PROC (WINAPI * PFNWGLLOADERWGLGETPROCADDRESSPROC) (LPCSTR lpszProc);
PFNWGLLOADERWGLGETPROCADDRESSPROC wgl_wglGetProcAddress = NULL;

PROC WINAPI wgl_GetProcAddress(LPCSTR procName) {
    PROC proc = NULL;
	proc = wgl_wglGetProcAddress(procName);

    if (!proc) {
        proc = GetProcAddress(opengl32, procName);
    }

    return proc;
}

void loadWGL() {
	if(opengl32) {
		return;
	}

	opengl32 = LoadLibraryA("opengl32.dll");
	wgl_wglGetProcAddress = (PFNWGLLOADERWGLGETPROCADDRESSPROC)GetProcAddress(opengl32, "wglGetProcAddress");

	wgl_glActiveTexture = (PFNGLACTIVETEXTUREPROC)wgl_GetProcAddress("glActiveTexture");
	wgl_glAttachShader = (PFNGLATTACHSHADERPROC)wgl_GetProcAddress("glAttachShader");
	wgl_glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)wgl_GetProcAddress("glBindAttribLocation");
	wgl_glBindBuffer = (PFNGLBINDBUFFERPROC)wgl_GetProcAddress("glBindBuffer");
	wgl_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wgl_GetProcAddress("glBindFramebuffer");
	wgl_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)wgl_GetProcAddress("glBindRenderbuffer");
	wgl_glBindTexture = (PFNGLBINDTEXTUREPROC)wgl_GetProcAddress("glBindTexture");
	wgl_glBlendColor = (PFNGLBLENDCOLORPROC)wgl_GetProcAddress("glBlendColor");
	wgl_glBlendEquation = (PFNGLBLENDEQUATIONPROC)wgl_GetProcAddress("glBlendEquation");
	wgl_glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC)wgl_GetProcAddress("glBlendEquationSeparate");
	wgl_glBlendFunc = (PFNGLBLENDFUNCPROC)wgl_GetProcAddress("glBlendFunc");
	wgl_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)wgl_GetProcAddress("glBlendFuncSeparate");
	wgl_glBufferData = (PFNGLBUFFERDATAPROC)wgl_GetProcAddress("glBufferData");
	wgl_glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wgl_GetProcAddress("glBufferSubData");
	wgl_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wgl_GetProcAddress("glCheckFramebufferStatus");
	wgl_glClear = (PFNGLCLEARPROC)wgl_GetProcAddress("glClear");
	wgl_glClearColor = (PFNGLCLEARCOLORPROC)wgl_GetProcAddress("glClearColor");
	wgl_glClearDepthf = (PFNGLCLEARDEPTHFPROC)wgl_GetProcAddress("glClearDepthf");
	wgl_glClearStencil = (PFNGLCLEARSTENCILPROC)wgl_GetProcAddress("glClearStencil");
	wgl_glColorMask = (PFNGLCOLORMASKPROC)wgl_GetProcAddress("glColorMask");
	wgl_glCompileShader = (PFNGLCOMPILESHADERPROC)wgl_GetProcAddress("glCompileShader");
	wgl_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)wgl_GetProcAddress("glCompressedTexImage2D");
	wgl_glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)wgl_GetProcAddress("glCompressedTexSubImage2D");
	wgl_glCopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC)wgl_GetProcAddress("glCopyTexImage2D");
	wgl_glCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC)wgl_GetProcAddress("glCopyTexSubImage2D");
	wgl_glCreateProgram = (PFNGLCREATEPROGRAMPROC)wgl_GetProcAddress("glCreateProgram");
	wgl_glCreateShader = (PFNGLCREATESHADERPROC)wgl_GetProcAddress("glCreateShader");
	wgl_glCullFace = (PFNGLCULLFACEPROC)wgl_GetProcAddress("glCullFace");
	wgl_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wgl_GetProcAddress("glDeleteBuffers");
	wgl_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wgl_GetProcAddress("glDeleteFramebuffers");
	wgl_glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wgl_GetProcAddress("glDeleteProgram");
	wgl_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)wgl_GetProcAddress("glDeleteRenderbuffers");
	wgl_glDeleteShader = (PFNGLDELETESHADERPROC)wgl_GetProcAddress("glDeleteShader");
	wgl_glDeleteTextures = (PFNGLDELETETEXTURESPROC)wgl_GetProcAddress("glDeleteTextures");
	wgl_glDepthFunc = (PFNGLDEPTHFUNCPROC)wgl_GetProcAddress("glDepthFunc");
	wgl_glDepthMask = (PFNGLDEPTHMASKPROC)wgl_GetProcAddress("glDepthMask");
	wgl_glDepthRangef = (PFNGLDEPTHRANGEFPROC)wgl_GetProcAddress("glDepthRangef");
	wgl_glDetachShader = (PFNGLDETACHSHADERPROC)wgl_GetProcAddress("glDetachShader");
	wgl_glDisable = (PFNGLDISABLEPROC)wgl_GetProcAddress("glDisable");
	wgl_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wgl_GetProcAddress("glDisableVertexAttribArray");
	wgl_glDrawArrays = (PFNGLDRAWARRAYSPROC)wgl_GetProcAddress("glDrawArrays");
	wgl_glDrawElements = (PFNGLDRAWELEMENTSPROC)wgl_GetProcAddress("glDrawElements");
	wgl_glEnable = (PFNGLENABLEPROC)wgl_GetProcAddress("glEnable");
	wgl_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wgl_GetProcAddress("glEnableVertexAttribArray");
	wgl_glFinish = (PFNGLFINISHPROC)wgl_GetProcAddress("glFinish");
	wgl_glFlush = (PFNGLFLUSHPROC)wgl_GetProcAddress("glFlush");
	wgl_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wgl_GetProcAddress("glFramebufferRenderbuffer");
	wgl_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wgl_GetProcAddress("glFramebufferTexture2D");
	wgl_glFrontFace = (PFNGLFRONTFACEPROC)wgl_GetProcAddress("glFrontFace");
	wgl_glGenBuffers = (PFNGLGENBUFFERSPROC)wgl_GetProcAddress("glGenBuffers");
	wgl_glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wgl_GetProcAddress("glGenerateMipmap");
	wgl_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wgl_GetProcAddress("glGenFramebuffers");
	wgl_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)wgl_GetProcAddress("glGenRenderbuffers");
	wgl_glGenTextures = (PFNGLGENTEXTURESPROC)wgl_GetProcAddress("glGenTextures");
	wgl_glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)wgl_GetProcAddress("glGetActiveAttrib");
	wgl_glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)wgl_GetProcAddress("glGetActiveUniform");
	wgl_glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)wgl_GetProcAddress("glGetAttachedShaders");
	wgl_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wgl_GetProcAddress("glGetAttribLocation");
	wgl_glGetBooleanv = (PFNGLGETBOOLEANVPROC)wgl_GetProcAddress("glGetBooleanv");
	wgl_glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)wgl_GetProcAddress("glGetBufferParameteriv");
	wgl_glGetError = (PFNGLGETERRORPROC)wgl_GetProcAddress("glGetError");
	wgl_glGetFloatv = (PFNGLGETFLOATVPROC)wgl_GetProcAddress("glGetFloatv");
	wgl_glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)wgl_GetProcAddress("glGetFramebufferAttachmentParameteriv");
	wgl_glGetIntegerv = (PFNGLGETINTEGERVPROC)wgl_GetProcAddress("glGetIntegerv");
	wgl_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wgl_GetProcAddress("glGetProgramInfoLog");
	wgl_glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wgl_GetProcAddress("glGetProgramiv");
	wgl_glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)wgl_GetProcAddress("glGetRenderbufferParameteriv");
	wgl_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wgl_GetProcAddress("glGetShaderInfoLog");
	wgl_glGetShaderiv = (PFNGLGETSHADERIVPROC)wgl_GetProcAddress("glGetShaderiv");
	wgl_glGetShaderSource = (PFNGLGETSHADERSOURCEPROC)wgl_GetProcAddress("glGetShaderSource");
	wgl_glGetString = (PFNGLGETSTRINGPROC)wgl_GetProcAddress("glGetString");
	wgl_glGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC)wgl_GetProcAddress("glGetTexParameterfv");
	wgl_glGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC)wgl_GetProcAddress("glGetTexParameteriv");
	wgl_glGetUniformfv = (PFNGLGETUNIFORMFVPROC)wgl_GetProcAddress("glGetUniformfv");
	wgl_glGetUniformiv = (PFNGLGETUNIFORMIVPROC)wgl_GetProcAddress("glGetUniformiv");
	wgl_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wgl_GetProcAddress("glGetUniformLocation");
	wgl_glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC)wgl_GetProcAddress("glGetVertexAttribfv");
	wgl_glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)wgl_GetProcAddress("glGetVertexAttribiv");
	wgl_glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC)wgl_GetProcAddress("glGetVertexAttribPointerv");
	wgl_glHint = (PFNGLHINTPROC)wgl_GetProcAddress("glHint");
	wgl_glIsBuffer = (PFNGLISBUFFERPROC)wgl_GetProcAddress("glIsBuffer");
	wgl_glIsEnabled = (PFNGLISENABLEDPROC)wgl_GetProcAddress("glIsEnabled");
	wgl_glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)wgl_GetProcAddress("glIsFramebuffer");
	wgl_glIsProgram = (PFNGLISPROGRAMPROC)wgl_GetProcAddress("glIsProgram");
	wgl_glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)wgl_GetProcAddress("glIsRenderbuffer");
	wgl_glIsShader = (PFNGLISSHADERPROC)wgl_GetProcAddress("glIsShader");
	wgl_glIsTexture = (PFNGLISTEXTUREPROC)wgl_GetProcAddress("glIsTexture");
	wgl_glLineWidth = (PFNGLLINEWIDTHPROC)wgl_GetProcAddress("glLineWidth");
	wgl_glLinkProgram = (PFNGLLINKPROGRAMPROC)wgl_GetProcAddress("glLinkProgram");
	wgl_glPixelStorei = (PFNGLPIXELSTOREIPROC)wgl_GetProcAddress("glPixelStorei");
	wgl_glPolygonOffset = (PFNGLPOLYGONOFFSETPROC)wgl_GetProcAddress("glPolygonOffset");
	wgl_glReadPixels = (PFNGLREADPIXELSPROC)wgl_GetProcAddress("glReadPixels");
	wgl_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)wgl_GetProcAddress("glRenderbufferStorage");
	wgl_glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC)wgl_GetProcAddress("glSampleCoverage");
	wgl_glScissor = (PFNGLSCISSORPROC)wgl_GetProcAddress("glScissor");
	wgl_glShaderSource = (PFNGLSHADERSOURCEPROC)wgl_GetProcAddress("glShaderSource");
	wgl_glStencilFunc = (PFNGLSTENCILFUNCPROC)wgl_GetProcAddress("glStencilFunc");
	wgl_glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC)wgl_GetProcAddress("glStencilFuncSeparate");
	wgl_glStencilMask = (PFNGLSTENCILMASKPROC)wgl_GetProcAddress("glStencilMask");
	wgl_glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC)wgl_GetProcAddress("glStencilMaskSeparate");
	wgl_glStencilOp = (PFNGLSTENCILOPPROC)wgl_GetProcAddress("glStencilOp");
	wgl_glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC)wgl_GetProcAddress("glStencilOpSeparate");
	wgl_glTexImage2D = (PFNGLTEXIMAGE2DPROC)wgl_GetProcAddress("glTexImage2D");
	wgl_glTexParameterf = (PFNGLTEXPARAMETERFPROC)wgl_GetProcAddress("glTexParameterf");
	wgl_glTexParameterfv = (PFNGLTEXPARAMETERFVPROC)wgl_GetProcAddress("glTexParameterfv");
	wgl_glTexParameteri = (PFNGLTEXPARAMETERIPROC)wgl_GetProcAddress("glTexParameteri");
	wgl_glTexParameteriv = (PFNGLTEXPARAMETERIVPROC)wgl_GetProcAddress("glTexParameteriv");
	wgl_glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC)wgl_GetProcAddress("glTexSubImage2D");
	wgl_glUniform1f = (PFNGLUNIFORM1FPROC)wgl_GetProcAddress("glUniform1f");
	wgl_glUniform1fv = (PFNGLUNIFORM1FVPROC)wgl_GetProcAddress("glUniform1fv");
	wgl_glUniform1i = (PFNGLUNIFORM1IPROC)wgl_GetProcAddress("glUniform1i");
	wgl_glUniform1iv = (PFNGLUNIFORM1IVPROC)wgl_GetProcAddress("glUniform1iv");
	wgl_glUniform2f = (PFNGLUNIFORM2FPROC)wgl_GetProcAddress("glUniform2f");
	wgl_glUniform2fv = (PFNGLUNIFORM2FVPROC)wgl_GetProcAddress("glUniform2fv");
	wgl_glUniform2i = (PFNGLUNIFORM2IPROC)wgl_GetProcAddress("glUniform2i");
	wgl_glUniform2iv = (PFNGLUNIFORM2IVPROC)wgl_GetProcAddress("glUniform2iv");
	wgl_glUniform3f = (PFNGLUNIFORM3FPROC)wgl_GetProcAddress("glUniform3f");
	wgl_glUniform3fv = (PFNGLUNIFORM3FVPROC)wgl_GetProcAddress("glUniform3fv");
	wgl_glUniform3i = (PFNGLUNIFORM3IPROC)wgl_GetProcAddress("glUniform3i");
	wgl_glUniform3iv = (PFNGLUNIFORM3IVPROC)wgl_GetProcAddress("glUniform3iv");
	wgl_glUniform4f = (PFNGLUNIFORM4FPROC)wgl_GetProcAddress("glUniform4f");
	wgl_glUniform4fv = (PFNGLUNIFORM4FVPROC)wgl_GetProcAddress("glUniform4fv");
	wgl_glUniform4i = (PFNGLUNIFORM4IPROC)wgl_GetProcAddress("glUniform4i");
	wgl_glUniform4iv = (PFNGLUNIFORM4IVPROC)wgl_GetProcAddress("glUniform4iv");
	wgl_glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)wgl_GetProcAddress("glUniformMatrix2fv");
	wgl_glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)wgl_GetProcAddress("glUniformMatrix3fv");
	wgl_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wgl_GetProcAddress("glUniformMatrix4fv");
	wgl_glUseProgram = (PFNGLUSEPROGRAMPROC)wgl_GetProcAddress("glUseProgram");
	wgl_glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)wgl_GetProcAddress("glValidateProgram");
	wgl_glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)wgl_GetProcAddress("glVertexAttrib1f");
	wgl_glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)wgl_GetProcAddress("glVertexAttrib1fv");
	wgl_glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC)wgl_GetProcAddress("glVertexAttrib2f");
	wgl_glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)wgl_GetProcAddress("glVertexAttrib2fv");
	wgl_glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC)wgl_GetProcAddress("glVertexAttrib3f");
	wgl_glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)wgl_GetProcAddress("glVertexAttrib3fv");
	wgl_glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)wgl_GetProcAddress("glVertexAttrib4f");
	wgl_glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)wgl_GetProcAddress("glVertexAttrib4fv");
	wgl_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wgl_GetProcAddress("glVertexAttribPointer");
	wgl_glViewport = (PFNGLVIEWPORTPROC)wgl_GetProcAddress("glViewport");

	wgl_wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wgl_GetProcAddress("wglChoosePixelFormatARB");
	wgl_wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wgl_GetProcAddress("wglCreateContextAttribsARB");
	wgl_wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wgl_GetProcAddress("wglGetExtensionsStringARB");
	wgl_wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wgl_GetProcAddress("wglGetExtensionsStringEXT");
}

#ifdef __cplusplus
}
#endif

namespace mbgl {
namespace platform {

void (* const glActiveTexture)(GLenum) = [](GLenum texture) { if (!opengl32) loadWGL(); ::wgl_glActiveTexture(texture); };
void (* const glAttachShader)(GLuint, GLuint) = [](GLuint program, GLuint shader) { if (!opengl32) loadWGL(); ::wgl_glAttachShader(program, shader); };
void (* const glBindAttribLocation)(GLuint, GLuint, const GLchar*) = [](GLuint program, GLuint index, const GLchar* name) { if (!opengl32) loadWGL(); ::wgl_glBindAttribLocation(program, index, name); };
void (* const glBindBuffer)(GLenum, GLuint) = [](GLenum target, GLuint buffer) { if (!opengl32) loadWGL(); ::wgl_glBindBuffer(target, buffer); };
void (* const glBindFramebuffer)(GLenum, GLuint) = [](GLenum target, GLuint framebuffer) { if (!opengl32) loadWGL(); ::wgl_glBindFramebuffer(target, framebuffer); };
void (* const glBindRenderbuffer)(GLenum, GLuint) = [](GLenum target, GLuint renderbuffer) { if (!opengl32) loadWGL(); ::wgl_glBindRenderbuffer(target, renderbuffer); };
void (* const glBindTexture)(GLenum, GLuint) = [](GLenum target, GLuint texture) { if (!opengl32) loadWGL(); ::wgl_glBindTexture(target, texture); };
void (* const glBlendColor)(GLfloat, GLfloat, GLfloat, GLfloat) = [](GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) { if (!opengl32) loadWGL(); ::wgl_glBlendColor(red, green, blue, alpha); };
void (* const glBlendEquation)(GLenum) = [](GLenum mode) { if (!opengl32) loadWGL(); ::wgl_glBlendEquation(mode); };
void (* const glBlendEquationSeparate)(GLenum, GLenum) = [](GLenum modeRGB, GLenum modeAlpha) { if (!opengl32) loadWGL(); ::wgl_glBlendEquationSeparate(modeRGB, modeAlpha); };
void (* const glBlendFunc)(GLenum, GLenum) = [](GLenum sfactor, GLenum dfactor) { if (!opengl32) loadWGL(); ::wgl_glBlendFunc(sfactor, dfactor); };
void (* const glBlendFuncSeparate)(GLenum, GLenum, GLenum, GLenum) = [](GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) { if (!opengl32) loadWGL(); ::wgl_glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha); };
void (* const glBufferData)(GLenum, GLsizeiptr, const void*, GLenum) = [](GLenum target, GLsizeiptr size, const void* data, GLenum usage) { if (!opengl32) loadWGL(); ::wgl_glBufferData(target, size, data, usage); };
void (* const glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const void*) = [](GLenum target, GLintptr offset, GLsizeiptr size, const void* data) { if (!opengl32) loadWGL(); ::wgl_glBufferSubData(target, offset, size, data); };
GLenum(* const glCheckFramebufferStatus)(GLenum) = [](GLenum target) { if (!opengl32) loadWGL(); return ::wgl_glCheckFramebufferStatus(target); };
void (* const glClear)(GLbitfield) = [](GLbitfield mask) { if (!opengl32) loadWGL(); ::wgl_glClear(mask); };
void (* const glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat) = [](GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) { if (!opengl32) loadWGL(); ::wgl_glClearColor(red, green, blue, alpha); };
void (* const glClearDepthf)(GLfloat) = [](GLfloat d) { if (!opengl32) loadWGL(); ::wgl_glClearDepthf(d); };
void (* const glClearStencil)(GLint) = [](GLint s) { if (!opengl32) loadWGL(); ::wgl_glClearStencil(s); };
void (* const glColorMask)(GLboolean, GLboolean, GLboolean, GLboolean) = [](GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) { if (!opengl32) loadWGL(); ::wgl_glColorMask(red, green, blue, alpha); };
void (* const glCompileShader)(GLuint) = [](GLuint shader) { if (!opengl32) loadWGL(); ::wgl_glCompileShader(shader); };
void (* const glCompressedTexImage2D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void*) = [](GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data) { if (!opengl32) loadWGL(); ::wgl_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data); };
void (* const glCompressedTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void*) = [](GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data) { if (!opengl32) loadWGL(); ::wgl_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data); };
void (* const glCopyTexImage2D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint) = [](GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) { if (!opengl32) loadWGL(); ::wgl_glCopyTexImage2D(target, level, internalformat, x, y, width, height, border); };
void (* const glCopyTexSubImage2D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) = [](GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) { if (!opengl32) loadWGL(); ::wgl_glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height); };
GLuint(* const glCreateProgram)() = [](void) { if (!opengl32) loadWGL(); return ::wgl_glCreateProgram(); };
GLuint(* const glCreateShader)(GLenum) = [](GLenum type) { if (!opengl32) loadWGL(); return ::wgl_glCreateShader(type); };
void (* const glCullFace)(GLenum) = [](GLenum mode) { if (!opengl32) loadWGL(); ::wgl_glCullFace(mode); };
void (* const glDeleteBuffers)(GLsizei, const GLuint*) = [](GLsizei n, const GLuint* buffers) { if (!opengl32) loadWGL(); ::wgl_glDeleteBuffers(n, buffers); };
void (* const glDeleteFramebuffers)(GLsizei, const GLuint*) = [](GLsizei n, const GLuint* framebuffers) { if (!opengl32) loadWGL(); ::wgl_glDeleteFramebuffers(n, framebuffers); };
void (* const glDeleteProgram)(GLuint) = [](GLuint program) { if (!opengl32) loadWGL(); ::wgl_glDeleteProgram(program); };
void (* const glDeleteRenderbuffers)(GLsizei, const GLuint*) = [](GLsizei n, const GLuint* renderbuffers) { if (!opengl32) loadWGL(); ::wgl_glDeleteRenderbuffers(n, renderbuffers); };
void (* const glDeleteShader)(GLuint) = [](GLuint shader) { if (!opengl32) loadWGL(); ::wgl_glDeleteShader(shader); };
void (* const glDeleteTextures)(GLsizei, const GLuint*) = [](GLsizei n, const GLuint* textures) { if (!opengl32) loadWGL(); ::wgl_glDeleteTextures(n, textures); };
void (* const glDepthFunc)(GLenum) = [](GLenum func) { if (!opengl32) loadWGL(); ::wgl_glDepthFunc(func); };
void (* const glDepthMask)(GLboolean) = [](GLboolean flag) { if (!opengl32) loadWGL(); ::wgl_glDepthMask(flag); };
void (* const glDepthRangef)(GLfloat, GLfloat) = [](GLfloat n, GLfloat f) { if (!opengl32) loadWGL(); ::wgl_glDepthRangef(n, f); };
void (* const glDetachShader)(GLuint, GLuint) = [](GLuint program, GLuint shader) { if (!opengl32) loadWGL(); ::wgl_glDetachShader(program, shader); };
void (* const glDisable)(GLenum) = [](GLenum cap) { if (!opengl32) loadWGL(); ::wgl_glDisable(cap); };
void (* const glDisableVertexAttribArray)(GLuint) = [](GLuint index) { if (!opengl32) loadWGL(); ::wgl_glDisableVertexAttribArray(index); };
void (* const glDrawArrays)(GLenum, GLint, GLsizei) = [](GLenum mode, GLint first, GLsizei count) { if (!opengl32) loadWGL(); ::wgl_glDrawArrays(mode, first, count); };
void (* const glDrawElements)(GLenum, GLsizei, GLenum, const void*) = [](GLenum mode, GLsizei count, GLenum type, const void* indices) { if (!opengl32) loadWGL(); ::wgl_glDrawElements(mode, count, type, indices); };
void (* const glEnable)(GLenum) = [](GLenum cap) { if (!opengl32) loadWGL(); ::wgl_glEnable(cap); };
void (* const glEnableVertexAttribArray)(GLuint) = [](GLuint index) { if (!opengl32) loadWGL(); ::wgl_glEnableVertexAttribArray(index); };
void (* const glFinish)() = [](void) { if (!opengl32) loadWGL(); ::wgl_glFinish(); };
void (* const glFlush)() = [](void) { if (!opengl32) loadWGL(); ::wgl_glFlush(); };
void (* const glFramebufferRenderbuffer)(GLenum, GLenum, GLenum, GLuint) = [](GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) { if (!opengl32) loadWGL(); ::wgl_glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer); };
void (* const glFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint) = [](GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) { if (!opengl32) loadWGL(); ::wgl_glFramebufferTexture2D(target, attachment, textarget, texture, level); };
void (* const glFrontFace)(GLenum) = [](GLenum mode) { if (!opengl32) loadWGL(); ::wgl_glFrontFace(mode); };
void (* const glGenBuffers)(GLsizei, GLuint*) = [](GLsizei n, GLuint* buffers) { if (!opengl32) loadWGL(); ::wgl_glGenBuffers(n, buffers); };
void (* const glGenerateMipmap)(GLenum) = [](GLenum target) { if (!opengl32) loadWGL(); ::wgl_glGenerateMipmap(target); };
void (* const glGenFramebuffers)(GLsizei, GLuint*) = [](GLsizei n, GLuint* framebuffers) { if (!opengl32) loadWGL(); ::wgl_glGenFramebuffers(n, framebuffers); };
void (* const glGenRenderbuffers)(GLsizei, GLuint*) = [](GLsizei n, GLuint* renderbuffers) { if (!opengl32) loadWGL(); ::wgl_glGenRenderbuffers(n, renderbuffers); };
void (* const glGenTextures)(GLsizei, GLuint*) = [](GLsizei n, GLuint* textures) { if (!opengl32) loadWGL(); ::wgl_glGenTextures(n, textures); };
void (* const glGetActiveAttrib)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*) = [](GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) { if (!opengl32) loadWGL(); ::wgl_glGetActiveAttrib(program, index, bufSize, length, size, type, name); };
void (* const glGetActiveUniform)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*) = [](GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) { if (!opengl32) loadWGL(); ::wgl_glGetActiveUniform(program, index, bufSize, length, size, type, name); };
void (* const glGetAttachedShaders)(GLuint, GLsizei, GLsizei*, GLuint*) = [](GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders) { if (!opengl32) loadWGL(); ::wgl_glGetAttachedShaders(program, maxCount, count, shaders); };
GLint(* const glGetAttribLocation)(GLuint, const GLchar*) = [](GLuint program, const GLchar* name) { if (!opengl32) loadWGL(); return ::wgl_glGetAttribLocation(program, name); };
void (* const glGetBooleanv)(GLenum, GLboolean*) = [](GLenum pname, GLboolean* data) { if (!opengl32) loadWGL(); ::wgl_glGetBooleanv(pname, data); };
void (* const glGetBufferParameteriv)(GLenum, GLenum, GLint*) = [](GLenum target, GLenum pname, GLint* params) { if (!opengl32) loadWGL(); ::wgl_glGetBufferParameteriv(target, pname, params); };
GLenum(* const glGetError)() = [](void) { if (!opengl32) loadWGL(); return ::wgl_glGetError(); };
void (* const glGetFloatv)(GLenum, GLfloat*) = [](GLenum pname, GLfloat* data) { if (!opengl32) loadWGL(); ::wgl_glGetFloatv(pname, data); };
void (* const glGetFramebufferAttachmentParameteriv)(GLenum, GLenum, GLenum, GLint*) = [](GLenum target, GLenum attachment, GLenum pname, GLint* params) { if (!opengl32) loadWGL(); ::wgl_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params); };
void (* const glGetIntegerv)(GLenum, GLint*) = [](GLenum pname, GLint* data) { if (!opengl32) loadWGL(); ::wgl_glGetIntegerv(pname, data); };
void (* const glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*) = [](GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog) { if (!opengl32) loadWGL(); ::wgl_glGetProgramInfoLog(program, bufSize, length, infoLog); };
void (* const glGetProgramiv)(GLuint, GLenum, GLint*) = [](GLuint program, GLenum pname, GLint* params) { if (!opengl32) loadWGL(); ::wgl_glGetProgramiv(program, pname, params); };
void (* const glGetRenderbufferParameteriv)(GLenum, GLenum, GLint*) = [](GLenum target, GLenum pname, GLint* params) { if (!opengl32) loadWGL(); ::wgl_glGetRenderbufferParameteriv(target, pname, params); };
void (* const glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*) = [](GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) { if (!opengl32) loadWGL(); ::wgl_glGetShaderInfoLog(shader, bufSize, length, infoLog); };
void (* const glGetShaderiv)(GLuint, GLenum, GLint*) = [](GLuint shader, GLenum pname, GLint* params) { if (!opengl32) loadWGL(); ::wgl_glGetShaderiv(shader, pname, params); };
void (* const glGetShaderSource)(GLuint, GLsizei, GLsizei*, GLchar*) = [](GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source) { if (!opengl32) loadWGL(); ::wgl_glGetShaderSource(shader, bufSize, length, source); };
const GLubyte* (* const glGetString)(GLenum) = [](GLenum name) { if (!opengl32) loadWGL(); return ::wgl_glGetString(name); };
void (* const glGetTexParameterfv)(GLenum, GLenum, GLfloat*) = [](GLenum target, GLenum pname, GLfloat* params) { if (!opengl32) loadWGL(); ::wgl_glGetTexParameterfv(target, pname, params); };
void (* const glGetTexParameteriv)(GLenum, GLenum, GLint*) = [](GLenum target, GLenum pname, GLint* params) { if (!opengl32) loadWGL(); ::wgl_glGetTexParameteriv(target, pname, params); };
void (* const glGetUniformfv)(GLuint, GLint, GLfloat*) = [](GLuint program, GLint location, GLfloat* params) { if (!opengl32) loadWGL(); ::wgl_glGetUniformfv(program, location, params); };
void (* const glGetUniformiv)(GLuint, GLint, GLint*) = [](GLuint program, GLint location, GLint* params) { if (!opengl32) loadWGL(); ::wgl_glGetUniformiv(program, location, params); };
GLint(* const glGetUniformLocation)(GLuint, const GLchar*) = [](GLuint program, const GLchar* name) { if (!opengl32) loadWGL(); return ::wgl_glGetUniformLocation(program, name); };
void (* const glGetVertexAttribfv)(GLuint, GLenum, GLfloat*) = [](GLuint index, GLenum pname, GLfloat* params) { if (!opengl32) loadWGL(); ::wgl_glGetVertexAttribfv(index, pname, params); };
void (* const glGetVertexAttribiv)(GLuint, GLenum, GLint*) = [](GLuint index, GLenum pname, GLint* params) { if (!opengl32) loadWGL(); ::wgl_glGetVertexAttribiv(index, pname, params); };
void (* const glGetVertexAttribPointerv)(GLuint, GLenum, void**) = [](GLuint index, GLenum pname, void** pointer) { if (!opengl32) loadWGL(); ::wgl_glGetVertexAttribPointerv(index, pname, pointer); };
void (* const glHint)(GLenum, GLenum) = [](GLenum target, GLenum mode) { if (!opengl32) loadWGL(); ::wgl_glHint(target, mode); };
GLboolean(* const glIsBuffer)(GLuint) = [](GLuint buffer) { if (!opengl32) loadWGL(); return ::wgl_glIsBuffer(buffer); };
GLboolean(* const glIsEnabled)(GLenum) = [](GLenum cap) { if (!opengl32) loadWGL(); return ::wgl_glIsEnabled(cap); };
GLboolean(* const glIsFramebuffer)(GLuint) = [](GLuint framebuffer) { if (!opengl32) loadWGL(); return ::wgl_glIsFramebuffer(framebuffer); };
GLboolean(* const glIsProgram)(GLuint) = [](GLuint program) { if (!opengl32) loadWGL(); return ::wgl_glIsProgram(program); };
GLboolean(* const glIsRenderbuffer)(GLuint) = [](GLuint renderbuffer) { if (!opengl32) loadWGL(); return ::wgl_glIsRenderbuffer(renderbuffer); };
GLboolean(* const glIsShader)(GLuint) = [](GLuint shader) { if (!opengl32) loadWGL(); return ::wgl_glIsShader(shader); };
GLboolean(* const glIsTexture)(GLuint) = [](GLuint texture) { if (!opengl32) loadWGL(); return ::wgl_glIsTexture(texture); };
void (* const glLineWidth)(GLfloat) = [](GLfloat width) { if (!opengl32) loadWGL(); ::wgl_glLineWidth(width); };
void (* const glLinkProgram)(GLuint) = [](GLuint program) { if (!opengl32) loadWGL(); ::wgl_glLinkProgram(program); };
void (* const glPixelStorei)(GLenum, GLint) = [](GLenum pname, GLint param) { if (!opengl32) loadWGL(); ::wgl_glPixelStorei(pname, param); };
void (* const glPolygonOffset)(GLfloat, GLfloat) = [](GLfloat factor, GLfloat units) { if (!opengl32) loadWGL(); ::wgl_glPolygonOffset(factor, units); };
void (* const glReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*) = [](GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels) { if (!opengl32) loadWGL(); ::wgl_glReadPixels(x, y, width, height, format, type, pixels); };
void (* const glRenderbufferStorage)(GLenum, GLenum, GLsizei, GLsizei) = [](GLenum target, GLenum internalformat, GLsizei width, GLsizei height) { if (!opengl32) loadWGL(); ::wgl_glRenderbufferStorage(target, internalformat, width, height); };
void (* const glSampleCoverage)(GLfloat, GLboolean) = [](GLfloat value, GLboolean invert) { if (!opengl32) loadWGL(); ::wgl_glSampleCoverage(value, invert); };
void (* const glScissor)(GLint, GLint, GLsizei, GLsizei) = [](GLint x, GLint y, GLsizei width, GLsizei height) { if (!opengl32) loadWGL(); ::wgl_glScissor(x, y, width, height); };
void (* const glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*) = [](GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) { if (!opengl32) loadWGL(); ::wgl_glShaderSource(shader, count, string, length); };
void (* const glStencilFunc)(GLenum, GLint, GLuint) = [](GLenum func, GLint ref, GLuint mask) { if (!opengl32) loadWGL(); ::wgl_glStencilFunc(func, ref, mask); };
void (* const glStencilFuncSeparate)(GLenum, GLenum, GLint, GLuint) = [](GLenum face, GLenum func, GLint ref, GLuint mask) { if (!opengl32) loadWGL(); ::wgl_glStencilFuncSeparate(face, func, ref, mask); };
void (* const glStencilMask)(GLuint) = [](GLuint mask) { if (!opengl32) loadWGL(); ::wgl_glStencilMask(mask); };
void (* const glStencilMaskSeparate)(GLenum, GLuint) = [](GLenum face, GLuint mask) { if (!opengl32) loadWGL(); ::wgl_glStencilMaskSeparate(face, mask); };
void (* const glStencilOp)(GLenum, GLenum, GLenum) = [](GLenum fail, GLenum zfail, GLenum zpass) { if (!opengl32) loadWGL(); ::wgl_glStencilOp(fail, zfail, zpass); };
void (* const glStencilOpSeparate)(GLenum, GLenum, GLenum, GLenum) = [](GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) { if (!opengl32) loadWGL(); ::wgl_glStencilOpSeparate(face, sfail, dpfail, dppass); };
void (* const glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*) = [](GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) { if (!opengl32) loadWGL(); ::wgl_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels); };
void (* const glTexParameterf)(GLenum, GLenum, GLfloat) = [](GLenum target, GLenum pname, GLfloat param) { if (!opengl32) loadWGL(); ::wgl_glTexParameterf(target, pname, param); };
void (* const glTexParameterfv)(GLenum, GLenum, const GLfloat*) = [](GLenum target, GLenum pname, const GLfloat* params) { if (!opengl32) loadWGL(); ::wgl_glTexParameterfv(target, pname, params); };
void (* const glTexParameteri)(GLenum, GLenum, GLint) = [](GLenum target, GLenum pname, GLint param) { if (!opengl32) loadWGL(); ::wgl_glTexParameteri(target, pname, param); };
void (* const glTexParameteriv)(GLenum, GLenum, const GLint*) = [](GLenum target, GLenum pname, const GLint* params) { if (!opengl32) loadWGL(); ::wgl_glTexParameteriv(target, pname, params); };
void (* const glTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*) = [](GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) { if (!opengl32) loadWGL(); ::wgl_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels); };
void (* const glUniform1f)(GLint, GLfloat) = [](GLint location, GLfloat v0) { if (!opengl32) loadWGL(); ::wgl_glUniform1f(location, v0); };
void (* const glUniform1fv)(GLint, GLsizei, const GLfloat*) = [](GLint location, GLsizei count, const GLfloat* value) { if (!opengl32) loadWGL(); ::wgl_glUniform1fv(location, count, value); };
void (* const glUniform1i)(GLint, GLint) = [](GLint location, GLint v0) { if (!opengl32) loadWGL(); ::wgl_glUniform1i(location, v0); };
void (* const glUniform1iv)(GLint, GLsizei, const GLint*) = [](GLint location, GLsizei count, const GLint* value) { if (!opengl32) loadWGL(); ::wgl_glUniform1iv(location, count, value); };
void (* const glUniform2f)(GLint, GLfloat, GLfloat) = [](GLint location, GLfloat v0, GLfloat v1) { if (!opengl32) loadWGL(); ::wgl_glUniform2f(location, v0, v1); };
void (* const glUniform2fv)(GLint, GLsizei, const GLfloat*) = [](GLint location, GLsizei count, const GLfloat* value) { if (!opengl32) loadWGL(); ::wgl_glUniform2fv(location, count, value); };
void (* const glUniform2i)(GLint, GLint, GLint) = [](GLint location, GLint v0, GLint v1) { if (!opengl32) loadWGL(); ::wgl_glUniform2i(location, v0, v1); };
void (* const glUniform2iv)(GLint, GLsizei, const GLint*) = [](GLint location, GLsizei count, const GLint* value) { if (!opengl32) loadWGL(); ::wgl_glUniform2iv(location, count, value); };
void (* const glUniform3f)(GLint, GLfloat, GLfloat, GLfloat) = [](GLint location, GLfloat v0, GLfloat v1, GLfloat v2) { if (!opengl32) loadWGL(); ::wgl_glUniform3f(location, v0, v1, v2); };
void (* const glUniform3fv)(GLint, GLsizei, const GLfloat*) = [](GLint location, GLsizei count, const GLfloat* value) { if (!opengl32) loadWGL(); ::wgl_glUniform3fv(location, count, value); };
void (* const glUniform3i)(GLint, GLint, GLint, GLint) = [](GLint location, GLint v0, GLint v1, GLint v2) { if (!opengl32) loadWGL(); ::wgl_glUniform3i(location, v0, v1, v2); };
void (* const glUniform3iv)(GLint, GLsizei, const GLint*) = [](GLint location, GLsizei count, const GLint* value) { if (!opengl32) loadWGL(); ::wgl_glUniform3iv(location, count, value); };
void (* const glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat) = [](GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) { if (!opengl32) loadWGL(); ::wgl_glUniform4f(location, v0, v1, v2, v3); };
void (* const glUniform4fv)(GLint, GLsizei, const GLfloat*) = [](GLint location, GLsizei count, const GLfloat* value) { if (!opengl32) loadWGL(); ::wgl_glUniform4fv(location, count, value); };
void (* const glUniform4i)(GLint, GLint, GLint, GLint, GLint) = [](GLint location, GLint v0, GLint v1, GLint v2, GLint v3) { if (!opengl32) loadWGL(); ::wgl_glUniform4i(location, v0, v1, v2, v3); };
void (* const glUniform4iv)(GLint, GLsizei, const GLint*) = [](GLint location, GLsizei count, const GLint* value) { if (!opengl32) loadWGL(); ::wgl_glUniform4iv(location, count, value); };
void (* const glUniformMatrix2fv)(GLint, GLsizei, GLboolean, const GLfloat*) = [](GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { if (!opengl32) loadWGL(); ::wgl_glUniformMatrix2fv(location, count, transpose, value); };
void (* const glUniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat*) = [](GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { if (!opengl32) loadWGL(); ::wgl_glUniformMatrix3fv(location, count, transpose, value); };
void (* const glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*) = [](GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { if (!opengl32) loadWGL(); ::wgl_glUniformMatrix4fv(location, count, transpose, value); };
void (* const glUseProgram)(GLuint) = [](GLuint program) { if (!opengl32) loadWGL(); ::wgl_glUseProgram(program); };
void (* const glValidateProgram)(GLuint) = [](GLuint program) { if (!opengl32) loadWGL(); ::wgl_glValidateProgram(program); };
void (* const glVertexAttrib1f)(GLuint, GLfloat) = [](GLuint index, GLfloat x) { if (!opengl32) loadWGL(); ::wgl_glVertexAttrib1f(index, x); };
void (* const glVertexAttrib1fv)(GLuint, const GLfloat*) = [](GLuint index, const GLfloat* v) { if (!opengl32) loadWGL(); ::wgl_glVertexAttrib1fv(index, v); };
void (* const glVertexAttrib2f)(GLuint, GLfloat, GLfloat) = [](GLuint index, GLfloat x, GLfloat y) { if (!opengl32) loadWGL(); ::wgl_glVertexAttrib2f(index, x, y); };
void (* const glVertexAttrib2fv)(GLuint, const GLfloat*) = [](GLuint index, const GLfloat* v) { if (!opengl32) loadWGL(); ::wgl_glVertexAttrib2fv(index, v); };
void (* const glVertexAttrib3f)(GLuint, GLfloat, GLfloat, GLfloat) = [](GLuint index, GLfloat x, GLfloat y, GLfloat z) { if (!opengl32) loadWGL(); ::wgl_glVertexAttrib3f(index, x, y, z); };
void (* const glVertexAttrib3fv)(GLuint, const GLfloat*) = [](GLuint index, const GLfloat* v) { if (!opengl32) loadWGL(); ::wgl_glVertexAttrib3fv(index, v); };
void (* const glVertexAttrib4f)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat) = [](GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) { if (!opengl32) loadWGL(); ::wgl_glVertexAttrib4f(index, x, y, z, w); };
void (* const glVertexAttrib4fv)(GLuint, const GLfloat*) = [](GLuint index, const GLfloat* v) { if (!opengl32) loadWGL(); ::wgl_glVertexAttrib4fv(index, v); };
void (* const glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*) = [](GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) { if (!opengl32) loadWGL(); ::wgl_glVertexAttribPointer(index, size, type, normalized, stride, pointer); };
void (* const glViewport)(GLint, GLint, GLsizei, GLsizei) = [](GLint x, GLint y, GLsizei width, GLsizei height) { if (!opengl32) loadWGL(); ::wgl_glViewport(x, y, width, height); };

BOOL(* const wglChoosePixelFormatARB)(HDC, const int*, const FLOAT*, UINT, int*, UINT*) = [](HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats) { if (!opengl32) loadWGL(); return ::wgl_wglChoosePixelFormatARB(hdc, piAttribIList, pfAttribFList, nMaxFormats, piFormats, nNumFormats); };
HGLRC(* const wglCreateContextAttribsARB)(HDC, HGLRC, const int*) = [](HDC hDC, HGLRC hShareContext, const int* attribList) { if (!opengl32) loadWGL(); return ::wgl_wglCreateContextAttribsARB(hDC, hShareContext, attribList); };
const char* (* const wglGetExtensionsStringARB)(HDC) = [](HDC hdc) { if (!opengl32) loadWGL(); return ::wgl_wglGetExtensionsStringARB(hdc); };
const char* (* const wglGetExtensionsStringEXT)() = [](void) { if (!opengl32) loadWGL(); return ::wgl_wglGetExtensionsStringEXT(); };

#ifndef MBGL_USE_GLES2
void (* const glDrawPixels)(GLsizei, GLsizei, GLenum, GLenum, const GLvoid *) = ::glDrawPixels;
void (* const glGetDoublev)(GLenum, GLdouble *) = ::glGetDoublev;
void (* const glPixelTransferf)(GLenum, GLfloat) = ::glPixelTransferf;
void (* const glPixelZoom)(GLfloat, GLfloat) = ::glPixelZoom;
void (* const glPointSize)(GLfloat) = ::glPointSize;
void (* const glRasterPos4d)(GLdouble, GLdouble, GLdouble, GLdouble) = ::glRasterPos4d;
#endif

}  // namespace platform
}  // namespace mbgl
#endif
