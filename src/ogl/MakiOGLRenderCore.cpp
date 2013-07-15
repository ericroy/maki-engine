#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLRenderCore.h"
#include "ogl/MakiOGLGPUTypes.h"
#include "ogl/MakiOGLCommon.h"

using namespace Maki::Core;

namespace Maki
{
	namespace OGL
	{
		static void APIENTRY OGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, void *userParam)
		{
			Console::Info("OGL: %s", message);
		}




		OGLRenderCore::OGLRenderCore(Window *window, const Config *config)
			:	RenderCore(),
			window(window),
			context(nullptr),
			frameBuffer(0),
			defaultRenderTarget(0),
			defaultDepthStencil(0),
			currentRenderTarget(0),
			currentDepthStencil(0),
			contextCurrentInRenderThread(false)
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, config->GetInt("engine.ogl_major_version", 2));
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, config->GetInt("engine.ogl_minor_version", 1));
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

#if _DEBUG
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif


			vsync = config->GetBool("engine.vertical_sync", true);
			// -1 allows late swaps to happen immediately
			SDL_GL_SetSwapInterval(vsync ? -1 : 0);
		}

		OGLRenderCore::~OGLRenderCore() {
			if(context != nullptr) {
				SDL_GL_MakeCurrent(window->window, context);

				if(frameBuffer != 0) {
					glDeleteFramebuffers(1, &frameBuffer);
				}

				SDL_GL_DeleteContext(context);
			}

			Console::Info("OpenGL renderer destroyed");
		}

		void OGLRenderCore::Init()
		{
			std::lock_guard<std::mutex> lock(mutex);

			assert(context == nullptr);
			context = SDL_GL_CreateContext(window->window);
			contextCurrentInRenderThread = true;
			DefineGLFunctions();
			MAKI_OGL_FAILED();		

#if _DEBUG
			glDebugMessageCallbackARB(OGLDebugCallback, nullptr);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
#endif

			Console::Info("GL Version: %s", (const char *)glGetString(GL_VERSION));

			glGenFramebuffers(1, &frameBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

			// Set initial state:
			windowWidth = 1;
			windowHeight = 1;
			Resized(windowWidth, windowHeight);

			currentRenderTarget = defaultRenderTarget;
			currentDepthStencil = defaultDepthStencil;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, currentRenderTarget, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, currentDepthStencil);
			MAKI_OGL_FAILED();

			glViewport(0, 0, window->width, window->height);
			MAKI_OGL_FAILED();

			// Render a blank frame so we don't see a flash of white on startup
			glClearColor(1, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			MAKI_OGL_FAILED();

			SDL_GL_SwapWindow(window->window);
		}

		void OGLRenderCore::Resized(uint32 newWidth, uint32 newHeight)
		{
			if(defaultRenderTarget != 0) {
				glDeleteTextures(1, &defaultRenderTarget);
			}
			if(defaultDepthStencil != 0) {
				glDeleteTextures(1, &defaultDepthStencil);
			}

			glGenTextures(1, &defaultRenderTarget);
			glBindTexture(GL_TEXTURE_2D, defaultRenderTarget);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);
			MAKI_OGL_FAILED();

			glGenRenderbuffers(1, &defaultDepthStencil);
			glBindRenderbuffer(GL_RENDERBUFFER, defaultDepthStencil);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, newWidth, newHeight);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			MAKI_OGL_FAILED();
		}

		void OGLRenderCore::Present()
		{
			std::lock_guard<std::mutex> lock(mutex);
			SDL_GL_SwapWindow(window->window);
		}

		void OGLRenderCore::Draw(const RenderState &state, const DrawCommandList &commands)
		{
			std::lock_guard<std::mutex> lock(mutex);
			if(!contextCurrentInRenderThread) {
				SDL_GL_MakeCurrent(window->window, context);
				contextCurrentInRenderThread = true;
			}

			// Resize screen buffers if necessary
			if(windowWidth != state.windowWidth || windowHeight != state.windowHeight) {
				Resized(state.windowWidth, state.windowHeight);
				windowWidth = state.windowWidth;
				windowHeight = state.windowHeight;
			}

			SetRenderTargetAndDepthStencil(state.renderTargetType, state.renderTarget, state.depthStencilType, state.depthStencil);

			// Set viewport
			glViewport(0, 0, window->width, window->height);
			MAKI_OGL_FAILED();

			GLuint clearFlags = 0;

			// Clear depth and render targets if necessary
			if(state.clearDepthStencil && currentDepthStencil != 0) {
				clearFlags |= GL_COLOR_BUFFER_BIT;
			}
			if(state.clearRenderTarget && currentRenderTarget != 0) {
				clearFlags |= GL_DEPTH_BUFFER_BIT;
			}
			glClear(clearFlags);

			// Depth test / depth write
			SetDepthState(state.depthTest, state.depthWrite);

			// Cull mode / wire frame mode
			SetRasterizerState(state.cullMode, state.wireFrame);

			const Buffer *currentBuffer = nullptr;
			uint32 currentLayer = 0;
		
			uint32 currentTranslucencyType = DrawCommand::TranslucencyType_Opaque;
			//context->OMSetBlendState(blendDisabled, nullptr, 0xffffffff);

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

				const GPUVertexShader *gvs = (GPUVertexShader *)shader->vertexShader.handle;
				const GPUPixelShader *gps = (GPUPixelShader *)shader->pixelShader.handle;


				//D3D11_MAPPED_SUBRESOURCE mapped;

				if(currentTranslucencyType != dc->fields.translucencyType) {
					if(dc->fields.translucencyType == DrawCommand::TranslucencyType_Translucent) {
						// Enable blending, disable depth write
						SetDepthState(state.depthTest, false);
						//context->OMSetBlendState(blendEnabled, nullptr, 0xffffffff);
					} else {
						// Disable blending, restore depth write
						SetDepthState(state.depthTest, state.depthWrite);
						//context->OMSetBlendState(blendDisabled, nullptr, 0xffffffff);
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
						for(uint32 i = 0; i < SHADOW_MAP_SLOT_INDEX_START+RenderState::MAX_SHADOW_LIGHTS; i++) {
							glActiveTextureARB(GL_TEXTURE0+i);
							glBindTexture(GL_TEXTURE_2D, 0);
						}
					}

					glUseProgram((GLuint)shader->handle);
					currentShaderProgram = dc->shaderProgram;

					// Set per-frame vertex constants
					if(shader->vertexShader.frameUniformBufferLocation != -1) {
						//context->Map(gvs->perFrameConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
						//SetPerFrameConstants(state, &shader->vertexShader, mapped);
						//context->Unmap(gvs->perFrameConstants, 0);
						//context->VSSetConstantBuffers(shader->vertexShader.frameUniformBufferLocation, 1, &gvs->perFrameConstants);
					}

					// Set per-frame pixel constants
					if(shader->pixelShader.frameUniformBufferLocation != -1) {
						//context->Map(gps->perFrameConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
						//SetPerFrameConstants(state, &shader->pixelShader, mapped);
						//context->Unmap(gps->perFrameConstants, 0);
						//context->PSSetConstantBuffers(shader->pixelShader.frameUniformBufferLocation, 1, &gps->perFrameConstants);
					}

					// Bind shadow map resources
					//ID3D11ShaderResourceView *shadowViews[RenderState::MAX_SHADOW_LIGHTS];
					//ID3D11SamplerState *shadowSamplers[RenderState::MAX_SHADOW_LIGHTS];
					for(uint8 i = 0; i < RenderState::MAX_SHADOW_LIGHTS; i++) {
						if(state.shadowMaps[i] != HANDLE_NONE) {
							GPUTexture *gtex = (GPUTexture *)TextureManager::Get(state.shadowMaps[i])->handle;
							//shadowViews[i] = gtex->shaderResourceView;
							//shadowSamplers[i] = gtex->samplerState;
						} else {
							//shadowViews[i] = nullptr;
							//shadowSamplers[i] = nullptr;
						}
					}
					//context->PSSetShaderResources(SHADOW_MAP_SLOT_INDEX_START, RenderState::MAX_SHADOW_LIGHTS, shadowViews);
					//context->PSSetSamplers(SHADOW_MAP_SLOT_INDEX_START, RenderState::MAX_SHADOW_LIGHTS, shadowSamplers);

					currentMaterial = HANDLE_NONE;
					currentTextureSet = HANDLE_NONE;
					currentBuffer = nullptr;
					setLayout = true;
				}

				// Get or create the input layout for this vertexformat+vertexshader combination
				if(setLayout) {
					//ID3D11InputLayout *layout = ((GPUVertexShader *)shader->vertexShader.handle)->GetOrCreateInputLayout(device, vf);
					//context->IASetInputLayout(layout);
					setLayout = false;
				}

				if(currentMaterial != dc->material) {
					Material *mat = MaterialManager::Get(dc->material);

					if(shader->vertexShader.materialUniformBufferLocation != -1) {
						//context->Map(gvs->materialConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
						//BindMaterialConstants(&shader->vertexShader, true, mapped, mat);
						//context->Unmap(gvs->materialConstants, 0);
						//context->VSSetConstantBuffers(shader->vertexShader.materialUniformBufferLocation, 1, &gvs->materialConstants);
					}
					if(shader->pixelShader.materialUniformBufferLocation != -1) {
						//context->Map(gps->materialConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
						//BindMaterialConstants(&shader->pixelShader, false, mapped, mat);
						//context->Unmap(gps->materialConstants, 0);
						//context->PSSetConstantBuffers(shader->pixelShader.materialUniformBufferLocation, 1, &gps->materialConstants);
					}

					currentMaterial = dc->material;

					if(currentTextureSet != dc->textureSet) {
						BindTextures(shader, TextureSetManager::Get(dc->textureSet));
						currentTextureSet = dc->textureSet;
					}
				}
			
				if(shader->vertexShader.objectUniformBufferLocation != -1 || shader->pixelShader.objectUniformBufferLocation != -1) {
					Matrix44 mv = state.view * matrix;
					Matrix44 mvp = state.projection * mv;

					// Set per-object vertex constants
					if(shader->vertexShader.objectUniformBufferLocation != -1) {
						//context->Map(gvs->perObjectConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
						//SetPerObjectConstants(&shader->vertexShader, mapped, matrix, mv, mvp);
						//context->Unmap(gvs->perObjectConstants, 0);
						//context->VSSetConstantBuffers(shader->vertexShader.objectUniformBufferLocation, 1, &gvs->perObjectConstants);
					}

					// Set per-object pixel constants
					if(shader->pixelShader.objectUniformBufferLocation != -1) {
						//context->Map(gps->perObjectConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
						//SetPerObjectConstants(&shader->pixelShader, mapped, matrix, mv, mvp);
						//context->Unmap(gps->perObjectConstants, 0);
						//context->PSSetConstantBuffers(shader->pixelShader.objectUniformBufferLocation, 1, &gps->perObjectConstants);
					}
				}

				// Bind buffers again if they have changed
				const Buffer *b = (Buffer *)MeshManager::Get(dc->mesh)->GetBuffer();
				if(currentBuffer != b) {

					if(currentBuffer != nullptr) {
						glEnd();
						MAKI_OGL_FAILED();
					}

					uint32 stride = vf->GetStride();
					uint32 offset = 0;
					glBindBufferARB(GL_ARRAY_BUFFER, b->vbos[0]);
					glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, b->vbos[1]);
					currentBuffer = b;

					glBegin(b->geometryType);
					MAKI_OGL_FAILED();
				}

				glDrawElements(b->geometryType, b->faceCount*b->indicesPerFace, b->indexDataType, nullptr);
				MAKI_OGL_FAILED();
			}

			glEnd();
			MAKI_OGL_FAILED();

			// Unbind all textures from current shader
			if(currentShaderProgram != HANDLE_NONE) {
				//context->PSSetShaderResources(0, SHADOW_MAP_SLOT_INDEX_START+RenderState::MAX_SHADOW_LIGHTS, (ID3D11ShaderResourceView **)nullArray);
			}

			currentRenderTarget = 0;
			currentDepthStencil = 0;
			//context->OMSetRenderTargets(0, nullptr, nullptr);
		}

		void OGLRenderCore::SetDepthState(RenderState::DepthTest test, bool write)
		{
			switch(test) {
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

		void OGLRenderCore::SetRasterizerState(RenderState::CullMode cullMode, bool wireFrame)
		{
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

		void OGLRenderCore::SetRenderTargetAndDepthStencil(RenderState::RenderTarget renderTargetType, Handle renderTarget, RenderState::DepthStencil depthStencilType, Handle depthStencil)
		{
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
		}

		//void OGLRenderCore::SetPerFrameConstants(const RenderState &state, const Shader *s, D3D11_MAPPED_SUBRESOURCE &mapped)
		//{
		//	int32 location = s->engineFrameUniformLocations[Shader::FrameUniform_View];
		//	if(location != -1) {
		//		memcpy(((char *)mapped.pData) + location, state.view.vals, 16*sizeof(float));
		//	}
		//	location = s->engineFrameUniformLocations[Shader::FrameUniform_Projection];
		//	if(location != -1) {
		//		memcpy(((char *)mapped.pData) + location, state.projection.vals, sizeof(state.projection));
		//	}
		//	location = s->engineFrameUniformLocations[Shader::FrameUniform_CameraWithHeightNearFar];
		//	if(location != -1) {
		//		memcpy(((char *)mapped.pData) + location, &state.cameraWidthHeightNearFar, sizeof(state.cameraWidthHeightNearFar));
		//	}
		//	location = s->engineFrameUniformLocations[Shader::FrameUniform_CameraSplitDistances];
		//	if(location != -1) {
		//		memcpy(((char *)mapped.pData) + location, &state.cameraSplitDistances, sizeof(state.cameraSplitDistances));
		//	}


		//	location = s->engineFrameUniformLocations[Shader::FrameUniform_LightViewProj];
		//	if(location != -1) {
		//		memcpy(((char *)mapped.pData) + location, state.lightViewProj, sizeof(state.lightViewProj));
		//	}
		//	location = s->engineFrameUniformLocations[Shader::FrameUniform_LightPositions];
		//	if(location != -1) {
		//		memcpy(((char *)mapped.pData) + location, state.lightPositions, sizeof(state.lightPositions));
		//	}
		//	location = s->engineFrameUniformLocations[Shader::FrameUniform_LightDirections];
		//	if(location != -1) {
		//		memcpy(((char *)mapped.pData) + location, state.lightDirections, sizeof(state.lightDirections));
		//	}
		//	location = s->engineFrameUniformLocations[Shader::FrameUniform_LightProperties];
		//	if(location != -1) {
		//		// Set all lighting slots here so that lights which are no longer in use get effectively turned off
		//		memcpy(((char *)mapped.pData) + location, state.lightProperties, sizeof(state.lightProperties));
		//	}
		//	location = s->engineFrameUniformLocations[Shader::FrameUniform_ShadowMapProperties];
		//	if(location != -1) {
		//		memcpy(((char *)mapped.pData) + location, state.shadowMapProperties, state.shadowLightCount*sizeof(RenderState::ShadowMapProperties));
		//	}
		//	location = s->engineFrameUniformLocations[Shader::FrameUniform_LightSplitRegions];
		//	if(location != -1) {
		//		memcpy(((char *)mapped.pData) + location, state.lightSplitRegions, state.cascadedShadowLightCount*RenderState::MAX_CASCADES*sizeof(RenderState::LightSplitRegion));
		//	}
		
		//	location = s->engineFrameUniformLocations[Shader::FrameUniform_GlobalAmbientColor];
		//	if(location != -1) {
		//		memcpy(((char *)mapped.pData) + location, &state.globalAmbientColor.x, sizeof(state.globalAmbientColor));
		//	}
		//}

		/*void OGLRenderCore::SetPerObjectConstants(const Shader *s, D3D11_MAPPED_SUBRESOURCE &mapped, const Matrix44 &model, const Matrix44 &modelView, const Matrix44 &modelViewProjection)
		{
			int32 location = s->engineObjectUniformLocations[Shader::ObjectUniform_Model];
			if(location != -1) {
				memcpy(((char *)mapped.pData) + location, model.vals, sizeof(model));
			}

			location = s->engineObjectUniformLocations[Shader::ObjectUniform_ModelView];
			if(location != -1) {
				memcpy(((char *)mapped.pData) + location, modelView.vals, sizeof(modelView));
			}

			location = s->engineObjectUniformLocations[Shader::ObjectUniform_ModelViewProjection];
			if(location != -1) {
				memcpy(((char *)mapped.pData) + location, modelViewProjection.vals, sizeof(modelViewProjection));
			}
		}*/

		/*void OGLRenderCore::BindMaterialConstants(const Shader *s, bool isVertexShader, D3D11_MAPPED_SUBRESOURCE &mapped, const Material *mat)
		{
			for(uint8 i = 0; i < mat->uniformCount; i++) {
				const Material::UniformValue &val = mat->uniformValues[i];
				int32 location = isVertexShader ? val.vsLocation : val.psLocation;
				if(location != -1) {
					memcpy(((char *)mapped.pData) + location, val.data, val.bytes);
				}
			}
		}*/

		void OGLRenderCore::BindTextures(const ShaderProgram *shader, const TextureSet *ts)
		{
			for(uint32 i = 0; i < ts->textureCount; i++) {
				const GPUTexture *gtex = (GPUTexture *)TextureManager::Get(ts->textures[i])->handle;
				glActiveTextureARB(GL_TEXTURE0+i);
				glBindTexture(GL_TEXTURE_2D, (GLuint)gtex->tex);
			}
			MAKI_OGL_FAILED();
		}









		// Resource creation, deletion, modification:

		void *OGLRenderCore::UploadBuffer(void *buffer, VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic)
		{
			std::lock_guard<std::mutex> lock(mutex);
			assert(!MAKI_OGL_FAILED());
			if(contextCurrentInRenderThread) {
				SDL_GL_MakeCurrent(window->window, context);
				contextCurrentInRenderThread = false;
			}
			assert(!MAKI_OGL_FAILED());

			Buffer *b = (Buffer *)buffer;
			if(b == nullptr) {
				b = new Buffer();
				memset(b, 0, sizeof(Buffer));
				buffer = b;
			} else {
				b->DeleteBuffers();
			}

			glGenBuffersARB(2, b->vbos);
			if(MAKI_OGL_FAILED()) {
				goto failed;
			}

			b->vertexCount = vertexCount;
			b->faceCount = faceCount;
			b->indicesPerFace = indicesPerFace;
			b->bytesPerIndex = bytesPerIndex;

			assert(indicesPerFace > 0 && indicesPerFace <= 3);
			b->geometryType = indicesPerFaceToGeometryType[indicesPerFace];
		
			assert(bytesPerIndex > 0 && bytesPerIndex <= 4 && bytesPerIndex != 3);
			b->indexDataType = bytesPerIndexToFormat[bytesPerIndex];

			int32 stride = vf->GetStride();

			// Create vertex buffer
			glBindBufferARB(GL_ARRAY_BUFFER, b->vbos[0]);
			glBufferDataARB(GL_ARRAY_BUFFER, stride*vertexCount, vertexData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			if(MAKI_OGL_FAILED()) {
				goto failed;
			}

			// Create index buffer
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, b->vbos[1]);
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, bytesPerIndex*indicesPerFace*faceCount, indexData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			if(MAKI_OGL_FAILED()) {
				goto failed;
			}

			return buffer;

failed:
			if(b != nullptr) {
				b->DeleteBuffers();
				delete b;
			}
			return nullptr;
		}

		void OGLRenderCore::FreeBuffer(void *buffer)
		{
			std::lock_guard<std::mutex> lock(mutex);
			if(contextCurrentInRenderThread) {
				SDL_GL_MakeCurrent(window->window, context);
				contextCurrentInRenderThread = false;
			}
			assert(!MAKI_OGL_FAILED());

			if(buffer != nullptr) {
				Buffer *b = (Buffer *)buffer;
				b->DeleteBuffers();
				delete b;
			}
		}

		bool OGLRenderCore::CreatePixelShader(Shader *ps)
		{
			GPUPixelShader *gps = new GPUPixelShader();

			gps->ps = glCreateShader(GL_FRAGMENT_SHADER);
			if(MAKI_OGL_FAILED()) {
				goto failed;
			}

			int32 length = (int32)ps->programDataBytes;
			glShaderSourceARB(gps->ps, 1, (const GLchar **)&ps->programData, &length);
			if(MAKI_OGL_FAILED()) {
				goto failed;
			}
			
			glCompileShaderARB(gps->ps);
			if(MAKI_OGL_FAILED()) {
				goto failed;
			}

			ps->handle = (intptr_t)gps;
			return true;

failed:
			if(gps != nullptr && gps->ps != 0) {
				glDeleteShader(gps->ps);
			}
			SAFE_DELETE(gps);
			return false;
		}

		bool OGLRenderCore::CreateVertexShader(Shader *vs)
		{
			GPUVertexShader *gvs = new GPUVertexShader();

			gvs->vs = glCreateShader(GL_VERTEX_SHADER);
			if(MAKI_OGL_FAILED()) { goto failed; }

			int32 length = (int32)vs->programDataBytes;
			glShaderSourceARB(gvs->vs, 1, (const GLchar **)&vs->programData, &length);
			if(MAKI_OGL_FAILED()) { goto failed; }
			
			glCompileShaderARB(gvs->vs);
			if(MAKI_OGL_FAILED()) { goto failed; }

			vs->handle = (intptr_t)gvs;
			return true;

failed:
			if(gvs != nullptr && gvs->vs != 0) {
				glDeleteShader(gvs->vs);
			}
			SAFE_DELETE(gvs);
			return false;
		}

		bool OGLRenderCore::CreateShaderProgram(ShaderProgram *s)
		{
			std::lock_guard<std::mutex> lock(mutex);
			if(contextCurrentInRenderThread) {
				SDL_GL_MakeCurrent(window->window, context);
				contextCurrentInRenderThread = false;
			}
			assert(!MAKI_OGL_FAILED());

			if(!CreateVertexShader(&s->vertexShader)) {
				return false;
			}

			if(!CreatePixelShader(&s->pixelShader)) {
				return false;
			}

			GLuint program = glCreateProgram();
			if(MAKI_OGL_FAILED()) { goto failed; }

			glAttachShader(program, (GLuint)s->vertexShader.handle);
			if(MAKI_OGL_FAILED()) { goto failed; }

			glAttachShader(program, (GLuint)s->pixelShader.handle);
			if(MAKI_OGL_FAILED()) { goto failed; }

			glLinkProgramARB(program);
			if(MAKI_OGL_FAILED()) { goto failed; }

			s->handle = (intptr_t)program;
			return true;

failed:
			if(program != 0) {
				glDeleteProgram(program);
			}
			return false;
		}

	

		bool OGLRenderCore::CreateEmptyTexture(Texture *t, uint8 channels)
		{
			std::lock_guard<std::mutex> lock(mutex);
			if(contextCurrentInRenderThread) {
				SDL_GL_MakeCurrent(window->window, context);
				contextCurrentInRenderThread = false;
			}
			assert(!MAKI_OGL_FAILED());

			if(channels == 0 || channels > 4 || channels == 3) {
				Console::Error("Unsupported number of channels in image: %d", channels);
				goto failed;
			}

			GLuint tex = 0;
			glGenTextures(1, &tex);
			if(MAKI_OGL_FAILED()) { goto failed; }

			glBindTexture(GL_TEXTURE_2D, tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, channels, t->width, t->height, 0, channelsToFormat[channels], GL_UNSIGNED_BYTE, nullptr);
			if(MAKI_OGL_FAILED()) { goto failed; }

			GPUTexture *gtex = new GPUTexture();
			gtex->tex = tex;
			t->handle = (intptr_t)gtex;
			return true;

failed:
			if(tex != 0) {
				glDeleteTextures(1, &tex);
			}
			return false;
		}

		bool OGLRenderCore::CreateRenderTarget(Texture *t)
		{
			std::lock_guard<std::mutex> lock(mutex);
			if(contextCurrentInRenderThread) {
				SDL_GL_MakeCurrent(window->window, context);
				contextCurrentInRenderThread = false;
			}
			assert(!MAKI_OGL_FAILED());

			GLuint tex = 0;
			glGenTextures(1, &tex);
			if(MAKI_OGL_FAILED()) { goto failed; }

			glBindTexture(GL_TEXTURE_2D, tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->width, t->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			if(MAKI_OGL_FAILED()) { goto failed; }

			GPUTexture *gtex = new GPUTexture();
			gtex->tex = tex;
			t->handle = (intptr_t)gtex;
			return true;

failed:
			if(tex != 0) {
				glDeleteTextures(1, &tex);
			}
			return false;
		}

		bool OGLRenderCore::CreateDepthTexture(Texture *t)
		{
			std::lock_guard<std::mutex> lock(mutex);
			if(contextCurrentInRenderThread) {
				SDL_GL_MakeCurrent(window->window, context);
				contextCurrentInRenderThread = false;
			}
			assert(!MAKI_OGL_FAILED());

			GLuint tex = 0;
			glGenTextures(1, &tex);
			if(MAKI_OGL_FAILED()) { goto failed; }

			glBindTexture(GL_TEXTURE_2D, tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_TEXTURE_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_TEXTURE_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, t->width, t->height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
			if(MAKI_OGL_FAILED()) { goto failed; }

			GPUTexture *gtex = new GPUTexture();
			gtex->tex = tex;
			t->handle = (intptr_t)gtex;
			return true;

failed:
			if(tex != 0) {
				glDeleteTextures(1, &tex);
			}
			return false;
		}

		bool OGLRenderCore::CreateTexture(Texture *t, char *data, uint32 dataLength)
		{
			std::lock_guard<std::mutex> lock(mutex);
			if(contextCurrentInRenderThread) {
				SDL_GL_MakeCurrent(window->window, context);
				contextCurrentInRenderThread = false;
			}
			assert(!MAKI_OGL_FAILED());

			// TODO:
			// Need to determine num channels here - probably need to borrow code from directx dds loader
			uint32 channels = 4;
			if(channels == 0 || channels > 4 || channels == 3) {
				Console::Error("Unsupported number of channels in image: %d", channels);
				goto failed;
			}

			GLuint tex = 0;
			glGenTextures(1, &tex);
			if(MAKI_OGL_FAILED()) { goto failed; }

			glBindTexture(GL_TEXTURE_2D, tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

			// TODO:
			// Need to determine internal format here - probably need to borrow code from directx dds loader
			glCompressedTexImage2DARB(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA, t->width, t->height, 0, dataLength, data);
			if(MAKI_OGL_FAILED()) { goto failed; }

			GPUTexture *gtex = new GPUTexture();
			gtex->tex = tex;
			t->handle = (intptr_t)gtex;
			return true;

failed:
			if(tex != 0) {
				glDeleteTextures(1, &tex);
			}
			return false;
		}

		void OGLRenderCore::WriteToTexture(Texture *t, int32 dstX, int32 dstY, int32 srcX, int32 srcY, uint32 srcWidth, uint32 srcHeight, uint32 srcPitch, uint8 channels, char *srcData)
		{
			std::lock_guard<std::mutex> lock(mutex);
			if(contextCurrentInRenderThread) {
				SDL_GL_MakeCurrent(window->window, context);
				contextCurrentInRenderThread = false;
			}
			assert(!MAKI_OGL_FAILED());
		
			GPUTexture *gtex = (GPUTexture *)t->handle;
			glBindTexture(GL_TEXTURE_2D, gtex->tex);
			MAKI_OGL_FAILED();

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			MAKI_OGL_FAILED();

			glPixelStorei(GL_UNPACK_ROW_LENGTH, srcPitch/channels);
			MAKI_OGL_FAILED();

			GLenum format = channelsToFormat[channels];

			// Find the data at point (srcX, srcY) in the source image
			char *p = srcData + (srcY*srcPitch) + (srcX*channels);

			glTexSubImage2D(GL_TEXTURE_2D, 0, dstX, dstY, (GLsizei)srcWidth, (GLsizei)srcHeight, format, GL_UNSIGNED_BYTE, p);
			MAKI_OGL_FAILED();

			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			MAKI_OGL_FAILED();
		}

		void OGLRenderCore::DeleteShaderProgram(ShaderProgram *s)
		{
			std::lock_guard<std::mutex> lock(mutex);
			if(contextCurrentInRenderThread) {
				SDL_GL_MakeCurrent(window->window, context);
				contextCurrentInRenderThread = false;
			}
			assert(!MAKI_OGL_FAILED());

			GPUVertexShader *gvs = (GPUVertexShader *)s->vertexShader.handle;
			SAFE_DELETE(gvs);
			s->vertexShader.handle = (intptr_t)nullptr;

			GPUPixelShader *gps = (GPUPixelShader *)s->pixelShader.handle;
			SAFE_DELETE(gps);
			s->pixelShader.handle = (intptr_t)nullptr;
		}

		void OGLRenderCore::DeleteTexture(Texture *t)
		{
			std::lock_guard<std::mutex> lock(mutex);
			if(contextCurrentInRenderThread) {
				SDL_GL_MakeCurrent(window->window, context);
				contextCurrentInRenderThread = false;
			}
			assert(!MAKI_OGL_FAILED());

			GPUTexture *gtex = (GPUTexture *)t->handle;
			SAFE_DELETE(gtex);
			t->handle = (intptr_t)nullptr;
		}


	} // namespace OGL

} // namespace Maki
