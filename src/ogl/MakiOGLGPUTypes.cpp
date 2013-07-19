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
			: vs(0)
		{
		}

		GPUVertexShader::~GPUVertexShader()
		{
			if(vs != 0) {
				glDeleteShader(vs);
			}
		}





		GPUPixelShader::GPUPixelShader()
			: ps(0)
		{
		}

		GPUPixelShader::~GPUPixelShader()
		{
			if(ps != 0) {
				glDeleteShader(ps);
			}
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
