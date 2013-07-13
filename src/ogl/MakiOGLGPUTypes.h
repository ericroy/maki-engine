#pragma once
#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLCommon.h"

namespace Maki
{
	namespace Core
	{
		class VertexFormat;

	} // namespace Core

	namespace OGL
	{

		struct Buffer
		{
			GLuint vbos[2];
			uint32 vertexCount;
			uint32 faceCount;
			uint32 indicesPerFace;
			uint32 bytesPerIndex;
			GLenum indexDataType;
			GLenum geometryType;
		};

		struct GPUVertexShader
		{
		public:
			GPUVertexShader(uint32 vertexFormatsPerVertexShader);
			~GPUVertexShader();

			GLuint vs;
			/*ID3D11Buffer *perFrameConstants;
			ID3D11Buffer *perObjectConstants;
			ID3D11Buffer *materialConstants;*/
			char *blob;
			uint32 blobSize;
		};

		struct GPUPixelShader
		{
			GPUPixelShader();
			~GPUPixelShader();
			
			GLuint ps;
			/*ID3D11Buffer *perFrameConstants;
			ID3D11Buffer *perObjectConstants;
			ID3D11Buffer *materialConstants;*/
		};

		struct GPUTexture
		{
			GPUTexture();
			~GPUTexture();

			//ID3D11ShaderResourceView *shaderResourceView;
			//ID3D11SamplerState *samplerState;

			// Only set if this was created as a depth target
			//ID3D11DepthStencilView *depthStencilView;

			// Only set if this was created as a render target
			//ID3D11RenderTargetView *renderTargetView;
		};

	} // namespace OGL

} // namespace Maki
