#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLCommon.h"

/*
#define MAKI_DEFINE_GL_FUNC(Name) Name = GetOGLFunc(Name, #Name);
template<typename FT> inline FT GetOGLFunc(FT f, const char *name) {
	FT val = nullptr;
	val = (FT)SDL_GL_GetProcAddress(name);
	if(val == nullptr) {
		Maki::Core::Console::Warning("Failed to get address of OpenGL function: %s", name);
	}
	return val;
}
	
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLMAPBUFFERPROC glMapBuffer;
PFNGLUNMAPBUFFERPROC glUnmapBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLISPROGRAMPROC glIsProgram;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM1FVPROC glUniform1fv;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM2IPROC glUniform2i;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORM3IPROC glUniform3i;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLUNIFORM4IPROC glUniform4i;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;


PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
PFNGLBINDBUFFERBASEPROC glBindBufferBase;


// GL__framebuffer_object
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;

PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
*/

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

		const GLenum typeToGLType[VertexFormat::DataTypeCount] = {
			GL_FLOAT, GL_UNSIGNED_INT, GL_UNSIGNED_SHORT, GL_UNSIGNED_BYTE
		};

		// This is a map from our attribute enums to the string names used in the glsl shaders.
		// The reason these strings are a bit weird (cg_Vertex for instance) is that we are using
		// the ugly glsl code generated by the cg compiler, and not using the cg runtime like they want you to.
		const char *attributeName[Core::VertexFormat::AttributeCount] = {
			"Position",
			"Normal",
			"Tangent",
			"Color",
			"Color1",
			"TexCoord",
			"BoneWeights"
		};

		GLboolean normalizeAttribute[Core::VertexFormat::AttributeCount] = {
			GL_FALSE,
			GL_FALSE,
			GL_FALSE,
			GL_TRUE,
			GL_TRUE,
			GL_FALSE,
			GL_FALSE
		};

		void DefineGLFunctions()
		{
			static bool loaded = false;
			if(loaded) {
				return;
			}

			glewInit();

/*
			MAKI_DEFINE_GL_FUNC(glBufferData);
			MAKI_DEFINE_GL_FUNC(glBufferSubData);
			MAKI_DEFINE_GL_FUNC(glGenerateMipmap);
			MAKI_DEFINE_GL_FUNC(glCompressedTexImage2D);
			MAKI_DEFINE_GL_FUNC(glCompileShader);
			MAKI_DEFINE_GL_FUNC(glAttachShader);
			MAKI_DEFINE_GL_FUNC(glCreateProgram);
			MAKI_DEFINE_GL_FUNC(glCreateShader);
			MAKI_DEFINE_GL_FUNC(glDeleteBuffers);
			MAKI_DEFINE_GL_FUNC(glDeleteProgram);
			MAKI_DEFINE_GL_FUNC(glDeleteShader);
			MAKI_DEFINE_GL_FUNC(glDisableVertexAttribArray);
			MAKI_DEFINE_GL_FUNC(glEnableVertexAttribArray);
			MAKI_DEFINE_GL_FUNC(glGenBuffers);
			MAKI_DEFINE_GL_FUNC(glGetProgramInfoLog);
			MAKI_DEFINE_GL_FUNC(glGetProgramiv);
			MAKI_DEFINE_GL_FUNC(glGetShaderInfoLog);
			MAKI_DEFINE_GL_FUNC(glGetShaderiv);
			MAKI_DEFINE_GL_FUNC(glGetUniformLocation);
			MAKI_DEFINE_GL_FUNC(glGetActiveUniform);
			MAKI_DEFINE_GL_FUNC(glGetActiveUniformsiv);
			MAKI_DEFINE_GL_FUNC(glGetActiveUniformBlockiv);
			MAKI_DEFINE_GL_FUNC(glLinkProgram);
			MAKI_DEFINE_GL_FUNC(glIsProgram);
			MAKI_DEFINE_GL_FUNC(glShaderSource);

			MAKI_DEFINE_GL_FUNC(glBindBuffer);
			MAKI_DEFINE_GL_FUNC(glMapBuffer);
			MAKI_DEFINE_GL_FUNC(glUnmapBuffer);
			MAKI_DEFINE_GL_FUNC(glActiveTexture);
			MAKI_DEFINE_GL_FUNC(glBindAttribLocation);
			MAKI_DEFINE_GL_FUNC(glGetAttribLocation);
			MAKI_DEFINE_GL_FUNC(glUniform1f);
			MAKI_DEFINE_GL_FUNC(glUniform1fv);
			MAKI_DEFINE_GL_FUNC(glUniform1i);
			MAKI_DEFINE_GL_FUNC(glUniform2f);
			MAKI_DEFINE_GL_FUNC(glUniform2i);
			MAKI_DEFINE_GL_FUNC(glUniform3f);
			MAKI_DEFINE_GL_FUNC(glUniform3fv);
			MAKI_DEFINE_GL_FUNC(glUniform3i);
			MAKI_DEFINE_GL_FUNC(glUniform4f);
			MAKI_DEFINE_GL_FUNC(glUniform4fv);
			MAKI_DEFINE_GL_FUNC(glUniform4i);
			MAKI_DEFINE_GL_FUNC(glUniformMatrix3fv);
			MAKI_DEFINE_GL_FUNC(glUniformMatrix4fv);
			MAKI_DEFINE_GL_FUNC(glUseProgram);
			MAKI_DEFINE_GL_FUNC(glGenVertexArrays);
			MAKI_DEFINE_GL_FUNC(glBindVertexArray);
			MAKI_DEFINE_GL_FUNC(glDeleteVertexArrays);
			MAKI_DEFINE_GL_FUNC(glVertexAttribPointer);


			MAKI_DEFINE_GL_FUNC(glGetUniformBlockIndex);
			MAKI_DEFINE_GL_FUNC(glUniformBlockBinding);
			MAKI_DEFINE_GL_FUNC(glBindBufferBase);

			// GL__framebuffer_object
			MAKI_DEFINE_GL_FUNC(glGenFramebuffers);
			MAKI_DEFINE_GL_FUNC(glGetFramebufferAttachmentParameteriv);
			MAKI_DEFINE_GL_FUNC(glDeleteFramebuffers);
			MAKI_DEFINE_GL_FUNC(glBindFramebuffer);
			MAKI_DEFINE_GL_FUNC(glFramebufferTexture2D);
			MAKI_DEFINE_GL_FUNC(glGenRenderbuffers);
			MAKI_DEFINE_GL_FUNC(glDeleteRenderbuffers);
			MAKI_DEFINE_GL_FUNC(glBindRenderbuffer);
			MAKI_DEFINE_GL_FUNC(glRenderbufferStorage);
			MAKI_DEFINE_GL_FUNC(glFramebufferRenderbuffer);
			MAKI_DEFINE_GL_FUNC(glCheckFramebufferStatus);

			MAKI_DEFINE_GL_FUNC(glDebugMessageCallback);
*/
			loaded = true;
		}


	} // namespace D3D

} // namespace Maki
