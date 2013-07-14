#pragma once
#include "ogl/ogl_stdafx.h"


#define MAKI_EXTERN_GL_FUNC(Type, Name) extern PFN##Type##PROC Name;
			
extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARB;
extern PFNGLBINDBUFFERARBPROC glBindBufferARB;
extern PFNGLBUFFERDATAARBPROC glBufferDataARB;
extern PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArrayARB;
extern PFNGLGENBUFFERSARBPROC glGenBuffersARB;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLGETPROGRAMIVARBPROC glGetProgramivARB;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
extern PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
extern PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
extern PFNGLUNIFORM1FARBPROC glUniform1fARB;
extern PFNGLUNIFORM1IARBPROC glUniform1iARB;
extern PFNGLUNIFORM2FARBPROC glUniform2fARB;
extern PFNGLUNIFORM2IARBPROC glUniform2iARB;
extern PFNGLUNIFORM3FARBPROC glUniform3fARB;
extern PFNGLUNIFORM3FVARBPROC glUniform3fvARB;
extern PFNGLUNIFORM3IARBPROC glUniform3iARB;
extern PFNGLUNIFORM4FARBPROC glUniform4fARB;
extern PFNGLUNIFORM4IARBPROC glUniform4iARB;
extern PFNGLUNIFORMMATRIX3FVARBPROC glUniformMatrix3fvARB;
extern PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fvARB;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

// GL_ARB_framebuffer_object
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;

#if _DEBUG
extern PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB;
#endif


namespace Maki
{
	namespace OGL
	{
		
		bool _Failed();

#if _DEBUG
#		define MAKI_OGL_FAILED() _Failed()
#else
#		define MAKI_OGL_FAILED() false
#endif

		extern const GLenum indicesPerFaceToGeometryType[4];
		extern const GLenum bytesPerIndexToFormat[5];
		extern const GLenum channelsToFormat[5];

		void DefineGLFunctions();

	} // namespace D3D

} // namespace Maki
