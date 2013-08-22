#pragma once
#include "ogl/ogl_stdafx.h"


#define MAKI_EXTERN_GL_FUNC(Type, Name) extern PFN##Type##PROC Name;
			
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLMAPBUFFERPROC glMapBuffer;
extern PFNGLUNMAPBUFFERPROC glUnmapBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
extern PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv;
extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLISPROGRAMPROC glIsProgram;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM1FVPROC glUniform1fv;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM2FPROC glUniform2f;
extern PFNGLUNIFORM2IPROC glUniform2i;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLUNIFORM3IPROC glUniform3i;
extern PFNGLUNIFORM4FPROC glUniform4f;
extern PFNGLUNIFORM4FVPROC glUniform4fv;
extern PFNGLUNIFORM4IPROC glUniform4i;
extern PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;


extern PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
extern PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
extern PFNGLBINDBUFFERBASEPROC glBindBufferBase;


// framebuffer_object
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;

extern PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;



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
		const int32 SHADOW_MAP_SLOT_INDEX_START = 8;
		
		extern const GLenum indicesPerFaceToGeometryType[4];
		extern const GLenum bytesPerIndexToFormat[5];
		extern const GLenum channelsToFormat[5];
		extern const GLenum typeToGLType[Core::VertexFormat::DataTypeCount];
		extern const char *attributeName[Core::VertexFormat::AttributeCount];
		extern GLboolean normalizeAttribute[Core::VertexFormat::AttributeCount];

		void DefineGLFunctions();
		
	} // namespace D3D

} // namespace Maki
