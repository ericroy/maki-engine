#pragma once
#include "core/core_stdafx.h"
#include <mutex>
#include "core/MakiThread.h"
#include "core/MakiSafeQueue.h"
#include "core/MakiRenderPayload.h"
#include "core/MakiShaderProgram.h"
#include "core/MakiRenderState.h"

namespace Maki
{
	namespace Core
	{
		class VertexFormat;

		class RenderCore : public Thread
		{
		public:
			enum Type
			{
				Type_D3D = 0,
				Type_OGL,
			};

		public:
			RenderCore();
			virtual ~RenderCore();
			void Run();

			// GPU resource creation, updates, destruction
			// These all acquire the execution mutex for the render core
			virtual void *UploadBuffer(void *buffer, VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic) = 0;
			virtual void FreeBuffer(void *buffer) = 0;
			virtual bool CreateShaderProgram(ShaderProgram *s) = 0;
			virtual void DeleteShaderProgram(ShaderProgram *s) = 0;
			virtual bool CreateTexture(Texture *t, char *data, uint32 dataLength) = 0;
			virtual bool CreateEmptyTexture(Texture *t, uint8 channels) = 0;
			virtual bool CreateRenderTarget(Texture *t) = 0;
			virtual bool CreateDepthTexture(Texture *t) = 0;
			virtual void WriteToTexture(Texture *t, int32 dstX, int32 dstY, int32 srcX, int32 srcY, uint32 srcWidth, uint32 srcHeight, uint32 srcPitch, uint8 channels, char *srcData) = 0;
			virtual void DeleteTexture(Texture *t) = 0;

		protected:
			virtual void Init() = 0;
			virtual void Present() = 0;
			virtual void Draw(const RenderState &state, const DrawCommandList &commands) = 0;

			template<class Derived>
			void GenericDraw(const RenderState &state, const DrawCommandList &commands);

		public:
			SafeQueue<RenderPayload> input;
			SafeQueue<RenderPayload> output;

		protected:
			std::mutex mutex;
			uint32 windowWidth;
			uint32 windowHeight;
		};


