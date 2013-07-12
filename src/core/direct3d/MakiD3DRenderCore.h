#pragma once
#include "core/core_stdafx.h"

#if MAKI_USE_D3D

#include "core/MakiRenderCore.h"

#include <windows.h>
#include <d3d11.h>

namespace Maki
{
	namespace Core
	{
		
		class Window;
		class Config;
		class VertexFormat;
		class ShaderProgram;
		class TextureSet;
		class TextureManager;
		class Material;
		class Texture;

		namespace D3D
		{
	
			class D3DRenderCore : public RenderCore
			{
			private:
				static const int32 SHADOW_MAP_SLOT_INDEX_START = 8;

			public:
				D3DRenderCore(Window *window, const Config *config);
				virtual ~D3DRenderCore();

				void *UploadBuffer(void *buffer, VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic);
				void FreeBuffer(void *buffer);
				bool CreateShaderProgram(ShaderProgram *s);
				void DeleteShaderProgram(ShaderProgram *s);
				bool CreateTexture(Texture *t, char *data, uint32 dataLength);
				bool CreateEmptyTexture(Texture *t, uint8 channels);
				bool CreateRenderTarget(Texture *t);
				bool CreateDepthTexture(Texture *t);
				void WriteToTexture(Texture *t, int32 dstX, int32 dstY, int32 srcX, int32 srcY, uint32 srcWidth, uint32 srcHeight, uint32 srcPitch, uint8 channels, char *srcData);
				void DeleteTexture(Texture *t);

			protected:
				void Init();
				void Draw(const RenderState &state, const DrawCommandList &commands);
				void Present();

			private:
				void Resized(uint32 newWidth, uint32 newHeight);
				void SetDepthState(RenderState::DepthTest test, bool write);
				void SetRasterizerState(RenderState::CullMode cullMode, bool wireFrame);
				void SetRenderTargetAndDepthStencil(RenderState::RenderTarget renderTargetType, Handle renderTarget, RenderState::DepthStencil depthStencilType, Handle depthStencil);
				void SetPerFrameConstants(const RenderState &state, const Shader *s, D3D11_MAPPED_SUBRESOURCE &mapped);
				void SetPerObjectConstants(const Shader *s, D3D11_MAPPED_SUBRESOURCE &mapped, const Matrix44 &model, const Matrix44 &modelView, const Matrix44 &modelViewProjection);
				void BindMaterialConstants(const Shader *s, bool isVertexShader, D3D11_MAPPED_SUBRESOURCE &mapped, const Material *mat);
				void BindTextures(const ShaderProgram *shader, const TextureSet *ts);
				bool CreatePixelShader(Shader *ps);
				bool CreateVertexShader(Shader *vs);

			private:
				uint32 windowWidth;
				uint32 windowHeight;

				IDXGISwapChain *swapChain;		// the pointer to the swap chain interface
				ID3D11Device *device;			// the pointer to our Direct3D device interface
				ID3D11DeviceContext *context;	// the pointer to our Direct3D device context
		
				ID3D11RenderTargetView *defaultRenderTargetView;
				ID3D11DepthStencilView *defaultDepthStencilView;

				ID3D11RenderTargetView *currentRenderTargetView;
				ID3D11DepthStencilView *currentDepthStencilView;

				ID3D11RasterizerState *rasterizerState;
				ID3D11RasterizerState *rasterizerStateCullBack;
				ID3D11RasterizerState *rasterizerStateCullFront;
				ID3D11RasterizerState *rasterizerStateWireFrame;
				ID3D11RasterizerState *rasterizerStateWireFrameCullBack;
				ID3D11RasterizerState *rasterizerStateWireFrameCullFront;

				ID3D11DepthStencilState *depthState;
				ID3D11DepthStencilState *depthStateWrite;
				ID3D11DepthStencilState *depthStateLess;
				ID3D11DepthStencilState *depthStateLessWrite;
				ID3D11DepthStencilState *depthStateEqual;
				ID3D11DepthStencilState *depthStateEqualWrite;
				ID3D11DepthStencilState *depthStateLessEqual;
				ID3D11DepthStencilState *depthStateLessEqualWrite;

				ID3D11BlendState *blendEnabled;
				ID3D11BlendState *blendDisabled;

				bool vsync;
				uint32 maxVertexFormatsPerVertexShader;

				void *nullArray[SHADOW_MAP_SLOT_INDEX_START+RenderState::MAX_LIGHTS];
			};

		} // namespace D3D
	
	} // namespace Core

} // namespace Maki


#endif