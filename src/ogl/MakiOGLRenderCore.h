#pragma once
#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLCommon.h"
#include "ogl/MakiOGLGPUTypes.h"

#pragma comment(lib, "MakiCore.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

namespace Maki
{
	namespace Core
	{
		
		class Window;
		class InputLayoutCache;
		class VertexFormat;
		class ShaderProgram;
		class TextureSet;
		class TextureManager;
		class Material;
		class Texture;
		class Config;

	} // namespace Core

	namespace OGL
	{


		class OGLRenderCore : public Core::RenderCore
		{
		private:
			static const int32 SHADOW_MAP_SLOT_INDEX_START = 8;

		public:
			OGLRenderCore(Core::Window *window, const Core::Config *config);
			virtual ~OGLRenderCore();
			void Init();
			inline void Draw(const Core::RenderState &state, const Core::DrawCommandList &commands) {
				GenericDraw<OGLRenderCore>(state, commands);
			}
			void Present();
			void Resized(uint32 width, uint32 height);

			// GPU resource creation / destruction
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
			inline void BindShadowMaps(const Core::RenderState &state);
			inline void SetInputLayout(const Core::ShaderProgram *shader, const Core::VertexFormat *vf);
			inline void SetMaterialVertexShaderConstants(const Core::ShaderProgram *shader, const Core::Material *mat);
			inline void SetMaterialPixelShaderConstants(const Core::ShaderProgram *shader, const Core::Material *mat);
			inline void BindTextures(const Core::TextureSet *ts);
			inline void SetPerObjectVertexShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader, const Core::Matrix44 &matrix, const Core::Matrix44 &mv, const Core::Matrix44 &mvp);
			inline void SetPerObjectPixelShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader, const Core::Matrix44 &matrix, const Core::Matrix44 &mv, const Core::Matrix44 &mvp);
			inline void BindBuffer(void *buffer, const Core::VertexFormat *vf);
			inline void DrawBuffer(void *buffer);

		private:
			bool CreatePixelShader(Core::Shader *ps);
			bool CreateVertexShader(Core::Shader *vs);

		private:
			Core::Window *window;
			SDL_GLContext renderThreadContext;
			SDL_GLContext mainThreadContext;

			uint32 windowWidth;
			uint32 windowHeight;

			GLuint frameBuffer;

			GLuint defaultRenderTarget;
			GLuint defaultDepthStencil;

			GLuint currentRenderTarget;
			GLuint currentDepthStencil;

