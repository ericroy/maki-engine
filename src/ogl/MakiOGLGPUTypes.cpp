#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLGPUTypes.h"

namespace Maki
{
	namespace OGL
	{

		void Buffer::DeleteBuffers()
		{
			if(vbos[0] != 0) {
				glDeleteBuffers(1, &vbos[0]);
			}
			if(vbos[1] != 0) {
				glDeleteBuffers(1, &vbos[1]);
			}
		}




		GPUShader::GPUShader()
			: sh(0), uboPerFrame(0), uboPerObject(0), uboMaterial(0), scratchBuffer(nullptr)
		{
		}

		GPUShader::~GPUShader()
		{
			if(uboPerFrame != 0) {
				glDeleteBuffers(1, &uboPerFrame);
			}
			if(uboPerObject != 0) {
				glDeleteBuffers(1, &uboPerObject);
			}
			if(uboMaterial != 0) {
				glDeleteBuffers(1, &uboMaterial);
			}
			if(sh != 0) {
				glDeleteShader(sh);
			}
			SAFE_FREE(scratchBuffer);
		}




		GPUTexture::GPUTexture()
			: tex(0)
		{
		}

		GPUTexture::~GPUTexture()
		{
			if(tex != 0) {
				glDeleteTextures(1, &tex);
			}
		}

	} // namespace OGL

} // namespace Maki
