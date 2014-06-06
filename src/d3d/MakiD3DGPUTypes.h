#pragma once
#include <d3d11.h>

namespace Maki
{
	namespace Core
	{
		class VertexFormat;

	} // namespace Core

	namespace D3D
	{

		struct Buffer
		{
			ID3D11Buffer *vbos[2];
			uint32 vertexCount;
			uint32 faceCount;
			uint32 indicesPerFace;
			uint32 bytesPerIndex;
			int32 indexDataType;
			int32 geometryType;
		};

		struct GPUVertexShader
		{
		private:
			struct InputLayout
			{
				uint64 vertexFormatKey;
				ID3D11InputLayout *inputLayout;
			};

		public:
			GPUVertexShader(uint32 vertexFormatsPerVertexShader);
			~GPUVertexShader();
			ID3D11InputLayout *GetOrCreateInputLayout(ID3D11Device *device, const Core::VertexFormat *vf);
		
			
			ID3D11VertexShader *vs;
			ID3D11Buffer *perFrameConstants;
			ID3D11Buffer *perObjectConstants;
			ID3D11Buffer *materialConstants;
			InputLayout *inputLayouts;
			uint32 inputLayoutCapacity;
			uint32 inputLayoutCount;
			char *blob;
			uint32 blobSize;
		};

		struct GPUPixelShader
		{
			GPUPixelShader();
			~GPUPixelShader();
			
			ID3D11PixelShader *ps;
			ID3D11Buffer *perFrameConstants;
			ID3D11Buffer *perObjectConstants;
			ID3D11Buffer *materialConstants;
		};

		struct GPUTexture
		{
			GPUTexture();
			~GPUTexture();

			ID3D11ShaderResourceView *shaderResourceView;
			ID3D11SamplerState *samplerState;

			// Only set if this was created as a depth target
			ID3D11DepthStencilView *depthStencilView;

			// Only set if this was created as a render target
			ID3D11RenderTargetView *renderTargetView;
		};

	} // namespace D3D

} // namespace Maki