		template<class Derived>
		void RenderCore::GenericDraw(const RenderState &state, const DrawCommandList &commands)
		{
			Derived *derived = static_cast<Derived *>(this);
			
			std::lock_guard<std::mutex> lock(mutex);
			derived->MakeContextCurrent(true);

			// Resize screen buffers if necessary
			if(windowWidth != state.windowWidth || windowHeight != state.windowHeight) {
				derived->SetRenderTargetAndDepthStencil(RenderState::RenderTarget_Null, HANDLE_NONE, RenderState::DepthStencil_Null, HANDLE_NONE);
				derived->Resized(state.windowWidth, state.windowHeight);
				windowWidth = state.windowWidth;
				windowHeight = state.windowHeight;
			}

			derived->SetRenderTargetAndDepthStencil(state.renderTargetType, state.renderTarget, state.depthStencilType, state.depthStencil);
			derived->SetViewport(state.viewPortRect);
			derived->Clear(state.clearRenderTarget, state.renderTargetClearValue.vals, state.clearDepthStencil, state.depthClearValue);
			derived->SetDepthState(state.depthTest, state.depthWrite);
			derived->SetRasterizerState(state.cullMode, state.wireFrame);

			void *currentBuffer = nullptr;
			uint32 currentLayer = 0;
		
			uint32 currentTranslucencyType = DrawCommand::TranslucencyType_Opaque;
			derived->SetBlendState(false);
			
			Handle currentVertexFormat = HANDLE_NONE;
			Handle currentShaderProgram = HANDLE_NONE;		
			Handle currentTextureSet = HANDLE_NONE;
			Handle currentMesh = HANDLE_NONE;
			Handle currentMaterial = HANDLE_NONE;

			bool setLayout = false;

			for(uint32 i = 0; i < commands.count; ++i) {
				const DrawCommandList::ValueEntry &ve = commands.values[commands.keys[i].index];
				const DrawCommand *dc = &ve.drawCommand;
				const Matrix44 &matrix = ve.m;

				const VertexFormat *vf = VertexFormatManager::Get(dc->vertexFormat);
				const ShaderProgram *baseShader = ShaderProgramManager::Get(dc->shaderProgram);
			
				assert(baseShader->variant == ShaderProgram::Variant_Normal && "can only get variants from a normal shader program");
				const ShaderProgram *shader = baseShader;
				if(state.shaderVariant != ShaderProgram::Variant_Normal) {
					Handle h = baseShader->variants[state.shaderVariant-1];
					if(h == HANDLE_NONE) {
						// No such variant for this shader, skip
						continue;
					}
					shader = ShaderProgramManager::Get(h);
				}

				if(currentTranslucencyType != dc->fields.translucencyType) {
					if(dc->fields.translucencyType == DrawCommand::TranslucencyType_Translucent) {
						// Enable blending, disable depth write
						derived->SetDepthState(state.depthTest, false);
						derived->SetBlendState(true);
					} else {
						// Disable blending, restore depth write
						derived->SetDepthState(state.depthTest, state.depthWrite);
						derived->SetBlendState(false);
					}
					currentTranslucencyType = dc->fields.translucencyType;
				}

				if(currentVertexFormat != dc->vertexFormat) {
					currentVertexFormat = dc->vertexFormat;
					setLayout = true;
				}

				if(currentShaderProgram != dc->shaderProgram) {
				
					// Unbind all textures from current shader
					if(currentShaderProgram != HANDLE_NONE) {
						derived->UnbindAllTextures();
					}

					// Bind the new shader and set per-frame constants
					derived->BindShaders(shader);
					currentShaderProgram = dc->shaderProgram;
					if(shader->vertexShader.frameUniformBufferLocation != -1) {
						derived->SetPerFrameVertexShaderConstants(state, shader);
					}
					if(shader->pixelShader.frameUniformBufferLocation != -1) {
						derived->SetPerFramePixelShaderConstants(state, shader);
					}
					derived->BindShadowMaps(state);

					currentMaterial = HANDLE_NONE;
					currentTextureSet = HANDLE_NONE;
					currentBuffer = nullptr;
					setLayout = true;
				}

				// Get or create the input layout for this vertexformat+vertexshader combination
				if(setLayout) {
					derived->SetInputLayout(shader, vf);
					setLayout = false;
				}

				if(currentMaterial != dc->material) {
					Material *mat = MaterialManager::Get(dc->material);
					if(shader->vertexShader.materialUniformBufferLocation != -1) {
						derived->SetMaterialVertexShaderConstants(shader, mat);
					}
					if(shader->pixelShader.materialUniformBufferLocation != -1) {
						derived->SetMaterialPixelShaderConstants(shader, mat);
					}
					currentMaterial = dc->material;
					if(currentTextureSet != dc->textureSet) {
						derived->BindTextures(TextureSetManager::Get(dc->textureSet));
						currentTextureSet = dc->textureSet;
					}
				}
			
				if(shader->vertexShader.objectUniformBufferLocation != -1 || shader->pixelShader.objectUniformBufferLocation != -1) {
					Matrix44 mv = state.view * matrix;
					Matrix44 mvp = state.projection * mv;

					if(shader->vertexShader.objectUniformBufferLocation != -1) {
						derived->SetPerObjectVertexShaderConstants(state, shader, matrix, mv, mvp);
					}
					if(shader->pixelShader.objectUniformBufferLocation != -1) {
						derived->SetPerObjectPixelShaderConstants(state, shader, matrix, mv, mvp);
					}
				}

				// Bind buffers again if they have changed
				void *b = MeshManager::Get(dc->mesh)->GetBuffer();
				if(currentBuffer != b) {
					derived->BindBuffer(b, vf);
					currentBuffer = b;
				}

				// Actually submit the vertex/index data
				derived->DrawBuffer(b);
			}


			if(currentShaderProgram != HANDLE_NONE) {
				derived->UnbindAllTextures();
			}

			derived->SetRenderTargetAndDepthStencil(RenderState::RenderTarget_Null, HANDLE_NONE, RenderState::DepthStencil_Null, HANDLE_NONE);
		}

	} // namespace Core

} // namespace Maki