#include "core/core_stdafx.h"
#include "core/MakiDrawCommand.h"
#include "core/MakiMesh.h"
#include "core/MakiTexture.h"
#include "core/MakiShaderProgram.h"
#include "core/MakiConfig.h"
#include "core/MakiDrawCommandList.h"
#include "core/opengl/MakiRenderCoreImpl.h"
#include "core/MakiMeshManager.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiShaderProgramManager.h"
#include "core/MakiTextureSetManager.h"


using namespace Maki::Core;

namespace Maki
{
	namespace Core
	{
		namespace OGL
		{

			RenderCoreImpl::RenderCoreImpl(Window *window, const Config *config)
				:	RenderCore()
			{
				memset(nullArray, 0, sizeof(nullArray));

				vsync = config->GetBool("engine.vertical_sync", true);
				maxVertexFormatsPerVertexShader = config->GetUint("engine.max_vertex_formats_per_vertex_shader", 6);
				
				
			}

			RenderCoreImpl::~RenderCoreImpl() {
				Console::Info("OpenGL renderer destroyed");
			}

			void RenderCoreImpl::Init()
			{
				std::lock_guard<std::mutex> lock(mutex);

				// Set initial state:
				windowWidth = 1;
				windowHeight = 1;

				Resized(windowWidth, windowHeight);

				// Render a blank frame so we don't see a flash of white on startup
				
			}

			void RenderCoreImpl::Resized(uint32 newWidth, uint32 newHeight)
			{

			}

			void RenderCoreImpl::Present()
			{
				std::lock_guard<std::mutex> lock(mutex);
			}

			void RenderCoreImpl::Draw(const RenderState &state, const DrawCommandList &commands)
			{
				std::lock_guard<std::mutex> lock(mutex);

				// Resize screen buffers if necessary
				if(windowWidth != state.windowWidth || windowHeight != state.windowHeight) {
					Resized(state.windowWidth, state.windowHeight);
					windowWidth = state.windowWidth;
					windowHeight = state.windowHeight;
				}

				SetRenderTargetAndDepthStencil(state.renderTargetType, state.renderTarget, state.depthStencilType, state.depthStencil);

				// Set viewport
				

				// Clear depth if necessary
				//if(state.clearDepthStencil && currentDepthStencilView != nullptr) {
				//	context->ClearDepthStencilView(currentDepthStencilView, D3D11_CLEAR_DEPTH, state.depthClearValue, 0);
				//}

				//// Clear render target if necessary
				//if(state.clearRenderTarget && currentRenderTargetView != nullptr) {
				//	context->ClearRenderTargetView(currentRenderTargetView, state.renderTargetClearValue.vals);
				//}

				// Depth test / depth write
				SetDepthState(state.depthTest, state.depthWrite);

				// Cull mode / wire frame mode
				SetRasterizerState(state.cullMode, state.wireFrame);

				//const Buffer *currentBuffer = nullptr;
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

					//const GPUVertexShader *gvs = (GPUVertexShader *)shader->vertexShader.handle;
					//const GPUPixelShader *gps = (GPUPixelShader *)shader->pixelShader.handle;


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
							//context->PSSetShaderResources(0, SHADOW_MAP_SLOT_INDEX_START+RenderState::MAX_SHADOW_LIGHTS, (ID3D11ShaderResourceView **)nullArray);
						}

						//context->VSSetShader(gvs->vs, nullptr, 0);
						//context->PSSetShader(gps->ps, nullptr, 0);
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
								//GPUTexture *tex = (GPUTexture *)TextureManager::Get(state.shadowMaps[i])->handle;
								//shadowViews[i] = tex->shaderResourceView;
								//shadowSamplers[i] = tex->samplerState;
							} else {
								//shadowViews[i] = nullptr;
								//shadowSamplers[i] = nullptr;
							}
						}
						//context->PSSetShaderResources(SHADOW_MAP_SLOT_INDEX_START, RenderState::MAX_SHADOW_LIGHTS, shadowViews);
						//context->PSSetSamplers(SHADOW_MAP_SLOT_INDEX_START, RenderState::MAX_SHADOW_LIGHTS, shadowSamplers);

