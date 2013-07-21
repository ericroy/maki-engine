#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLGPUTypes.h"

namespace Maki
{
	namespace OGL
	{

		void Buffer::DeleteBuffers()
		{
			glDeleteBuffers(2, vbos);
		}




		GPUShader::GPUShader()
			: sh(0), uboPerFrame(0), uboPerObject(0), uboMaterial(0), scratchBuffer(nullptr)
		{
		}

		GPUShader::~GPUShader()
		{
			glDeleteBuffers(1, &uboPerFrame);
			glDeleteBuffers(1, &uboPerObject);
			glDeleteBuffers(1, &uboMaterial);
			glDeleteShader(sh);
			SAFE_FREE(scratchBuffer);
		}




		GPUTexture::GPUTexture()
			: tex(0)
		{
		}

		GPUTexture::~GPUTexture()
		{
			glDeleteTextures(1, &tex);
		}

	} // namespace OGL

} // namespace Maki
