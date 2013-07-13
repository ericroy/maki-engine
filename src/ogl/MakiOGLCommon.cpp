#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLCommon.h"

using namespace Maki::Core;

namespace Maki
{
	namespace OGL
	{
			
#define MAKI_DECLARE_GL_FUNC(Type, Name) PFNGL##Type##PROC gl##Name;
#define MAKI_DEFINE_GL_FUNC(Name) gl##Name = get_func(gl##Name, "gl"#Name);

		template<typename FT> inline FT get_func(FT f, char *name) {
			FT val = 0;
			val = (FT)SDL_GL_GetProcAddress(name);
			Console::Warning("Failed to get address of OpenGL function: %s", name);
			return val;
		}
			
		MAKI_DECLARE_GL_FUNC(ACTIVETEXTURE, ActiveTexture);
		MAKI_DECLARE_GL_FUNC(ATTACHSHADER, AttachShader);
		MAKI_DECLARE_GL_FUNC(BINDATTRIBLOCATION, BindAttribLocation);
		MAKI_DECLARE_GL_FUNC(BINDBUFFER, BindBuffer);
		MAKI_DECLARE_GL_FUNC(BUFFERDATA, BufferData);
		MAKI_DECLARE_GL_FUNC(COMPILESHADER, CompileShader);
		//MAKI_DECLARE_GL_FUNC(CREATEPROGRAM, CreateProgram);
		MAKI_DECLARE_GL_FUNC(CREATESHADER, CreateShader);
		MAKI_DECLARE_GL_FUNC(DELETEBUFFERS, DeleteBuffers);
		MAKI_DECLARE_GL_FUNC(DELETEPROGRAM, DeleteProgram);
		MAKI_DECLARE_GL_FUNC(DELETESHADER, DeleteShader);
		MAKI_DECLARE_GL_FUNC(DISABLEVERTEXATTRIBARRAY, DisableVertexAttribArray);
		MAKI_DECLARE_GL_FUNC(ENABLEVERTEXATTRIBARRAY, EnableVertexAttribArray);
		MAKI_DECLARE_GL_FUNC(GENBUFFERS, GenBuffers);
		MAKI_DECLARE_GL_FUNC(GETPROGRAMINFOLOG, GetProgramInfoLog);
		MAKI_DECLARE_GL_FUNC(GETPROGRAMIV, GetProgramiv);
		MAKI_DECLARE_GL_FUNC(GETSHADERINFOLOG, GetShaderInfoLog);
		MAKI_DECLARE_GL_FUNC(GETSHADERIV, GetShaderiv);
		MAKI_DECLARE_GL_FUNC(GETUNIFORMLOCATION, GetUniformLocation);
		MAKI_DECLARE_GL_FUNC(LINKPROGRAM, LinkProgram);
		MAKI_DECLARE_GL_FUNC(SHADERSOURCE, ShaderSource);
		MAKI_DECLARE_GL_FUNC(UNIFORM1F, Uniform1f);
		MAKI_DECLARE_GL_FUNC(UNIFORM1I, Uniform1i);
		MAKI_DECLARE_GL_FUNC(UNIFORM1UI, Uniform1ui);
		MAKI_DECLARE_GL_FUNC(UNIFORM2F, Uniform2f);
		MAKI_DECLARE_GL_FUNC(UNIFORM2I, Uniform2i);
		MAKI_DECLARE_GL_FUNC(UNIFORM2UI, Uniform2ui);
		MAKI_DECLARE_GL_FUNC(UNIFORM3F, Uniform3f);
		MAKI_DECLARE_GL_FUNC(UNIFORM3FV, Uniform3fv);
		MAKI_DECLARE_GL_FUNC(UNIFORM3I, Uniform3i);
		MAKI_DECLARE_GL_FUNC(UNIFORM3UI, Uniform3ui);
		MAKI_DECLARE_GL_FUNC(UNIFORM4F, Uniform4f);
		MAKI_DECLARE_GL_FUNC(UNIFORM4I, Uniform4i);
		MAKI_DECLARE_GL_FUNC(UNIFORM4UI, Uniform4ui);
		MAKI_DECLARE_GL_FUNC(UNIFORMMATRIX3FV, UniformMatrix3fv);
		MAKI_DECLARE_GL_FUNC(UNIFORMMATRIX4FV, UniformMatrix4fv);
		MAKI_DECLARE_GL_FUNC(USEPROGRAM, UseProgram);
		MAKI_DECLARE_GL_FUNC(VERTEXATTRIBPOINTER, VertexAttribPointer);

		void DefineGLFunctions()
		{
			static bool loaded = false;
			if(loaded) {
				return;
			}

			MAKI_DEFINE_GL_FUNC(ActiveTexture);
			MAKI_DEFINE_GL_FUNC(AttachShader);
			MAKI_DEFINE_GL_FUNC(BindAttribLocation);
			MAKI_DEFINE_GL_FUNC(BindBuffer);
			MAKI_DEFINE_GL_FUNC(BufferData);
			MAKI_DEFINE_GL_FUNC(CompileShader);
			//MAKI_DEFINE_GL_FUNC(CreateProgram);
			MAKI_DEFINE_GL_FUNC(CreateShader);
			MAKI_DEFINE_GL_FUNC(DeleteBuffers);
			MAKI_DEFINE_GL_FUNC(DeleteProgram);
			MAKI_DEFINE_GL_FUNC(DeleteShader);
			MAKI_DEFINE_GL_FUNC(DisableVertexAttribArray);
			MAKI_DEFINE_GL_FUNC(EnableVertexAttribArray);
			MAKI_DEFINE_GL_FUNC(GenBuffers);
			MAKI_DEFINE_GL_FUNC(GetProgramInfoLog);
			MAKI_DEFINE_GL_FUNC(GetProgramiv);
			MAKI_DEFINE_GL_FUNC(GetShaderInfoLog);
			MAKI_DEFINE_GL_FUNC(GetShaderiv);
			MAKI_DEFINE_GL_FUNC(GetUniformLocation);
			MAKI_DEFINE_GL_FUNC(LinkProgram);
			MAKI_DEFINE_GL_FUNC(ShaderSource);
			MAKI_DEFINE_GL_FUNC(Uniform1f);
			MAKI_DEFINE_GL_FUNC(Uniform1i);
			MAKI_DEFINE_GL_FUNC(Uniform1ui);
			MAKI_DEFINE_GL_FUNC(Uniform2f);
			MAKI_DEFINE_GL_FUNC(Uniform2i);
			MAKI_DEFINE_GL_FUNC(Uniform2ui);
			MAKI_DEFINE_GL_FUNC(Uniform3f);
			MAKI_DEFINE_GL_FUNC(Uniform3fv);
			MAKI_DEFINE_GL_FUNC(Uniform3i);
			MAKI_DEFINE_GL_FUNC(Uniform3ui);
			MAKI_DEFINE_GL_FUNC(Uniform4f);
			MAKI_DEFINE_GL_FUNC(Uniform4i);
			MAKI_DEFINE_GL_FUNC(Uniform4ui);
			MAKI_DEFINE_GL_FUNC(UniformMatrix3fv);
			MAKI_DEFINE_GL_FUNC(UniformMatrix4fv);
			MAKI_DEFINE_GL_FUNC(UseProgram);
			MAKI_DEFINE_GL_FUNC(VertexAttribPointer);

			loaded = true;
		}


	} // namespace D3D

} // namespace Maki