						currentMaterial = HANDLE_NONE;
						currentTextureSet = HANDLE_NONE;
						//currentBuffer = nullptr;
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
					//const Buffer *b = (Buffer *)MeshManager::Get(dc->mesh)->GetBuffer();
					//if(currentBuffer != b) {
						//uint32 stride = vf->GetStride();
						//uint32 offset = 0;
						//context->IASetVertexBuffers(0, 1, &b->vbos[0], &stride, &offset);
						//context->IASetIndexBuffer(b->vbos[1], (DXGI_FORMAT)b->indexDataType, 0);
						//currentBuffer = b;
					//}

					//context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)b->geometryType);
					//context->DrawIndexed(b->indicesPerFace*b->faceCount, 0, 0);
				}

				// Unbind all textures from current shader
				if(currentShaderProgram != HANDLE_NONE) {
					//context->PSSetShaderResources(0, SHADOW_MAP_SLOT_INDEX_START+RenderState::MAX_SHADOW_LIGHTS, (ID3D11ShaderResourceView **)nullArray);
				}

				//currentRenderTargetView = nullptr;
				//currentDepthStencilView = nullptr;
				//context->OMSetRenderTargets(0, nullptr, nullptr);
			}

			void RenderCoreImpl::SetDepthState(RenderState::DepthTest test, bool write)
			{
				switch(test) {
				case RenderState::DepthTest_Less:
					//context->OMSetDepthStencilState(write ? depthStateLessWrite : depthStateLess, 1);
					break;
				case RenderState::DepthTest_Equal:
					//context->OMSetDepthStencilState(write ? depthStateEqualWrite : depthStateEqual, 1);
					break;
				case RenderState::DepthTest_LessEqual:
					//context->OMSetDepthStencilState(write ? depthStateLessEqualWrite : depthStateLessEqual, 1);
					break;
				case RenderState::DepthTest_Disabled:
				default:
					//context->OMSetDepthStencilState(write ? depthStateWrite : depthState, 1);
					break;
				}
			}

			void RenderCoreImpl::SetRasterizerState(RenderState::CullMode cullMode, bool wireFrame)
			{
				switch(cullMode) {
				case RenderState::CullMode_Front:
					//context->RSSetState(wireFrame ? rasterizerStateWireFrameCullFront : rasterizerStateCullFront);
					break;
				case RenderState::CullMode_Back:
					//context->RSSetState(wireFrame ? rasterizerStateWireFrameCullBack : rasterizerStateCullBack);
					break;
				case RenderState::CullMode_None:
				default:
					//context->RSSetState(wireFrame ? rasterizerStateWireFrame : rasterizerState);
					break;
				}
			}

			void RenderCoreImpl::SetRenderTargetAndDepthStencil(RenderState::RenderTarget renderTargetType, Handle renderTarget, RenderState::DepthStencil depthStencilType, Handle depthStencil)
			{
				//currentRenderTargetView = nullptr;
				if(renderTargetType == RenderState::RenderTarget_Default) {
					//currentRenderTargetView = defaultRenderTargetView;
				} else if(renderTargetType == RenderState::RenderTarget_Custom) {
					//currentRenderTargetView = ((GPUTexture *)TextureManager::Get(renderTarget)->handle)->renderTargetView;
					//if(currentRenderTargetView == nullptr) {
					//	Console::Error("Tried to set render target to an invalid texture");
					//}
				}

				//currentDepthStencilView = nullptr;
				if(depthStencilType == RenderState::DepthStencil_Default) {
					//currentDepthStencilView = defaultDepthStencilView;
				} else if(depthStencilType == RenderState::DepthStencil_Custom) {
					//currentDepthStencilView = ((GPUTexture *)TextureManager::Get(depthStencil)->handle)->depthStencilView;
					//if(currentDepthStencilView == nullptr) {
					//	Console::Error("Tried to set depth stencil to an invalid texture");
					//}
				}

				//context->OMSetRenderTargets(1, &currentRenderTargetView, currentDepthStencilView);
			}

			//void RenderCoreImpl::SetPerFrameConstants(const RenderState &state, const Shader *s, D3D11_MAPPED_SUBRESOURCE &mapped)
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

			/*void RenderCoreImpl::SetPerObjectConstants(const Shader *s, D3D11_MAPPED_SUBRESOURCE &mapped, const Matrix44 &model, const Matrix44 &modelView, const Matrix44 &modelViewProjection)
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

			/*void RenderCoreImpl::BindMaterialConstants(const Shader *s, bool isVertexShader, D3D11_MAPPED_SUBRESOURCE &mapped, const Material *mat)
			{
				for(uint8 i = 0; i < mat->uniformCount; i++) {
					const Material::UniformValue &val = mat->uniformValues[i];
					int32 location = isVertexShader ? val.vsLocation : val.psLocation;
					if(location != -1) {
						memcpy(((char *)mapped.pData) + location, val.data, val.bytes);
					}
				}
			}*/

			/*void RenderCoreImpl::BindTextures(const ShaderProgram *shader, const TextureSet *ts)
			{
				ID3D11ShaderResourceView *views[TextureSet::MAX_TEXTURES_PER_SET];
				ID3D11SamplerState *samplers[TextureSet::MAX_TEXTURES_PER_SET];

				for(uint8 i = 0; i < ts->textureCount; i++) {
					const GPUTexture *tex = (GPUTexture *)TextureManager::Get(ts->textures[i])->handle;
					views[i] = tex->shaderResourceView;
					samplers[i] = tex->samplerState;
				}

				context->PSSetShaderResources(0, ts->textureCount, views);
				context->PSSetSamplers(0, ts->textureCount, samplers);
			}*/









			// Resource creation, deletion, modification:

			void *RenderCoreImpl::UploadBuffer(void *buffer, VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic)
			{
				std::lock_guard<std::mutex> lock(mutex);

				//Buffer *b = (Buffer *)buffer;
				//if(b == nullptr) {
				//	b = new Buffer();
				//	memset(b, 0, sizeof(Buffer));
				//	buffer = b;
				//} else {
				//	SAFE_RELEASE(b->vbos[0]);
				//	SAFE_RELEASE(b->vbos[1]);
				//}

				//b->vertexCount = vertexCount;
				//b->faceCount = faceCount;
				//b->indicesPerFace = indicesPerFace;
				//b->bytesPerIndex = bytesPerIndex;

				//assert(indicesPerFace > 0 && indicesPerFace <= 3);
				//b->geometryType = indicesPerFaceToGeometryType[indicesPerFace];
		
				//assert(bytesPerIndex > 0 && bytesPerIndex <= 4 && bytesPerIndex != 3);
				//b->indexDataType = bytesPerIndexToFormat[bytesPerIndex];

				//// Create vertex buffer
				//D3D11_SUBRESOURCE_DATA srd;
				//D3D11_BUFFER_DESC bd;
				//ZeroMemory(&bd, sizeof(bd));
				//ZeroMemory(&srd, sizeof(srd));
				//bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
				//bd.ByteWidth = vf->GetStride()*vertexCount;
				//bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				//if(dynamic) {
				//	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				//}
				//srd.pSysMem = vertexData;
				//if(MAKI_D3D_FAILED(device->CreateBuffer(&bd, &srd, &b->vbos[0]))) {
				//	Console::Error("Failed to create d3d vertex buffer");
				//}
		
				//// Create index buffer
				//ZeroMemory(&bd, sizeof(bd));
				//ZeroMemory(&srd, sizeof(srd));
				//bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
				//bd.ByteWidth = bytesPerIndex*indicesPerFace*faceCount;
				//bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
				//if(dynamic) {
				//	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				//}
				//srd.pSysMem = indexData;
				//if(MAKI_D3D_FAILED(device->CreateBuffer(&bd, &srd, &b->vbos[1]))) {
				//	Console::Error("Failed to create d3d index buffer");
				//}

				return buffer;
			}

			void RenderCoreImpl::FreeBuffer(void *buffer)
			{
				std::lock_guard<std::mutex> lock(mutex);

				/*if(buffer != nullptr) {
					Buffer *b = (Buffer *)buffer;
					SAFE_RELEASE(b->vbos[0]);
					SAFE_RELEASE(b->vbos[1]);
					delete b;
				}*/
			}

			bool RenderCoreImpl::CreatePixelShader(Shader *ps)
			{
		//		GPUPixelShader *gps = new GPUPixelShader();

		//		if(MAKI_D3D_FAILED(device->CreatePixelShader(ps->programData, ps->programDataBytes, nullptr, &gps->ps))) {
		//			goto failed;
		//		}

		//		// Allocate buffers for constants
		//		D3D11_BUFFER_DESC bufferDesc;
		//		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		//		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		//		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		//		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		//		bufferDesc.MiscFlags = 0;
		//		bufferDesc.StructureByteStride = 0;

		//#		define ROUND_SIXTEEN(x) ((x) & 0x7) != 0 ? ((x) & ~0x7)+0x10 : (x)
		//		if(ps->frameUniformBufferLocation != -1) {
		//			bufferDesc.ByteWidth = ROUND_SIXTEEN(ps->engineFrameUniformBytes);
		//			if(MAKI_D3D_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &gps->perFrameConstants))) {
		//				goto failed;
		//			}
		//		}
		//		if(ps->objectUniformBufferLocation != -1) {
		//			bufferDesc.ByteWidth = ROUND_SIXTEEN(ps->engineObjectUniformBytes);
		//			if(MAKI_D3D_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &gps->perObjectConstants))) {
		//				goto failed;
		//			}
		//		}
		//		if(ps->materialUniformBufferLocation != -1) {
		//			bufferDesc.ByteWidth = ROUND_SIXTEEN(ps->materialUniformBytes);
		//			if(MAKI_D3D_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &gps->materialConstants))) {
		//				goto failed;
		//			}
		//		}
		//#		undef ROUND_SIXTEEN

		//		ps->handle = (intptr_t)gps;
		//		return true;

		//failed:
		//		SAFE_DELETE(gps);
				return false;
			}

			bool RenderCoreImpl::CreateVertexShader(Shader *vs)
			{
		//		GPUVertexShader *gvs = new GPUVertexShader(maxVertexFormatsPerVertexShader);

		//		// Since vertex shader keeps a pointer to this data, we will not delete the buffer as we exit the function
		//		char *blob = (char *)Allocator::Malloc(vs->programDataBytes);
		//		memcpy(blob, vs->programData, vs->programDataBytes);
		//		gvs->blob = blob;
		//		gvs->blobSize = vs->programDataBytes;
		//
		//		if(MAKI_D3D_FAILED(device->CreateVertexShader(gvs->blob, gvs->blobSize, nullptr, &gvs->vs))) {
		//			goto failed;
		//		}

		//		// Allocate buffers for constants
		//		D3D11_BUFFER_DESC bufferDesc;
		//		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		//		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		//		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		//		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		//		bufferDesc.MiscFlags = 0;
		//		bufferDesc.StructureByteStride = 0;

		//#		define ROUND_SIXTEEN(x) ((x) & 0x7) != 0 ? ((x) & ~0x7)+0x10 : (x)
		//		if(vs->frameUniformBufferLocation != -1) {
		//			bufferDesc.ByteWidth = ROUND_SIXTEEN(vs->engineFrameUniformBytes);
		//			if(MAKI_D3D_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &gvs->perFrameConstants))) {
		//				goto failed;
		//			}
		//		}
		//		if(vs->objectUniformBufferLocation != -1) {
		//			bufferDesc.ByteWidth = ROUND_SIXTEEN(vs->engineObjectUniformBytes);
		//			if(MAKI_D3D_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &gvs->perObjectConstants))) {
		//				goto failed;
		//			}
		//		}
		//		if(vs->materialUniformBufferLocation != -1) {
		//			bufferDesc.ByteWidth = ROUND_SIXTEEN(vs->materialUniformBytes);
		//			if(MAKI_D3D_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &gvs->materialConstants))) {
		//				goto failed;
		//			}
		//		}
		//#		undef ROUND_SIXTEEN

		//		vs->handle = (intptr_t)gvs;
		//		return true;

		//failed:
		//		SAFE_DELETE(gvs);
				return false;
			}

			bool RenderCoreImpl::CreateShaderProgram(ShaderProgram *s)
			{
				std::lock_guard<std::mutex> lock(mutex);

				if(!CreateVertexShader(&s->vertexShader)) {
					return false;
				}

				if(!CreatePixelShader(&s->pixelShader)) {
					return false;
				}

				return true;
			}

	

			bool RenderCoreImpl::CreateEmptyTexture(Texture *t, uint8 channels)
			{
				std::lock_guard<std::mutex> lock(mutex);

				/*ID3D11Texture2D *tex = nullptr;
				ID3D11ShaderResourceView *shaderResourceView = nullptr;
				ID3D11SamplerState *samplerState = nullptr;

				if(channels == 0 || channels > 4 || channels == 3) {
					Console::Error("Unsupported number of channels in image: %d", channels);
					goto failed;
				}

				D3D11_SAMPLER_DESC samplerDesc;
				ZeroMemory(&samplerDesc, sizeof(samplerDesc));
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
				samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
				samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
				samplerDesc.MinLOD = 0;
				samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
				if(MAKI_D3D_FAILED(device->CreateSamplerState(&samplerDesc, &samplerState))) {
					Console::Error("CreateSamplerState failed");
					goto failed;
				}

				D3D11_TEXTURE2D_DESC textureDesc;
				ZeroMemory(&textureDesc, sizeof(textureDesc));
				textureDesc.Width = t->width;
				textureDesc.Height = t->height;
				textureDesc.Format = channelsToFormat[channels];
				textureDesc.MipLevels = 1;
				textureDesc.ArraySize = 1;
				textureDesc.SampleDesc.Count = 1;
				textureDesc.SampleDesc.Quality = 0;
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
				textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				textureDesc.CPUAccessFlags = 0;
				textureDesc.MiscFlags = 0;
				if(MAKI_D3D_FAILED(device->CreateTexture2D(&textureDesc, nullptr, &tex))) {
					Console::Error("CreateTexture2D failed");
					goto failed;
				}

				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
				ZeroMemory(&shaderResViewDesc, sizeof(shaderResViewDesc));
				shaderResViewDesc.Format = textureDesc.Format;
				shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				shaderResViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
				if(MAKI_D3D_FAILED(device->CreateShaderResourceView(tex, &shaderResViewDesc, &shaderResourceView))) {
					Console::Error("CreateShaderResourceView failed");
					goto failed;
				}
		
		

				GPUTexture *gtex = new GPUTexture();
				gtex->shaderResourceView = shaderResourceView;
				gtex->samplerState = samplerState;
				t->handle = (intptr_t)gtex;

				SAFE_RELEASE(tex);
				return true;

		failed:		
				SAFE_RELEASE(tex);
				SAFE_RELEASE(shaderResourceView);
				SAFE_RELEASE(samplerState);*/
				return false;
			}

			bool RenderCoreImpl::CreateRenderTarget(Texture *t)
			{
				std::lock_guard<std::mutex> lock(mutex);

				/*ID3D11SamplerState *samplerState = nullptr;
				ID3D11Texture2D *tex = nullptr;
				ID3D11RenderTargetView *renderTargetView = nullptr;
				ID3D11ShaderResourceView *shaderResourceView = nullptr;


				D3D11_SAMPLER_DESC samplerDesc;
				ZeroMemory(&samplerDesc, sizeof(samplerDesc));
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
				samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
				samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
				samplerDesc.MinLOD = 0;
				samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
				if(MAKI_D3D_FAILED(device->CreateSamplerState(&samplerDesc, &samplerState))) {
					Console::Error("CreateSamplerState failed");
					goto failed;
				}

				D3D11_TEXTURE2D_DESC textureDesc;
				ZeroMemory(&textureDesc, sizeof(textureDesc));
				textureDesc.Width = t->width;
				textureDesc.Height = t->height;
				textureDesc.MipLevels = 1;
				textureDesc.ArraySize = 1;
				textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				textureDesc.SampleDesc.Count = 1;
				textureDesc.SampleDesc.Quality = 0;
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
				textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
				textureDesc.CPUAccessFlags = 0;
				textureDesc.MiscFlags = 0;
				if(MAKI_D3D_FAILED(device->CreateTexture2D(&textureDesc, nullptr, &tex))) {
					Console::Error("CreateTexture2D failed");
					goto failed;
				}

				D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
				ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
				renderTargetViewDesc.Format = textureDesc.Format;
				renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				if(MAKI_D3D_FAILED(device->CreateRenderTargetView(tex, &renderTargetViewDesc, &renderTargetView))) {
					Console::Error("CreateRenderTargetView failed");
					goto failed;
				}

				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
				ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
				shaderResourceViewDesc.Format = textureDesc.Format;
				shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				shaderResourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
				if(MAKI_D3D_FAILED(device->CreateShaderResourceView(tex, &shaderResourceViewDesc, &shaderResourceView))) {
					Console::Error("CreateShaderResourceView failed");
					goto failed;
				}
		
				GPUTexture *gtex = new GPUTexture();
				gtex->shaderResourceView = shaderResourceView;
				gtex->samplerState = samplerState;
				gtex->renderTargetView = renderTargetView;
				t->handle = (intptr_t)gtex;

				SAFE_RELEASE(tex);
				return true;

		failed:		
				SAFE_RELEASE(tex);
				SAFE_RELEASE(shaderResourceView);
				SAFE_RELEASE(samplerState);
				SAFE_RELEASE(renderTargetView);*/
				return false;
			}

			bool RenderCoreImpl::CreateDepthTexture(Texture *t)
			{
				std::lock_guard<std::mutex> lock(mutex);

				/*ID3D11SamplerState *samplerState = nullptr;
				ID3D11Texture2D *tex = nullptr;
				ID3D11DepthStencilView *depthStencilView = nullptr;
				ID3D11ShaderResourceView *shaderResourceView = nullptr;


				D3D11_SAMPLER_DESC samplerDesc;
				ZeroMemory(&samplerDesc, sizeof(samplerDesc));
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
				samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
				samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
				samplerDesc.BorderColor[0] = 1.0f;
				samplerDesc.BorderColor[1] = 1.0f;
				samplerDesc.BorderColor[2] = 1.0f;
				samplerDesc.BorderColor[3] = 1.0f;

				if(MAKI_D3D_FAILED(device->CreateSamplerState(&samplerDesc, &samplerState))) {
					Console::Error("CreateSamplerState failed");
					goto failed;
				}

				D3D11_TEXTURE2D_DESC textureDesc;
				ZeroMemory(&textureDesc, sizeof(textureDesc));
				textureDesc.Width = t->width;
				textureDesc.Height = t->height;
				textureDesc.MipLevels = 1;
				textureDesc.ArraySize = 1;
				textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
				textureDesc.SampleDesc.Count = 1;
				textureDesc.SampleDesc.Quality = 0;
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
				textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE;
				textureDesc.CPUAccessFlags = 0;
				textureDesc.MiscFlags = 0;
				if(MAKI_D3D_FAILED(device->CreateTexture2D(&textureDesc, nullptr, &tex))) {
					Console::Error("CreateTexture2D failed");
					goto failed;
				}

				D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
				ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
				depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
				depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				if(MAKI_D3D_FAILED(device->CreateDepthStencilView(tex, &depthStencilViewDesc, &depthStencilView))) {
					Console::Error("CreateDepthStencilView failed");
					goto failed;
				}

				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
				ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
				shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
				shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				shaderResourceViewDesc.Texture2D.MipLevels = 1;
				if(MAKI_D3D_FAILED(device->CreateShaderResourceView(tex, &shaderResourceViewDesc, &shaderResourceView))) {
					Console::Error("CreateShaderResourceView failed");
					goto failed;
				}
		
				GPUTexture *gtex = new GPUTexture();
				gtex->shaderResourceView = shaderResourceView;
				gtex->samplerState = samplerState;
				gtex->depthStencilView = depthStencilView;
				t->handle = (intptr_t)gtex;

				SAFE_RELEASE(tex);
				return true;

		failed:		
				SAFE_RELEASE(tex);
				SAFE_RELEASE(shaderResourceView);
				SAFE_RELEASE(samplerState);
				SAFE_RELEASE(depthStencilView);*/
				return false;
			}

			bool RenderCoreImpl::CreateTexture(Texture *t, char *data, uint32 dataLength)
			{
				std::lock_guard<std::mutex> lock(mutex);

				/*ID3D11Texture2D *tex = nullptr;
				ID3D11ShaderResourceView *shaderResourceView = nullptr;
				ID3D11SamplerState *samplerState = nullptr;
		
				if(MAKI_D3D_FAILED(DirectX::CreateDDSTextureFromMemory(device, (uint8 *)data, dataLength, (ID3D11Resource **)&tex, &shaderResourceView))) {
					Console::Error("Failed to create texture from memory");
					goto failed;
				}

				D3D11_TEXTURE2D_DESC desc;
				tex->GetDesc(&desc);
				t->width = desc.Width;
				t->height = desc.Height;

				D3D11_SAMPLER_DESC samplerDesc;
				ZeroMemory(&samplerDesc, sizeof(samplerDesc));
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
				samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
				samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
				samplerDesc.MinLOD = 0;
				samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
				if(MAKI_D3D_FAILED(device->CreateSamplerState(&samplerDesc, &samplerState))) {
					Console::Error("CreateSamplerState failed");
					goto failed;
				}
		
				GPUTexture *gtex = new GPUTexture();
				gtex->shaderResourceView = shaderResourceView;
				gtex->samplerState = samplerState;
				t->handle = (intptr_t)gtex;

				SAFE_RELEASE(tex);
				return true;

		failed:		
				SAFE_RELEASE(tex);
				SAFE_RELEASE(shaderResourceView);
				SAFE_RELEASE(samplerState);*/
				return false;
			}

			void RenderCoreImpl::WriteToTexture(Texture *t, int32 dstX, int32 dstY, int32 srcX, int32 srcY, uint32 srcWidth, uint32 srcHeight, uint32 srcPitch, uint8 channels, char *srcData)
			{
				std::lock_guard<std::mutex> lock(mutex);
		
				/*GPUTexture *tex = (GPUTexture *)t->handle;

				D3D11_BOX box;
				box.left = dstX;
				box.right = dstX+srcWidth;
				box.top = dstY;
				box.bottom = dstY+srcHeight;
				box.front = 0;
				box.back = 1;

				char *p = srcData + (srcY*srcPitch) + (srcX*channels);

				ID3D11Resource *res = nullptr;
				tex->shaderResourceView->GetResource(&res);
				context->UpdateSubresource(res, 0, &box, p, srcPitch, srcPitch);
				SAFE_RELEASE(res);*/
			}

			void RenderCoreImpl::DeleteShaderProgram(ShaderProgram *s)
			{
				std::lock_guard<std::mutex> lock(mutex);

				/*GPUVertexShader *gvs = (GPUVertexShader *)s->vertexShader.handle;
				SAFE_DELETE(gvs);
				s->vertexShader.handle = (intptr_t)nullptr;

				GPUPixelShader *gps = (GPUPixelShader *)s->pixelShader.handle;
				SAFE_DELETE(gps);
				s->pixelShader.handle = (intptr_t)nullptr;*/
			}

			void RenderCoreImpl::DeleteTexture(Texture *t)
			{
				std::lock_guard<std::mutex> lock(mutex);

				/*GPUTexture *gtex = (GPUTexture *)t->handle;
				SAFE_DELETE(gtex);
				t->handle = (intptr_t)nullptr;*/
			}


		} // namespace OGL
	
	} // namespace Core

} // namespace Maki
