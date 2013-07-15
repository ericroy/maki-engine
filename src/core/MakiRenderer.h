#pragma once
#include "core/core_stdafx.h"
#include "core/MakiWindow.h"
#include "core/MakiDrawCommandList.h"
#include "core/MakiRenderCore.h"

namespace Maki
{
	namespace Core
	{

		class ResourceLibrary;
		class Config;
		class VertexFormat;
		class ShaderProgram;
		class Shader;
		class PixelShader;
		class VertexShader;
		class Texture;
		class Light;

		
		

		// The Renderer is the front-end (main thread) counterpart to the back-end (worker thread) RenderCore.
		// It accepts render commands and state changes locally, and then sends them to the RenderCore
		// when you call Submit().
		class Renderer : public Aligned<SIMD_ALIGN>
		{
		public:
			static const int32 MAX_RENDER_PAYLOADS = 16;
			static const int32 DEFAULT_MAX_DRAW_COMMANDS_PER_PASS = 1024;

		public:
			Renderer(Window *window, RenderCore *core, const Config *config);
			virtual ~Renderer();

			// Basic frame lifecycle
			bool Begin();
			inline void Draw(const DrawCommand &drawCommand, const Matrix44 &m);
			void Submit();
			void End();

			// Call this to block until the core has finished all its current tasks.
			// Once this function returns, all the render state objects and draw command
			// lists will once again be in the posession of this renderer object (in the main thread).
			void SyncWithCore();
		
			// State actions
			inline void ClearDepth(float value);
			inline void ClearColor(const Vector4 &rgba);

			// State setters
			void SetRenderTargetAndDepthStencil(RenderState::RenderTarget renderTargetType, Handle renderTarget, RenderState::DepthStencil depthStencilType, Handle depthStencil);
			inline void SetViewport(const Rect &rect);
			inline void SetView(const Matrix44 &view);
			inline void SetCameraMatrix(const Matrix44 &cameraMatrix);
			void SetPerspectiveProjection(const Frustum &frustum);
			void SetOrthoProjection(const Frustum &frustum);
			inline void SetCullMode(RenderState::CullMode mode);
			inline void SetWireFrameEnabled(bool on);
			inline void SetDepthWriteEnabled(bool on);
			inline void SetDepthTest(RenderState::DepthTest test);
			inline void SetGlobalAmbientColor(const Vector4 &color);
			inline void SetShaderVariant(ShaderProgram::Variant variant);
	
			inline void SetLightCount(uint32 count, uint32 shadowCount = 0, uint32 splitShadowCount = 0);
			void SetLight(uint32 lightIndex, const RenderState::LightProperties *props = nullptr, const RenderState::ShadowMapProperties *shadProps = nullptr, const Matrix44 *matrix = nullptr, float fov = 0.0f, Handle depthBuffer = HANDLE_NONE);
			void SetLightCascade(uint32 lightIndex, uint32 cascadeIndex, const Frustum &frustum);
			inline void SetCameraSplitDistances(uint32 count, float *splitDistances);

			// Getters
			inline Window *GetWindow() const;
			inline RenderState::CullMode GetCullMode() const;
			inline bool GetWireframeEnabled() const;
			inline bool GetDepthWriteEnabled() const;
			inline RenderState::DepthTest GetDepthTest() const;
			inline const Vector4 &GetGlobalAmbientColor() const;
			inline ShaderProgram::Variant GetShaderVariant() const;
			inline const Matrix44 &GetView() const;
			inline const Matrix44 &GetCameraMatrix() const;

			// GPU resource creation, updates, destruction
			// These actions are applied synchonously on the core, so they involve acquiring a mutex
			inline void *UploadBuffer(void *buffer, VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic);
			inline void FreeBuffer(void *buffer);
			inline bool CreateShaderProgram(ShaderProgram *s);
			inline void DeleteShaderProgram(ShaderProgram *s);
			inline bool CreateTexture(Texture *t, char *data, uint32 dataLength);
			inline bool CreateEmptyTexture(Texture *t, uint8 channels);
			inline bool CreateRenderTarget(Texture *t);
			inline bool CreateDepthTexture(Texture *t);
			inline void DeleteTexture(Texture *t);
			inline void WriteToTexture(Texture *t, int32 dstX, int32 dstY, int32 srcX, int32 srcY, uint32 srcWidth, uint32 srcHeight, uint32 srcPitch, uint8 channels, char *srcData);
		

		private:
			void PrepareNextRenderState();

		protected:
			Window *window;
			RenderState current;
			uint32 lightDirtyFlags;
			RenderState *state;
			DrawCommandList *commands;
			std::vector<RenderState *> renderStates;
			std::vector<DrawCommandList *> commandLists;
			RenderCore *core;

		private:
			// Memebers that are used to provide convenience functions but are not part of the renderstate
			Matrix44 cameraMatrix;
		};






		inline void Renderer::Draw(const DrawCommand &drawCommand, const Matrix44 &m)
		{
			commands->Add(drawCommand, m);
		}
		
