#pragma once
#include "d3d/d3d_stdafx.h"
#include "d3d/MakiD3DCommon.h"
#include "d3d/MakiD3DGPUTypes.h"

#pragma comment(lib, "MakiCore.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

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
			void Init();
			inline void Draw(const Core::RenderState &state, const Core::DrawCommandList &commands) { GenericDraw<D3DRenderCore>(state, commands); }
			void Present();
			void Resized(uint32 width, uint32 height);

			// GPU resource creation / destruction
			void *UploadBuffer(void *buffer, Core::VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic, bool lengthChanged);
			void FreeBuffer(void *buffer);
			bool CreateShaderProgram(Core::ShaderProgram *s);
			void DeleteShaderProgram(Core::ShaderProgram *s);
			bool CreateTexture(Core::Texture *t, char *data, uint32 dataLength);
			bool CreateEmptyTexture(Core::Texture *t, uint8 channels);
			bool CreateRenderTarget(Core::Texture *t);
			bool CreateDepthTexture(Core::Texture *t);
			void WriteToTexture(Core::Texture *t, int32 dstX, int32 dstY, int32 srcX, int32 srcY, uint32 srcWidth, uint32 srcHeight, uint32 srcPitch, uint8 channels, char *srcData);
			void DeleteTexture(Core::Texture *t);

			// Non-virtual interface
			inline void AcquireContext();
			inline void ReleaseContext();
			inline void SetRenderTargetAndDepthStencil(Core::RenderState::RenderTarget renderTargetType, Handle renderTarget, Core::RenderState::DepthStencil depthStencilType, Handle depthStencil);
			inline void SetViewport(const Core::Rect &viewPortRect);
			inline void Clear(bool clearRenderTarget, const float clearColorValues[4], bool clearDepthStencil,	float clearDepthValue);
			inline void SetDepthState(Core::RenderState::DepthTest depthTest, bool depthWrite);
			inline void SetRasterizerState(Core::RenderState::CullMode cullMode, bool wireFrame);
			inline void SetBlendState(bool enabled);
			inline void UnbindAllTextures();
			inline void BindShaders(const Core::ShaderProgram *shader);
			inline void SetPerFrameVertexShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader);
			inline void SetPerFramePixelShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader);
			inline void BindShadowMaps(const Core::ShaderProgram *shader, const Core::RenderState &state);
			inline void SetInputLayout(const Core::ShaderProgram *shader, const Core::VertexFormat *vf);
			inline void SetMaterialVertexShaderConstants(const Core::ShaderProgram *shader, const Core::Material *mat);
			inline void SetMaterialPixelShaderConstants(const Core::ShaderProgram *shader, const Core::Material *mat);
			inline void BindTextures(const Core::ShaderProgram *shader, const Core::TextureSet *ts);
			inline void SetPerObjectVertexShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader, const Core::Matrix44 &matrix, const Core::Matrix44 &mv, const Core::Matrix44 &mvp);
			inline void SetPerObjectPixelShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader, const Core::Matrix44 &matrix, const Core::Matrix44 &mv, const Core::Matrix44 &mvp);
			inline void BindBuffer(void *buffer, const Core::VertexFormat *vf);
			inline void DrawBuffer(void *buffer);

		private:
			bool CreatePixelShader(Core::Shader *ps);
			bool CreateVertexShader(Core::Shader *vs);

		private:
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
			std::mutex mutex;
		};






		inline void D3DRenderCore::AcquireContext()
		{
			mutex.lock();
		}

		inline void D3DRenderCore::ReleaseContext()
		{
			mutex.unlock();
		}

		inline void D3DRenderCore::SetRenderTargetAndDepthStencil(Core::RenderState::RenderTarget renderTargetType, Handle renderTarget, Core::RenderState::DepthStencil depthStencilType, Handle depthStencil)
		{
			using namespace Core;

			currentRenderTargetView = nullptr;
			if(renderTargetType == RenderState::RenderTarget_Default) {
				currentRenderTargetView = defaultRenderTargetView;
			} else if(renderTargetType == RenderState::RenderTarget_Custom) {
				currentRenderTargetView = ((GPUTexture *)TextureManager::Get(renderTarget)->handle)->renderTargetView;
				if(currentRenderTargetView == nullptr) {
					Console::Error("Tried to set render target to an invalid texture");
				}
			}

			currentDepthStencilView = nullptr;
			if(depthStencilType == RenderState::DepthStencil_Default) {
				currentDepthStencilView = defaultDepthStencilView;
			} else if(depthStencilType == RenderState::DepthStencil_Custom) {
				currentDepthStencilView = ((GPUTexture *)TextureManager::Get(depthStencil)->handle)->depthStencilView;
				if(currentDepthStencilView == nullptr) {
					Console::Error("Tried to set depth stencil to an invalid texture");
				}
			}

			context->OMSetRenderTargets(1, &currentRenderTargetView, currentDepthStencilView);
		}

		inline void D3DRenderCore::SetViewport(const Core::Rect &viewPortRect)
		{
			D3D11_VIEWPORT viewport;
			ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
			viewport.TopLeftX = viewPortRect.left;
			viewport.TopLeftY = viewPortRect.top;
			viewport.Width = viewPortRect.GetWidth();
			viewport.Height = viewPortRect.GetHeight();
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			context->RSSetViewports(1, &viewport);
		}

		inline void D3DRenderCore::Clear(bool clearRenderTarget, const float clearColorValues[4], bool clearDepthStencil, float clearDepthValue)
		{
			if(clearRenderTarget && currentRenderTargetView != nullptr) {
				context->ClearRenderTargetView(currentRenderTargetView, clearColorValues);
			}
			if(clearDepthStencil && currentDepthStencilView != nullptr) {
				context->ClearDepthStencilView(currentDepthStencilView, D3D11_CLEAR_DEPTH, clearDepthValue, 0);
			}
		}

		inline void D3DRenderCore::SetDepthState(Core::RenderState::DepthTest depthTest, bool depthWrite)
		{
			using namespace Core;

			switch(depthTest) {
			case RenderState::DepthTest_Less:
				context->OMSetDepthStencilState(depthWrite ? depthStateLessWrite : depthStateLess, 1);
				break;
			case RenderState::DepthTest_Equal:
				context->OMSetDepthStencilState(depthWrite ? depthStateEqualWrite : depthStateEqual, 1);
				break;
			case RenderState::DepthTest_LessEqual:
				context->OMSetDepthStencilState(depthWrite ? depthStateLessEqualWrite : depthStateLessEqual, 1);
				break;
			case RenderState::DepthTest_Disabled:
			default:
				context->OMSetDepthStencilState(depthWrite ? depthStateWrite : depthState, 1);
				break;
			}
		}

		inline void D3DRenderCore::SetRasterizerState(Core::RenderState::CullMode cullMode, bool wireFrame)
		{
			using namespace Core;

			switch(cullMode) {
			case RenderState::CullMode_Front:
				context->RSSetState(wireFrame ? rasterizerStateWireFrameCullFront : rasterizerStateCullFront);
				break;
			case RenderState::CullMode_Back:
				context->RSSetState(wireFrame ? rasterizerStateWireFrameCullBack : rasterizerStateCullBack);
				break;
			case RenderState::CullMode_None:
			default:
				context->RSSetState(wireFrame ? rasterizerStateWireFrame : rasterizerState);
				break;
			}
		}

		inline void D3DRenderCore::SetBlendState(bool enabled)
		{
			context->OMSetBlendState(enabled ? blendEnabled : blendDisabled, nullptr, 0xffffffff);
		}

		inline void D3DRenderCore::UnbindAllTextures()
		{
			context->PSSetShaderResources(0, SHADOW_MAP_SLOT_INDEX_START+Core::RenderState::MAX_SHADOW_LIGHTS, (ID3D11ShaderResourceView **)nullArray);
		}

		inline void D3DRenderCore::BindShaders(const Core::ShaderProgram *shader)
		{
			if(shader != nullptr) {
				context->VSSetShader(((GPUVertexShader *)shader->vertexShader.handle)->vs, nullptr, 0);
				context->PSSetShader(((GPUPixelShader *)shader->pixelShader.handle)->ps, nullptr, 0);
			} else {
				context->VSSetShader(nullptr, nullptr, 0);
				context->PSSetShader(nullptr, nullptr, 0);
			}
		}

		inline void D3DRenderCore::SetPerFrameVertexShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader)
		{
			D3D11_MAPPED_SUBRESOURCE mapped;
			const GPUVertexShader *gvs = (GPUVertexShader *)shader->vertexShader.handle;
			context->Map(gvs->perFrameConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			SetPerFrameConstants(state, &shader->vertexShader, (char *)mapped.pData);
			context->Unmap(gvs->perFrameConstants, 0);
			context->VSSetConstantBuffers(shader->vertexShader.frameUniformBufferLocation, 1, &gvs->perFrameConstants);
		}

		inline void D3DRenderCore::SetPerFramePixelShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader)
		{
			D3D11_MAPPED_SUBRESOURCE mapped;
			const GPUPixelShader *gps = (GPUPixelShader *)shader->pixelShader.handle;
			context->Map(gps->perFrameConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			SetPerFrameConstants(state, &shader->pixelShader, (char *)mapped.pData);
			context->Unmap(gps->perFrameConstants, 0);
			context->PSSetConstantBuffers(shader->pixelShader.frameUniformBufferLocation, 1, &gps->perFrameConstants);
		}

		inline void D3DRenderCore::BindShadowMaps(const Core::ShaderProgram *shader, const Core::RenderState &state)
		{
			using namespace Core;

			ID3D11ShaderResourceView *shadowViews[RenderState::MAX_SHADOW_LIGHTS];
			ID3D11SamplerState *shadowSamplers[RenderState::MAX_SHADOW_LIGHTS];
			for(uint8 i = 0; i < RenderState::MAX_SHADOW_LIGHTS; i++) {
				if(state.shadowMaps[i] != HANDLE_NONE) {
					GPUTexture *gtex = (GPUTexture *)TextureManager::Get(state.shadowMaps[i])->handle;
					shadowViews[i] = gtex->shaderResourceView;
					shadowSamplers[i] = gtex->samplerState;
				} else {
					shadowViews[i] = nullptr;
					shadowSamplers[i] = nullptr;
				}
			}
			context->PSSetShaderResources(SHADOW_MAP_SLOT_INDEX_START, RenderState::MAX_SHADOW_LIGHTS, shadowViews);
			context->PSSetSamplers(SHADOW_MAP_SLOT_INDEX_START, RenderState::MAX_SHADOW_LIGHTS, shadowSamplers);
		}

		inline void D3DRenderCore::SetInputLayout(const Core::ShaderProgram *shader, const Core::VertexFormat *vf)
		{
			ID3D11InputLayout *layout = ((GPUVertexShader *)shader->vertexShader.handle)->GetOrCreateInputLayout(device, vf);
			context->IASetInputLayout(layout);
		}

		inline void D3DRenderCore::SetMaterialVertexShaderConstants(const Core::ShaderProgram *shader, const Core::Material *mat)
		{
			D3D11_MAPPED_SUBRESOURCE mapped;
			const GPUVertexShader *gvs = (GPUVertexShader *)shader->vertexShader.handle;
			context->Map(gvs->materialConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			BindMaterialConstants(&shader->vertexShader, true, (char *)mapped.pData, mat);
			context->Unmap(gvs->materialConstants, 0);
			context->VSSetConstantBuffers(shader->vertexShader.materialUniformBufferLocation, 1, &gvs->materialConstants);
		}

		inline void D3DRenderCore::SetMaterialPixelShaderConstants(const Core::ShaderProgram *shader, const Core::Material *mat)
		{
			D3D11_MAPPED_SUBRESOURCE mapped;
			const GPUPixelShader *gps = (GPUPixelShader *)shader->pixelShader.handle;
			context->Map(gps->materialConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			BindMaterialConstants(&shader->pixelShader, false, (char *)mapped.pData, mat);
			context->Unmap(gps->materialConstants, 0);
			context->PSSetConstantBuffers(shader->pixelShader.materialUniformBufferLocation, 1, &gps->materialConstants);
		}

		inline void D3DRenderCore::BindTextures(const Core::ShaderProgram *shader, const Core::TextureSet *ts)
		{
			using namespace Core;

			ID3D11ShaderResourceView *views[TextureSet::MAX_TEXTURES_PER_SET];
			ID3D11SamplerState *samplers[TextureSet::MAX_TEXTURES_PER_SET];

			for(uint8 i = 0; i < ts->textureCount; i++) {
				const GPUTexture *tex = (GPUTexture *)TextureManager::Get(ts->textures[i])->handle;
				views[i] = tex->shaderResourceView;
				samplers[i] = tex->samplerState;
			}

			context->PSSetShaderResources(0, ts->textureCount, views);
			context->PSSetSamplers(0, ts->textureCount, samplers);
		}

		inline void D3DRenderCore::SetPerObjectVertexShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader, const Core::Matrix44 &matrix, const Core::Matrix44 &mv, const Core::Matrix44 &mvp)
		{
			D3D11_MAPPED_SUBRESOURCE mapped;
			const GPUVertexShader *gvs = (GPUVertexShader *)shader->vertexShader.handle;
			context->Map(gvs->perObjectConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			SetPerObjectConstants(&shader->vertexShader, (char *)mapped.pData, matrix, mv, mvp);
			context->Unmap(gvs->perObjectConstants, 0);
			context->VSSetConstantBuffers(shader->vertexShader.objectUniformBufferLocation, 1, &gvs->perObjectConstants);
		}

		inline void D3DRenderCore::SetPerObjectPixelShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader, const Core::Matrix44 &matrix, const Core::Matrix44 &mv, const Core::Matrix44 &mvp)
		{
			D3D11_MAPPED_SUBRESOURCE mapped;
			const GPUPixelShader *gps = (GPUPixelShader *)shader->pixelShader.handle;
			context->Map(gps->perObjectConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			SetPerObjectConstants(&shader->pixelShader, (char *)mapped.pData, matrix, mv, mvp);
			context->Unmap(gps->perObjectConstants, 0);
			context->PSSetConstantBuffers(shader->pixelShader.objectUniformBufferLocation, 1, &gps->perObjectConstants);
		}

		inline void D3DRenderCore::BindBuffer(void *buffer, const Core::VertexFormat *vf)
		{
			if(buffer == nullptr) {
				return;
			}

			const Buffer *b = (Buffer *)buffer;
			uint32 stride = vf->GetStride();
			uint32 offset = 0;
			context->IASetVertexBuffers(0, 1, &b->vbos[0], &stride, &offset);
			context->IASetIndexBuffer(b->vbos[1], (DXGI_FORMAT)b->indexDataType, 0);
		}

		inline void D3DRenderCore::DrawBuffer(void *buffer)
		{
			const Buffer *b = (Buffer *)buffer;
			context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)b->geometryType);
			context->DrawIndexed(b->indicesPerFace*b->faceCount, 0, 0);
		}

		// End of non-virtual interface


	} // namespace D3D

} // namespace Maki
