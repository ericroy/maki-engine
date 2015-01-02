#pragma once
#include <GL/gl.h>


/*
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
*/


namespace maki
{
	namespace ogl
	{
		
		bool __failed();

#if _DEBUG
#		define MAKI_OGL_FAILED() __failed()
#else
#		define MAKI_OGL_FAILED() false
#endif
		const int32 shadow_map_slot_index_start = 8;
		
		extern const GLenum indices_per_face_to_geometry_type[4];
		extern const GLenum bytes_per_index_to_format[5];
		extern const GLenum channels_to_format[5];
		extern const GLenum type_to_gl_type[core::vertex_format_t::data_type_count_];
		extern const char *attribute_name[core::vertex_format_t::attribute_count_];
		extern GLboolean normalize_attribute[core::vertex_format_t::attribute_count_];

		void define_gl_functions();
		
	} // namespace D3D

} // namespace maki
