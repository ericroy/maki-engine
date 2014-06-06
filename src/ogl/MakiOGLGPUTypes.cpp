#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLGPUTypes.h"

namespace Maki
{
	namespace OGL
	{

		void Buffer::DeleteBuffers()
		{
			glDeleteBuffers(2, vbos);
			vbos[0] = 0;
			vbos[1] = 0;
		}


		GPUShader::GPUShader() : sh(0) {}
		GPUShader::~GPUShader() { glDeleteShader(sh); }


		GPUVertexShader::GPUVertexShader()
			: GPUShader(), uboPerFrame(0), uboPerObject(0), uboMaterial(0), scratchBuffer(nullptr)
		{
			memset(textureSamplerLocations, 0xff, sizeof(textureSamplerLocations));
		}

		GPUVertexShader::~GPUVertexShader()
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