			bool vsync;
			std::mutex mutex;
		};







		inline void OGLRenderCore::AcquireContext()
		{
			mutex.lock();
		}

		inline void OGLRenderCore::ReleaseContext()
		{
			mutex.unlock();
		}

		inline void OGLRenderCore::SetRenderTargetAndDepthStencil(Core::RenderState::RenderTarget renderTargetType, Handle renderTarget, Core::RenderState::DepthStencil depthStencilType, Handle depthStencil)
		{
			using namespace Core;

			currentRenderTarget = 0;
			if(renderTargetType == RenderState::RenderTarget_Default) {
				currentRenderTarget = defaultRenderTarget;
			} else if(renderTargetType == RenderState::RenderTarget_Custom) {
				currentRenderTarget = ((GPUTexture *)TextureManager::Get(renderTarget)->handle)->tex;
				if(currentRenderTarget == 0) {
					Console::Error("Tried to set render target to an invalid texture");
				}
			}

			currentDepthStencil = 0;
			if(depthStencilType == RenderState::DepthStencil_Default) {
				currentDepthStencil = defaultDepthStencil;
			} else if(depthStencilType == RenderState::DepthStencil_Custom) {
				currentDepthStencil = ((GPUTexture *)TextureManager::Get(depthStencil)->handle)->tex;
				if(currentDepthStencil == 0) {
					Console::Error("Tried to set depth stencil to an invalid texture");
				}
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, currentRenderTarget, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, currentDepthStencil);
			MAKI_OGL_FAILED();
		}

		inline void OGLRenderCore::SetViewport(const Core::Rect &viewPortRect)
		{
			glViewport(0, 0, (GLsizei)viewPortRect.GetWidth(), (GLsizei)viewPortRect.GetHeight());
			MAKI_OGL_FAILED();
		}

		inline void OGLRenderCore::Clear(bool clearRenderTarget, const float clearColorValues[4], bool clearDepthStencil, float clearDepthValue)
		{
			GLuint clearFlags = 0;
			if(clearRenderTarget && currentRenderTarget != 0) {
				glClearColor(clearColorValues[0], clearColorValues[1], clearColorValues[2], clearColorValues[3]);
				clearFlags |= GL_COLOR_BUFFER_BIT;
			}
			if(clearDepthStencil && currentDepthStencil != 0) {
				glClearDepth(clearDepthValue);
				clearFlags |= GL_DEPTH_BUFFER_BIT;
			}
			glClear(clearFlags);
			MAKI_OGL_FAILED();
		}

		inline void OGLRenderCore::SetDepthState(Core::RenderState::DepthTest depthTest, bool depthWrite)
		{
			using namespace Core;

			switch(depthTest) {
			case RenderState::DepthTest_Less:
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);
				break;
			case RenderState::DepthTest_Equal:
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_EQUAL);
				break;
			case RenderState::DepthTest_LessEqual:
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LEQUAL);
				break;
			case RenderState::DepthTest_Disabled:
			default:
				glDisable(GL_DEPTH_TEST);
				break;
			}
			MAKI_OGL_FAILED();
		}

		inline void OGLRenderCore::SetRasterizerState(Core::RenderState::CullMode cullMode, bool wireFrame)
		{
			using namespace Core;

			switch(cullMode) {
			case RenderState::CullMode_Front:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;
			case RenderState::CullMode_Back:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;
			case RenderState::CullMode_None:
			default:
				glDisable(GL_CULL_FACE);
				break;
			}
			MAKI_OGL_FAILED();
		}

		inline void OGLRenderCore::SetBlendState(bool enabled)
		{
			if(enabled) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			} else {
				glDisable(GL_BLEND);
			}
			MAKI_OGL_FAILED();
		}

		inline void OGLRenderCore::UnbindAllTextures()
		{
			using namespace Core;

			for(uint32 i = 0; i < SHADOW_MAP_SLOT_INDEX_START+Core::RenderState::MAX_LIGHTS; i++) {
				glActiveTexture(GL_TEXTURE0+i);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			MAKI_OGL_FAILED();
		}

		inline void OGLRenderCore::BindShaders(const Core::ShaderProgram *shader)
		{
			glUseProgram((GLuint)shader->handle);
			MAKI_OGL_FAILED();
		}

		inline void OGLRenderCore::SetPerFrameVertexShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader)
		{
			const GPUVertexShader *gvs = (GPUVertexShader *)shader->vertexShader.handle;
			glBindBuffer(GL_UNIFORM_BUFFER, gvs->uboPerFrame);
			MAKI_OGL_FAILED();

			SetPerFrameConstants(state, &shader->vertexShader, gvs->scratchBuffer);
			MAKI_OGL_FAILED();

			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->vertexShader.engineFrameUniformBytes, gvs->scratchBuffer);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		inline void OGLRenderCore::SetPerFramePixelShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader)
		{
			const GPUPixelShader *gps = (GPUPixelShader *)shader->pixelShader.handle;
			glBindBuffer(GL_UNIFORM_BUFFER, gps->uboPerFrame);
			MAKI_OGL_FAILED();

			SetPerFrameConstants(state, &shader->pixelShader, gps->scratchBuffer);
			MAKI_OGL_FAILED();

			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->pixelShader.engineFrameUniformBytes, gps->scratchBuffer);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		inline void OGLRenderCore::BindShadowMaps(const Core::RenderState &state)
		{

			MAKI_OGL_FAILED();
		}

		inline void OGLRenderCore::SetInputLayout(const Core::ShaderProgram *shader, const Core::VertexFormat *vf)
		{
			// Do nothing, this only applies to d3d
		}

		inline void OGLRenderCore::SetMaterialVertexShaderConstants(const Core::ShaderProgram *shader, const Core::Material *mat)
		{
			const GPUVertexShader *gvs = (GPUVertexShader *)shader->vertexShader.handle;
			glBindBuffer(GL_UNIFORM_BUFFER, gvs->uboMaterial);
			MAKI_OGL_FAILED();

			BindMaterialConstants(&shader->vertexShader, true, gvs->scratchBuffer, mat);
			MAKI_OGL_FAILED();

			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->vertexShader.materialUniformBytes, gvs->scratchBuffer);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		inline void OGLRenderCore::SetMaterialPixelShaderConstants(const Core::ShaderProgram *shader, const Core::Material *mat)
		{
			const GPUPixelShader *gps = (GPUPixelShader *)shader->pixelShader.handle;
			glBindBuffer(GL_UNIFORM_BUFFER, gps->uboMaterial);
			MAKI_OGL_FAILED();

			BindMaterialConstants(&shader->pixelShader, false, gps->scratchBuffer, mat);
			MAKI_OGL_FAILED();

			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->pixelShader.materialUniformBytes, gps->scratchBuffer);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		inline void OGLRenderCore::BindTextures(const Core::TextureSet *ts)
		{
			using namespace Core;

			for(uint32 i = 0; i < ts->textureCount; i++) {
				const GPUTexture *gtex = (GPUTexture *)TextureManager::Get(ts->textures[i])->handle;
				glActiveTexture(GL_TEXTURE0+i);
				glBindTexture(GL_TEXTURE_2D, (GLuint)gtex->tex);
			}
			MAKI_OGL_FAILED();
		}

		inline void OGLRenderCore::SetPerObjectVertexShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader, const Core::Matrix44 &matrix, const Core::Matrix44 &mv, const Core::Matrix44 &mvp)
		{
			const GPUVertexShader *gvs = (GPUVertexShader *)shader->vertexShader.handle;
			glBindBuffer(GL_UNIFORM_BUFFER, gvs->uboPerObject);
			MAKI_OGL_FAILED();

			SetPerObjectConstants(&shader->vertexShader, gvs->scratchBuffer, matrix, mv, mvp);
			MAKI_OGL_FAILED();

			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->vertexShader.engineObjectUniformBytes, gvs->scratchBuffer);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		inline void OGLRenderCore::SetPerObjectPixelShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader, const Core::Matrix44 &matrix, const Core::Matrix44 &mv, const Core::Matrix44 &mvp)
		{
			const GPUPixelShader *gps = (GPUPixelShader *)shader->pixelShader.handle;
			glBindBuffer(GL_UNIFORM_BUFFER, gps->uboPerObject);
			MAKI_OGL_FAILED();

			SetPerObjectConstants(&shader->vertexShader, gps->scratchBuffer, matrix, mv, mvp);
			MAKI_OGL_FAILED();

			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->pixelShader.engineObjectUniformBytes, gps->scratchBuffer);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		inline void OGLRenderCore::BindBuffer(void *buffer, const Core::VertexFormat *vf)
		{
			const Buffer *b = (Buffer *)buffer;
			uint32 stride = vf->GetStride();
			uint32 offset = 0;
			glBindBuffer(GL_ARRAY_BUFFER, b->vbos[0]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b->vbos[1]);
			MAKI_OGL_FAILED();
		}

		inline void OGLRenderCore::DrawBuffer(void *buffer)
		{
			const Buffer *b = (Buffer *)buffer;			
			glDrawElements(b->geometryType, b->faceCount*b->indicesPerFace, b->indexDataType, nullptr);
			MAKI_OGL_FAILED();
		}


		// End of non-virtual interface

	} // namespace OGL

} // namespace Maki
