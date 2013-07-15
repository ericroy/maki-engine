#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLCommon.h"


#define MAKI_DEFINE_GL_FUNC(Name) Name = GetOGLFunc(Name, #Name);
template<typename FT> inline FT GetOGLFunc(FT f, char *name) {
	FT val = nullptr;
	val = (FT)SDL_GL_GetProcAddress(name);
	if(val == nullptr) {
		Console::Warning("Failed to get address of OpenGL function: %s", name);
	}
	return val;
}
			
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARB;
PFNGLBINDBUFFERARBPROC glBindBufferARB;
PFNGLBUFFERDATAARBPROC glBufferDataARB;
PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArrayARB;
PFNGLGENBUFFERSARBPROC glGenBuffersARB;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLGETPROGRAMIVARBPROC glGetProgramivARB;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
PFNGLUNIFORM1FARBPROC glUniform1fARB;
PFNGLUNIFORM1IARBPROC glUniform1iARB;
PFNGLUNIFORM2FARBPROC glUniform2fARB;
PFNGLUNIFORM2IARBPROC glUniform2iARB;
PFNGLUNIFORM3FARBPROC glUniform3fARB;
PFNGLUNIFORM3FVARBPROC glUniform3fvARB;
PFNGLUNIFORM3IARBPROC glUniform3iARB;
PFNGLUNIFORM4FARBPROC glUniform4fARB;
PFNGLUNIFORM4IARBPROC glUniform4iARB;
PFNGLUNIFORMMATRIX3FVARBPROC glUniformMatrix3fvARB;
PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fvARB;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

// GL_ARB_framebuffer_object
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;


#if _DEBUG
PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB;
#endif

using namespace Maki::Core;

namespace Maki
{
	namespace OGL
	{
		
		bool _Failed()
		{
			GLenum error = glGetError();
			if(error != GL_NONE) {
				Console::Error("OGL: %s", (const char *)gluErrorString(error));
				return true;
			}
			return false;
		}

		const GLenum indicesPerFaceToGeometryType[4] = {
			GL_INVALID_ENUM,
			GL_POINTS,
			GL_LINES,
			GL_TRIANGLES,
		};
	
		const GLenum bytesPerIndexToFormat[5] = {
			GL_INVALID_ENUM,
			GL_INVALID_ENUM,
			GL_UNSIGNED_SHORT,
			GL_INVALID_ENUM,
			GL_UNSIGNED_INT,
		};

		const GLenum channelsToFormat[5] = {
			GL_INVALID_ENUM,
			GL_ALPHA,
			GL_RG,
			GL_INVALID_ENUM,			// 3 channels texture not supported
			GL_RGBA
		};


		void DefineGLFunctions()
		{
			static bool loaded = false;
			if(loaded) {
				return;
			}

			MAKI_DEFINE_GL_FUNC(glActiveTextureARB);
			MAKI_DEFINE_GL_FUNC(glAttachShader);
			MAKI_DEFINE_GL_FUNC(glBindAttribLocationARB);
			MAKI_DEFINE_GL_FUNC(glBindBufferARB);
			MAKI_DEFINE_GL_FUNC(glBufferDataARB);
			MAKI_DEFINE_GL_FUNC(glCompileShaderARB);
			MAKI_DEFINE_GL_FUNC(glCreateProgram);
			MAKI_DEFINE_GL_FUNC(glCreateShader);
			MAKI_DEFINE_GL_FUNC(glDeleteBuffersARB);
			MAKI_DEFINE_GL_FUNC(glDeleteProgram);
			MAKI_DEFINE_GL_FUNC(glDeleteShader);
			MAKI_DEFINE_GL_FUNC(glDisableVertexAttribArrayARB);
			MAKI_DEFINE_GL_FUNC(glEnableVertexAttribArrayARB);
			MAKI_DEFINE_GL_FUNC(glGenBuffersARB);
			MAKI_DEFINE_GL_FUNC(glGetProgramInfoLog);
			MAKI_DEFINE_GL_FUNC(glGetProgramivARB);
			MAKI_DEFINE_GL_FUNC(glGetShaderInfoLog);
			MAKI_DEFINE_GL_FUNC(glGetShaderiv);
			MAKI_DEFINE_GL_FUNC(glGetUniformLocationARB);
			MAKI_DEFINE_GL_FUNC(glLinkProgramARB);
			MAKI_DEFINE_GL_FUNC(glShaderSourceARB);
			MAKI_DEFINE_GL_FUNC(glUniform1fARB);
			MAKI_DEFINE_GL_FUNC(glUniform1iARB);
			MAKI_DEFINE_GL_FUNC(glUniform2fARB);
			MAKI_DEFINE_GL_FUNC(glUniform2iARB);
			MAKI_DEFINE_GL_FUNC(glUniform3fARB);
			MAKI_DEFINE_GL_FUNC(glUniform3fvARB);
			MAKI_DEFINE_GL_FUNC(glUniform3iARB);
			MAKI_DEFINE_GL_FUNC(glUniform4fARB);
			MAKI_DEFINE_GL_FUNC(glUniform4iARB);
			MAKI_DEFINE_GL_FUNC(glUniformMatrix3fvARB);
			MAKI_DEFINE_GL_FUNC(glUniformMatrix4fvARB);
			MAKI_DEFINE_GL_FUNC(glUseProgram);
			MAKI_DEFINE_GL_FUNC(glVertexAttribPointer);

			// GL_ARB_framebuffer_object
			MAKI_DEFINE_GL_FUNC(glGenFramebuffers);
			MAKI_DEFINE_GL_FUNC(glDeleteFramebuffers);
			MAKI_DEFINE_GL_FUNC(glBindFramebuffer);
			MAKI_DEFINE_GL_FUNC(glFramebufferTexture2D);
			MAKI_DEFINE_GL_FUNC(glGenRenderbuffers);
			MAKI_DEFINE_GL_FUNC(glBindRenderbuffer);
			MAKI_DEFINE_GL_FUNC(glRenderbufferStorage);
			MAKI_DEFINE_GL_FUNC(glFramebufferRenderbuffer);

#if _DEBUG
			MAKI_DEFINE_GL_FUNC(glDebugMessageCallbackARB);
#endif

			loaded = true;
		}


	} // namespace D3D

} // namespace Maki
