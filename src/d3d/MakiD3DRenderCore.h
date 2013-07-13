#pragma once
#include "d3d/d3d_stdafx.h"

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

	} // namespace Core


	namespace D3D
	{
	
		class D3DRenderCore : public Core::RenderCore
		{
		private:
			static const int32 SHADOW_MAP_SLOT_INDEX_START = 8;

		public:
			D3DRenderCore(Core::Window *window, const Core::Config *config);
			virtual ~D3DRenderCore();

			void *UploadBuffer(void *buffer, Core::VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic);
			void FreeBuffer(void *buffer);
			bool CreateShaderProgram(Core::ShaderProgram *s);
			void DeleteShaderProgram(Core::ShaderProgram *s);
			bool CreateTexture(Core::Texture *t, char *data, uint32 dataLength);
			bool CreateEmptyTexture(Core::Texture *t, uint8 channels);
			bool CreateRenderTarget(Core::Texture *t);
			bool CreateDepthTexture(Core::Texture *t);
			void WriteToTexture(Core::Texture *t, int32 dstX, int32 dstY, int32 srcX, int32 srcY, uint32 srcWidth, uint32 srcHeight, uint32 srcPitch, uint8 channels, char *srcData);
			void DeleteTexture(Core::Texture *t);

		protected:
			void Init();
			void Draw(const Core::RenderState &state, const Core::DrawCommandList &commands);
			void Present();

		private:
			void Resized(uint32 newWidth, uint32 newHeight);
			void SetDepthState(Core::RenderState::DepthTest test, bool write);
			void SetRasterizerState(Core::RenderState::CullMode cullMode, bool wireFrame);
			void SetRenderTargetAndDepthStencil(Core::RenderState::RenderTarget renderTargetType, Handle renderTarget, Core::RenderState::DepthStencil depthStencilType, Handle depthStencil);
			void SetPerFrameConstants(const Core::RenderState &state, const Core::Shader *s, D3D11_MAPPED_SUBRESOURCE &mapped);
			void SetPerObjectConstants(const Core::Shader *s, D3D11_MAPPED_SUBRESOURCE &mapped, const Core::Matrix44 &model, const Core::Matrix44 &modelView, const Core::Matrix44 &modelViewProjection);
			void BindMaterialConstants(const Core::Shader *s, bool isVertexShader, D3D11_MAPPED_SUBRESOURCE &mapped, const Core::Material *mat);
			void BindTextures(const Core::ShaderProgram *shader, const Core::TextureSet *ts);
			bool CreatePixelShader(Core::Shader *ps);
			bool CreateVertexShader(Core::Shader *vs);

		private:
			uint32 windowWidth;
			uint32 windowHeight;

			IDXGISwapChain *swapChain;
			ID3D11Device *device;
			ID3D11DeviceContext *context;
		
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

			void *nullArray[SHADOW_MAP_SLOT_INDEX_START+Core::RenderState::MAX_LIGHTS];
		};


	} // namespace D3D

} // namespace Maki
