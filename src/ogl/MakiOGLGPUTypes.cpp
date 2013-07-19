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




		GPUVertexShader::GPUVertexShader()
			: vs(0), uboPerFrame(0), uboPerObject(0), uboMaterial(0), scratchBuffer(nullptr)
		{
		}

		GPUVertexShader::~GPUVertexShader()
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
			if(vs != 0) {
				glDeleteShader(vs);
			}
			SAFE_FREE(scratchBuffer);
		}





		GPUPixelShader::GPUPixelShader()
			: ps(0), uboPerFrame(0), uboPerObject(0), uboMaterial(0), scratchBuffer(nullptr)
		{
		}

		GPUPixelShader::~GPUPixelShader()
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
			if(ps != 0) {
				glDeleteShader(ps);
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
