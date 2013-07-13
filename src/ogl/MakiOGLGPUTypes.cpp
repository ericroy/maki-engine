#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLGPUTypes.h"

namespace Maki
{
	namespace OGL
	{

		GPUVertexShader::GPUVertexShader(uint32 vertexFormatsPerVertexShader)
			: blob(nullptr),
			blobSize(0),
			vs(0)
			//perFrameConstants(nullptr),
			//perObjectConstants(nullptr),
			//materialConstants(nullptr)
		{
		}

		GPUVertexShader::~GPUVertexShader()
		{
			if(vs != 0) {
				glDeleteShader(vs);
			}
			SAFE_FREE(blob);
			//SAFE_RELEASE(perFrameConstants);
			//SAFE_RELEASE(perObjectConstants);
			//SAFE_RELEASE(materialConstants);
		}





		GPUPixelShader::GPUPixelShader()
			: ps(0)
			//perFrameConstants(nullptr),
			//perObjectConstants(nullptr),
			//materialConstants(nullptr)
		{
		}

		GPUPixelShader::~GPUPixelShader()
		{
			if(ps != 0) {
				glDeleteShader(ps);
			}
			//SAFE_RELEASE(perFrameConstants);
			//SAFE_RELEASE(perObjectConstants);
			//SAFE_RELEASE(materialConstants);
		}
			




		GPUTexture::GPUTexture()
			//: shaderResourceView(nullptr),
			//samplerState(nullptr),
			//depthStencilView(nullptr),
			//renderTargetView(nullptr)
		{
		}

		GPUTexture::~GPUTexture()
		{
			//SAFE_RELEASE(shaderResourceView);
			//SAFE_RELEASE(samplerState);
			//SAFE_RELEASE(depthStencilView);
			//SAFE_RELEASE(renderTargetView);
		}

	} // namespace OGL

} // namespace Maki