		inline void Renderer::ClearDepth(float value)
		{
			current.clearDepthStencil = true; current.depthClearValue = value;
		}
	
		inline void Renderer::ClearColor(const Vector4 &rgba)
		{
			current.clearRenderTarget = true;
			current.renderTargetClearValue = rgba;
		}

		inline void Renderer::SetViewport(const Rect &rect)
		{
			current.viewPortRect = rect;
		}

		inline void Renderer::SetView(const Matrix44 &view)
		{
			current.view = view;
			Matrix44::AffineInverse(view, cameraMatrix);
		}

		inline void Renderer::SetCameraMatrix(const Matrix44 &cameraMatrix)
		{
			this->cameraMatrix = cameraMatrix;
			Matrix44::AffineInverse(cameraMatrix, current.view);
		}

		inline void Renderer::SetLightCount(uint32 lightCount, uint32 shadowLightCount, uint32 cascadedShadowLightCount)
		{
			assert(lightCount <= RenderState::MAX_LIGHTS);
			assert(shadowLightCount <= lightCount);
			assert(cascadedShadowLightCount <= shadowLightCount);

			current.lightCount = lightCount;
			current.shadowLightCount = shadowLightCount;
			current.cascadedShadowLightCount = cascadedShadowLightCount;
		}

		inline void Renderer::SetCameraSplitDistances(uint32 splitCount, float *splitDistances)
		{
			assert(splitCount < RenderState::MAX_CASCADES);
			memcpy(current.cameraSplitDistances.splits, splitDistances, splitCount*sizeof(float));
		}

		inline void Renderer::SetCullMode(RenderState::CullMode mode)
		{
			current.cullMode = mode;
		}

		inline void Renderer::SetWireFrameEnabled(bool on)
		{
			current.wireFrame = on;
		}

		inline void Renderer::SetDepthWriteEnabled(bool on)
		{
			current.depthWrite = on;
		}

		inline void Renderer::SetDepthTest(RenderState::DepthTest test)
		{
			current.depthTest = test;
		}

		inline void Renderer::SetGlobalAmbientColor(const Vector4 &color)
		{
			current.globalAmbientColor = color;
		}

		inline void Renderer::SetShaderVariant(ShaderProgram::Variant variant)
		{
			current.shaderVariant = variant;
		}
	




		inline Window *Renderer::GetWindow() const
		{
			return window;
		}

		inline RenderState::CullMode Renderer::GetCullMode() const
		{
			return current.cullMode;
		}
	
		inline bool Renderer::GetWireframeEnabled() const
		{
			return current.wireFrame;
		}

		inline bool Renderer::GetDepthWriteEnabled() const
		{
			return current.depthWrite;
		}

		inline RenderState::DepthTest Renderer::GetDepthTest() const
		{
			return current.depthTest;
		}

		inline const Vector4 &Renderer::GetGlobalAmbientColor() const
		{
			return current.globalAmbientColor;
		}

		inline ShaderProgram::Variant Renderer::GetShaderVariant() const
		{
			return current.shaderVariant;
		}

		inline const Matrix44 &Renderer::GetView() const
		{
			return current.view;
		}

		inline const Matrix44 &Renderer::GetCameraMatrix() const
		{
			return cameraMatrix;
		}



		inline void *Renderer::UploadBuffer(void *buffer, VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic)
		{
			return core->UploadBuffer(buffer, vf, vertexData, vertexCount, indexData, faceCount, indicesPerFace, bytesPerIndex, dynamic);
		}
	
		inline void Renderer::FreeBuffer(void *buffer)
		{
			core->FreeBuffer(buffer);
		}

		inline bool Renderer::CreateShaderProgram(ShaderProgram *s)
		{
			return core->CreateShaderProgram(s);
		}
	
		inline void Renderer::DeleteShaderProgram(ShaderProgram *s)
		{
			core->DeleteShaderProgram(s);
		}

		inline bool Renderer::CreateTexture(Texture *t, char *data, uint32 dataLength)
		{
			return core->CreateTexture(t, data, dataLength);
		}

		inline bool Renderer::CreateEmptyTexture(Texture *t, uint8 channels)
		{
			return core->CreateEmptyTexture(t, channels);
		}

		inline bool Renderer::CreateRenderTarget(Texture *t)
		{
			return core->CreateRenderTarget(t);
		}

		inline bool Renderer::CreateDepthTexture(Texture *t)
		{
			return core->CreateDepthTexture(t);
		}

		inline void Renderer::DeleteTexture(Texture *t)
		{
			core->DeleteTexture(t);
		}

		inline void Renderer::WriteToTexture(Texture *t, int32 dstX, int32 dstY, int32 srcX, int32 srcY, uint32 srcWidth, uint32 srcHeight, uint32 srcPitch, uint8 channels, char *srcData)
		{
			core->WriteToTexture(t, dstX, dstY, srcX, srcY, srcWidth, srcHeight, srcPitch, channels, srcData);
		}

	} // namespace Core

} // namespace Maki