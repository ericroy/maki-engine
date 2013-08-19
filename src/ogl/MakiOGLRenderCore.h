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
		public:
			OGLRenderCore(Core::Window *window, const Core::Config *config);
			virtual ~OGLRenderCore();
			void Init();
			inline void Draw(const Core::RenderState &state, const Core::DrawCommandList &commands) { GenericDraw<OGLRenderCore>(state, commands); }
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
			bool CreateShader(GLenum shaderType, Core::Shader *ps);

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
			Core::RenderState::RenderTarget currentRenderTargetType;

			GLuint currentDepthStencil;
			Core::RenderState::DepthStencil currentDepthStencilType;

			bool depthWriteEnabled;
			bool blendEnabled;
			Core::RenderState::DepthTest currentDepthTest;
			Core::RenderState::CullMode currentCullMode;
			
			bool vsync;
			bool debugOutput;
			std::mutex mutex;


			GLuint vao;
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

			// Default framebuffer
			GLuint fbo = 0;

			currentRenderTarget = 0;
			currentRenderTargetType = renderTargetType;
			if(renderTargetType == RenderState::RenderTarget_Default) {
				assert(depthStencilType == RenderState::DepthStencil_Default && "If one of render target or depth stencil is 'default', then the other must be too");
				currentRenderTarget = defaultRenderTarget;
			} else if(renderTargetType == RenderState::RenderTarget_Custom) {
				fbo = frameBuffer;
				currentRenderTarget = ((GPUTexture *)TextureManager::Get(renderTarget)->handle)->tex;
				if(currentRenderTarget == 0) {
					Console::Error("Tried to set render target to an invalid texture");
				}
			} else {
				fbo = frameBuffer;
			}
			
			currentDepthStencil = 0;
			currentDepthStencilType = depthStencilType;
			if(depthStencilType == RenderState::DepthStencil_Default) {
				assert(renderTargetType == RenderState::RenderTarget_Default && "If one of render target or depth stencil is 'default', then the other must be too");
				currentDepthStencil = defaultDepthStencil;
			} else if(depthStencilType == RenderState::DepthStencil_Custom) {
				fbo = frameBuffer;
				currentDepthStencil = ((GPUTexture *)TextureManager::Get(depthStencil)->handle)->tex;
				if(currentDepthStencil == 0) {
					Console::Error("Tried to set depth stencil to an invalid texture");
				}
			} else {
				fbo = frameBuffer;
			}			
			
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			if(fbo != 0) {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, currentRenderTarget, 0);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, currentDepthStencil);
			}
		}

		inline void OGLRenderCore::SetViewport(const Core::Rect &viewPortRect)
		{
			glViewport(0, 0, (GLsizei)viewPortRect.GetWidth(), (GLsizei)viewPortRect.GetHeight());
		}

		inline void OGLRenderCore::Clear(bool clearRenderTarget, const float clearColorValues[4], bool clearDepthStencil, float clearDepthValue)
		{
			using namespace Core;

			GLuint clearFlags = 0;
			if(clearRenderTarget && currentRenderTargetType != RenderState::RenderTarget_Null) {
				glClearColor(clearColorValues[0], clearColorValues[1], clearColorValues[2], clearColorValues[3]);
				clearFlags |= GL_COLOR_BUFFER_BIT;
			}
			if(clearDepthStencil && currentDepthStencilType != RenderState::DepthStencil_Null) {
				glClearDepth(clearDepthValue);
				clearFlags |= GL_DEPTH_BUFFER_BIT;
			}
			glClear(clearFlags);
		}

		inline void OGLRenderCore::SetDepthState(Core::RenderState::DepthTest depthTest, bool depthWrite)
		{
			using namespace Core;

			if(depthWriteEnabled != depthWrite) {
				glDepthMask(depthWrite ? GL_TRUE : GL_FALSE);
				depthWriteEnabled = depthWrite;
			}

			if(currentDepthTest != depthTest) {
				if(depthTest == RenderState::DepthTest_Disabled) {
					glDisable(GL_DEPTH_TEST);
				} else if(currentDepthTest == RenderState::DepthTest_Disabled) {
					glEnable(GL_DEPTH_TEST);
				}

				switch(depthTest) {
				case RenderState::DepthTest_Less:
					glDepthFunc(GL_LESS);
					break;
				case RenderState::DepthTest_Equal:
					glDepthFunc(GL_EQUAL);
					break;
				case RenderState::DepthTest_LessEqual:
					glDepthFunc(GL_LEQUAL);
					break;
				}

				currentDepthTest = depthTest;
			}
		}

		inline void OGLRenderCore::SetRasterizerState(Core::RenderState::CullMode cullMode, bool wireFrame)
		{
			using namespace Core;

			if(currentCullMode != cullMode) {
				if(cullMode == RenderState::CullMode_None) {
					glDisable(GL_CULL_FACE);
				} else if(currentCullMode == RenderState::CullMode_None) {
					glEnable(GL_CULL_FACE);
				}

				switch(cullMode) {
				case RenderState::CullMode_Front:
					glCullFace(GL_FRONT);
					break;
				case RenderState::CullMode_Back:
					glCullFace(GL_BACK);
					break;
				}

				currentCullMode = cullMode;
			}
		}

		inline void OGLRenderCore::SetBlendState(bool enabled)
		{
			if(blendEnabled != enabled) {
				if(enabled) {
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				} else {
					glDisable(GL_BLEND);
				}

				blendEnabled = enabled;
			}
		}

		inline void OGLRenderCore::UnbindAllTextures()
		{
			for(uint32 i = 0; i < SHADOW_MAP_SLOT_INDEX_START+Core::RenderState::MAX_LIGHTS; i++) {
				glActiveTexture(GL_TEXTURE0+i);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		inline void OGLRenderCore::BindShaders(const Core::ShaderProgram *shader)
		{
			glUseProgram((GLuint)shader->handle);
		}

		inline void OGLRenderCore::SetPerFrameVertexShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader)
		{
			const GPUShader *gs = (GPUShader *)shader->vertexShader.handle;
			glBindBuffer(GL_UNIFORM_BUFFER, gs->uboPerFrame);
			SetPerFrameConstants(state, &shader->vertexShader, gs->scratchBuffer);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->vertexShader.engineFrameUniformBytes, gs->scratchBuffer);
			glBindBufferBase(GL_UNIFORM_BUFFER, shader->vertexShader.frameUniformBufferLocation, gs->uboPerFrame);
		}

		inline void OGLRenderCore::SetPerFramePixelShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader)
		{
			const GPUShader *gs = (GPUShader *)shader->pixelShader.handle;
			glBindBuffer(GL_UNIFORM_BUFFER, gs->uboPerFrame);
			SetPerFrameConstants(state, &shader->pixelShader, gs->scratchBuffer);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->pixelShader.engineFrameUniformBytes, gs->scratchBuffer);
			glBindBufferBase(GL_UNIFORM_BUFFER, shader->pixelShader.frameUniformBufferLocation, gs->uboPerFrame);
		}

		inline void OGLRenderCore::BindShadowMaps(const Core::RenderState &state)
		{
			using namespace Core;

			for(uint8 i = 0; i < RenderState::MAX_SHADOW_LIGHTS; i++) {
				glActiveTexture(GL_TEXTURE0+SHADOW_MAP_SLOT_INDEX_START+i);

				if(state.shadowMaps[i] != HANDLE_NONE) {
					GPUTexture *gtex = (GPUTexture *)TextureManager::Get(state.shadowMaps[i])->handle;
					glBindTexture(GL_TEXTURE_2D, (GLuint)gtex->tex);
				} else {
					glBindTexture(GL_TEXTURE_2D, 0);
				}
			}
		}

		inline void OGLRenderCore::SetInputLayout(const Core::ShaderProgram *shader, const Core::VertexFormat *vf)
		{
			using namespace Core;

			for(uint32 i = 0; i < VertexFormat::AttributeCount; i++) {
				VertexFormat::Attribute attr = (VertexFormat::Attribute)i;
				if(vf->HasAttribute(attr)) {
					glEnableVertexAttribArray(attr);
				} else {
					glDisableVertexAttribArray(attr);
				}
			}
		}

		inline void OGLRenderCore::SetMaterialVertexShaderConstants(const Core::ShaderProgram *shader, const Core::Material *mat)
		{
			const GPUShader *gs = (GPUShader *)shader->vertexShader.handle;
			glBindBuffer(GL_UNIFORM_BUFFER, gs->uboMaterial);
			BindMaterialConstants(&shader->vertexShader, true, gs->scratchBuffer, mat);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->vertexShader.materialUniformBytes, gs->scratchBuffer);
			glBindBufferBase(GL_UNIFORM_BUFFER, shader->vertexShader.materialUniformBufferLocation, gs->uboMaterial);
		}

		inline void OGLRenderCore::SetMaterialPixelShaderConstants(const Core::ShaderProgram *shader, const Core::Material *mat)
		{
			const GPUShader *gs = (GPUShader *)shader->pixelShader.handle;
			glBindBuffer(GL_UNIFORM_BUFFER, gs->uboMaterial);
			BindMaterialConstants(&shader->pixelShader, false, gs->scratchBuffer, mat);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->pixelShader.materialUniformBytes, gs->scratchBuffer);
			glBindBufferBase(GL_UNIFORM_BUFFER, shader->pixelShader.materialUniformBufferLocation, gs->uboMaterial);
		}

		inline void OGLRenderCore::BindTextures(const Core::TextureSet *ts)
		{
			using namespace Core;

			for(uint32 i = 0; i < ts->textureCount; i++) {
				const GPUTexture *gtex = (GPUTexture *)TextureManager::Get(ts->textures[i])->handle;
				glActiveTexture(GL_TEXTURE0+i);
				glBindTexture(GL_TEXTURE_2D, (GLuint)gtex->tex);
			}
		}

		inline void OGLRenderCore::SetPerObjectVertexShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader, const Core::Matrix44 &matrix, const Core::Matrix44 &mv, const Core::Matrix44 &mvp)
		{
			const GPUShader *gs = (GPUShader *)shader->vertexShader.handle;
			glBindBuffer(GL_UNIFORM_BUFFER, gs->uboPerObject);
			SetPerObjectConstants(&shader->vertexShader, gs->scratchBuffer, matrix, mv, mvp);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->vertexShader.engineObjectUniformBytes, gs->scratchBuffer);
			glBindBufferBase(GL_UNIFORM_BUFFER, shader->vertexShader.objectUniformBufferLocation, gs->uboPerObject);
		}

		inline void OGLRenderCore::SetPerObjectPixelShaderConstants(const Core::RenderState &state, const Core::ShaderProgram *shader, const Core::Matrix44 &matrix, const Core::Matrix44 &mv, const Core::Matrix44 &mvp)
		{
			const GPUShader *gs = (GPUShader *)shader->pixelShader.handle;
			glBindBuffer(GL_UNIFORM_BUFFER, gs->uboPerObject);
			SetPerObjectConstants(&shader->vertexShader, gs->scratchBuffer, matrix, mv, mvp);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->pixelShader.engineObjectUniformBytes, gs->scratchBuffer);
			glBindBufferBase(GL_UNIFORM_BUFFER, shader->pixelShader.objectUniformBufferLocation, gs->uboPerObject);
		}

		inline void OGLRenderCore::BindBuffer(void *buffer, const Core::VertexFormat *vf)
		{
			using namespace Core;

			if(buffer == nullptr) {
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				return;
			}

			const Buffer *b = (Buffer *)buffer;
			glBindBuffer(GL_ARRAY_BUFFER, b->vbos[0]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b->vbos[1]);

			const int32 stride = vf->GetStride();
			uint32 offset = 0;
			for(uint32 i = 0; i < VertexFormat::AttributeCount; i++) {
				VertexFormat::Attribute attr = (VertexFormat::Attribute)i;
				if(vf->HasAttribute(attr)) {
					GLint count = vf->GetDataCount(attr);
					VertexFormat::DataType type = vf->GetDataType(attr);
					glVertexAttribPointer(attr, count, typeToGLType[type], normalizeAttribute[attr], stride, (GLvoid *)offset);
					offset += count * VertexFormat::DataTypeSizes[type];
				}
			}
		}

		inline void OGLRenderCore::DrawBuffer(void *buffer)
		{
			const Buffer *b = (Buffer *)buffer;
			glDrawElements(b->geometryType, b->faceCount*b->indicesPerFace, b->indexDataType, nullptr);
		}


		// End of non-virtual interface

	} // namespace OGL

} // namespace Maki
