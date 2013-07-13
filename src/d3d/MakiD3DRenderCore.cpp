#include "d3d/d3d_stdafx.h"
#include "d3d/MakiD3DRenderCore.h"
#include "d3d/MakiD3DCommon.h"
#include "d3d/MakiD3DGPUTypes.h"
#include "d3d/DDSTextureLoader.h"
#include "SDL_syswm.h"

using namespace Maki::Core;

namespace Maki
{
	namespace D3D
	{

		D3DRenderCore::D3DRenderCore(Window *window, const Config *config)
			:	RenderCore(),
				swapChain(nullptr),
				device(nullptr),
				context(nullptr),
				defaultRenderTargetView(nullptr),
				defaultDepthStencilView(nullptr),
				currentRenderTargetView(nullptr),
				currentDepthStencilView(nullptr),
				rasterizerState(nullptr),
				rasterizerStateCullBack(nullptr),
				rasterizerStateCullFront(nullptr),
				rasterizerStateWireFrame(nullptr),
				rasterizerStateWireFrameCullBack(nullptr),
				rasterizerStateWireFrameCullFront(nullptr),
				depthState(nullptr),
				depthStateWrite(nullptr),
				depthStateLess(nullptr),
				depthStateLessWrite(nullptr),
				depthStateEqual(nullptr),
				depthStateEqualWrite(nullptr),
				depthStateLessEqual(nullptr),
				depthStateLessEqualWrite(nullptr),
				blendEnabled(nullptr),
				blendDisabled(nullptr)
		{
			memset(nullArray, 0, sizeof(nullArray));

			vsync = config->GetBool("engine.vertical_sync", true);
			maxVertexFormatsPerVertexShader = config->GetUint("engine.max_vertex_formats_per_vertex_shader", 6);
				
			// Get hwnd from SDL window
			SDL_SysWMinfo sdlInfo;
			SDL_version sdlVer;
			SDL_VERSION(&sdlVer);
			sdlInfo.version = sdlVer;
			SDL_GetWindowWMInfo(window->window, &sdlInfo);
			HWND hwnd = sdlInfo.info.win.window;

			// create a struct to hold information about the swap chain
			DXGI_SWAP_CHAIN_DESC scd;
			ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
			scd.BufferCount = 1;
			scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			scd.OutputWindow = hwnd;
			scd.SampleDesc.Count = 1;
			scd.SampleDesc.Quality = 0;
			scd.Windowed = !window->fullscreen;
			scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


			UINT flags = 0;
	#if _DEBUG
			flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif
			D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_0;
			HRESULT ret = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags|D3D11_CREATE_DEVICE_SINGLETHREADED, &featureLevels, 1,
				D3D11_SDK_VERSION, &scd, &swapChain, &device, nullptr, &context);
			if(MAKI_D3D_FAILED(ret)) {
				Console::Error("Failed to create device and swap chain");
			}
		}

		D3DRenderCore::~D3DRenderCore() {
			if(swapChain != nullptr) {
				swapChain->SetFullscreenState(false, nullptr);
			}

			SAFE_RELEASE(rasterizerState);
			SAFE_RELEASE(rasterizerStateCullBack);
			SAFE_RELEASE(rasterizerStateCullFront);
			SAFE_RELEASE(rasterizerStateWireFrame);
			SAFE_RELEASE(rasterizerStateWireFrameCullBack);
			SAFE_RELEASE(rasterizerStateWireFrameCullFront);

			SAFE_RELEASE(depthState);
			SAFE_RELEASE(depthStateWrite);
			SAFE_RELEASE(depthStateLess);
			SAFE_RELEASE(depthStateLessWrite);
			SAFE_RELEASE(depthStateEqual);
			SAFE_RELEASE(depthStateEqualWrite);
			SAFE_RELEASE(depthStateLessEqual);
			SAFE_RELEASE(depthStateLessEqualWrite);

			SAFE_RELEASE(blendEnabled);
			SAFE_RELEASE(blendDisabled);

			SAFE_RELEASE(defaultRenderTargetView);
			SAFE_RELEASE(defaultDepthStencilView);
			SAFE_RELEASE(swapChain);		
			SAFE_RELEASE(context);
			SAFE_RELEASE(device);

			Console::Info("Direct3D renderer destroyed");
		}

		void D3DRenderCore::Init()
		{
			std::lock_guard<std::mutex> lock(mutex);

			// Setup rasterizer state
			D3D11_RASTERIZER_DESC rasterizerDesc;
			ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
			rasterizerDesc.FillMode = D3D11_FILL_SOLID;
			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			rasterizerDesc.FrontCounterClockwise = true;
			rasterizerDesc.DepthBias = 0;
			rasterizerDesc.DepthBiasClamp = 0.0f;
			rasterizerDesc.SlopeScaledDepthBias = 0.0f;
			rasterizerDesc.DepthClipEnable = true;
			rasterizerDesc.ScissorEnable = false;
			rasterizerDesc.MultisampleEnable = false;
			rasterizerDesc.AntialiasedLineEnable = false;
			if(MAKI_D3D_FAILED(device->CreateRasterizerState(&rasterizerDesc, &rasterizerState))) {
				Console::Error("Failed to create rasterizer state");
			}
			rasterizerDesc.CullMode = D3D11_CULL_BACK;
			if(MAKI_D3D_FAILED(device->CreateRasterizerState(&rasterizerDesc, &rasterizerStateCullBack))) {
				Console::Error("Failed to create rasterizer state (cull back)");
			}
			rasterizerDesc.CullMode = D3D11_CULL_FRONT;
			if(MAKI_D3D_FAILED(device->CreateRasterizerState(&rasterizerDesc, &rasterizerStateCullFront))) {
				Console::Error("Failed to create rasterizer state (cull front)");
			}
			rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			if(MAKI_D3D_FAILED(device->CreateRasterizerState(&rasterizerDesc, &rasterizerStateWireFrame))) {
				Console::Error("Failed to create rasterizer state (wire frame)");
			}
			rasterizerDesc.CullMode = D3D11_CULL_BACK;
			if(MAKI_D3D_FAILED(device->CreateRasterizerState(&rasterizerDesc, &rasterizerStateWireFrameCullBack))) {
				Console::Error("Failed to create rasterizer state (wire frame, cull back)");
			}
			rasterizerDesc.CullMode = D3D11_CULL_FRONT;
			if(MAKI_D3D_FAILED(device->CreateRasterizerState(&rasterizerDesc, &rasterizerStateWireFrameCullFront))) {
				Console::Error("Failed to create rasterizer state (wire frame, cull front)");
			}

			// Set up depth stencil state
			D3D11_DEPTH_STENCIL_DESC depthDesc;
			ZeroMemory(&depthDesc, sizeof(depthDesc));
			depthDesc.DepthEnable = false;
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthDesc.DepthFunc = D3D11_COMPARISON_NEVER;
			depthDesc.StencilEnable = false;
			if(MAKI_D3D_FAILED(device->CreateDepthStencilState(&depthDesc, &depthState))) {
				Console::Error("Failed to create depth stencil state");
			}
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			if(MAKI_D3D_FAILED(device->CreateDepthStencilState(&depthDesc, &depthStateWrite))) {
				Console::Error("Failed to create depth stencil state");
			}
			depthDesc.DepthEnable = true;
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
			if(MAKI_D3D_FAILED(device->CreateDepthStencilState(&depthDesc, &depthStateLess))) {
				Console::Error("Failed to create depth stencil state");
			}
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			if(MAKI_D3D_FAILED(device->CreateDepthStencilState(&depthDesc, &depthStateLessWrite))) {
				Console::Error("Failed to create depth stencil state");
			}
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
			if(MAKI_D3D_FAILED(device->CreateDepthStencilState(&depthDesc, &depthStateEqual))) {
				Console::Error("Failed to create depth stencil state");
			}
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			if(MAKI_D3D_FAILED(device->CreateDepthStencilState(&depthDesc, &depthStateEqualWrite))) {
				Console::Error("Failed to create depth stencil state");
			}
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			if(MAKI_D3D_FAILED(device->CreateDepthStencilState(&depthDesc, &depthStateLessEqual))) {
				Console::Error("Failed to create depth stencil state");
			}
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			if(MAKI_D3D_FAILED(device->CreateDepthStencilState(&depthDesc, &depthStateLessEqualWrite))) {
				Console::Error("Failed to create depth stencil state");
			}


			// Setup blend state
			D3D11_BLEND_DESC blendDesc;
			ZeroMemory(&blendDesc, sizeof(blendDesc));
			blendDesc.RenderTarget[0].BlendEnable = true;
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;
			if(MAKI_D3D_FAILED(device->CreateBlendState(&blendDesc, &blendEnabled))) {
				Console::Error("Failed to create blend state (enabled)");
			}
			blendDesc.RenderTarget[0].BlendEnable = false;
			if(MAKI_D3D_FAILED(device->CreateBlendState(&blendDesc, &blendDisabled))) {
				Console::Error("Failed to create blend state (disabled)");
			}
		

			// Set initial state:
			windowWidth = 1;
			windowHeight = 1;

			context->RSSetState(rasterizerState);
			context->OMSetBlendState(blendEnabled, nullptr, 0xffffffff);
			context->OMSetDepthStencilState(depthStateLessWrite, 1);

			Resized(windowWidth, windowHeight);

			// Render a blank frame so we don't see a flash of white on startup
			SetRenderTargetAndDepthStencil(RenderState::RenderTarget_Default, HANDLE_NONE, RenderState::DepthStencil_Null, HANDLE_NONE);
			float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
			context->ClearRenderTargetView(defaultRenderTargetView, color);
			swapChain->Present(0, 0);
		}

		void D3DRenderCore::Resized(uint32 newWidth, uint32 newHeight)
		{
			context->OMSetRenderTargets(0, nullptr, nullptr);
			SAFE_RELEASE(defaultRenderTargetView);
			SAFE_RELEASE(defaultDepthStencilView);

			// ResizeBuffers breaks the graphics diagnostics, so don't call it while using the graphics profiler
	#if !MAKI_PROFILING
			if(MAKI_D3D_FAILED(swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0))) {
				Console::Error("Failed to resize swap chain buffers");
			}
	#endif
		
			// Setup render target
			ID3D11Texture2D *renderBuffer = nullptr;
			if(MAKI_D3D_FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&renderBuffer))) {
				Console::Error("Failed to get buffer from swap chain for render target view");
			}
			if(MAKI_D3D_FAILED(device->CreateRenderTargetView(renderBuffer, nullptr, &defaultRenderTargetView))) {
				Console::Error("Failed to create render target view");
			}
			SAFE_RELEASE(renderBuffer);
		

			// Setup depth buffer
			D3D11_TEXTURE2D_DESC depthDesc;
			ZeroMemory(&depthDesc, sizeof(depthDesc));
			depthDesc.Width = newWidth;
			depthDesc.Height = newHeight;
			depthDesc.MipLevels = 1;
			depthDesc.ArraySize = 1;
			depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
			depthDesc.SampleDesc.Count = 1;
			depthDesc.SampleDesc.Quality = 0;
			depthDesc.Usage = D3D11_USAGE_DEFAULT;
			depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			depthDesc.CPUAccessFlags = 0;
			depthDesc.MiscFlags = 0;
		
			D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
			ZeroMemory(&depthViewDesc, sizeof(depthViewDesc));
			depthViewDesc.Format = depthDesc.Format;
			depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depthViewDesc.Texture2D.MipSlice = 0;

			ID3D11Texture2D *depthBuffer = nullptr;
			if(MAKI_D3D_FAILED(device->CreateTexture2D(&depthDesc, nullptr, &depthBuffer))) {
				Console::Error("Failed to create texture for depth buffer");
			}
			if(MAKI_D3D_FAILED(device->CreateDepthStencilView(depthBuffer, &depthViewDesc, &defaultDepthStencilView))) {
				Console::Error("Failed to create depth stencil view");
			}
			SAFE_RELEASE(depthBuffer);
		}

		void D3DRenderCore::Present()
		{
			std::lock_guard<std::mutex> lock(mutex);
			swapChain->Present(vsync ? 1 : 0, 0);
		}

		void D3DRenderCore::Draw(const RenderState &state, const DrawCommandList &commands)
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
			D3D11_VIEWPORT viewport;
			ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
			viewport.TopLeftX = state.viewPortRect.left;
			viewport.TopLeftY = state.viewPortRect.top;
			viewport.Width = state.viewPortRect.GetWidth();
			viewport.Height = state.viewPortRect.GetHeight();
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			context->RSSetViewports(1, &viewport);

			// Clear depth if necessary
			if(state.clearDepthStencil && currentDepthStencilView != nullptr) {
				context->ClearDepthStencilView(currentDepthStencilView, D3D11_CLEAR_DEPTH, state.depthClearValue, 0);
			}

			// Clear render target if necessary
			if(state.clearRenderTarget && currentRenderTargetView != nullptr) {
				context->ClearRenderTargetView(currentRenderTargetView, state.renderTargetClearValue.vals);
			}

			// Depth test / depth write
			SetDepthState(state.depthTest, state.depthWrite);

			// Cull mode / wire frame mode
			SetRasterizerState(state.cullMode, state.wireFrame);

			const Buffer *currentBuffer = nullptr;
			uint32 currentLayer = 0;
		
			uint32 currentTranslucencyType = DrawCommand::TranslucencyType_Opaque;
			context->OMSetBlendState(blendDisabled, nullptr, 0xffffffff);

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
						SetDepthState(state.depthTest, false);
						context->OMSetBlendState(blendEnabled, nullptr, 0xffffffff);
					} else {
						// Disable blending, restore depth write
						SetDepthState(state.depthTest, state.depthWrite);
						context->OMSetBlendState(blendDisabled, nullptr, 0xffffffff);
					}
					currentTranslucencyType = dc->fields.translucencyType;
				}

				if(currentVertexFormat != dc->vertexFormat) {
					currentVertexFormat = dc->vertexFormat;
					setLayout = true;
				}

					
				D3D11_MAPPED_SUBRESOURCE mapped;
				const GPUVertexShader *gvs = (GPUVertexShader *)shader->vertexShader.handle;
				const GPUPixelShader *gps = (GPUPixelShader *)shader->pixelShader.handle;

				if(currentShaderProgram != dc->shaderProgram) {
				
					// Unbind all textures from current shader
					if(currentShaderProgram != HANDLE_NONE) {
						context->PSSetShaderResources(0, SHADOW_MAP_SLOT_INDEX_START+RenderState::MAX_SHADOW_LIGHTS, (ID3D11ShaderResourceView **)nullArray);
					}

					context->VSSetShader(gvs->vs, nullptr, 0);
					context->PSSetShader(gps->ps, nullptr, 0);
					currentShaderProgram = dc->shaderProgram;

					// Set per-frame vertex constants
					if(shader->vertexShader.frameUniformBufferLocation != -1) {
						context->Map(gvs->perFrameConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
						SetPerFrameConstants(state, &shader->vertexShader, mapped);
						context->Unmap(gvs->perFrameConstants, 0);
						context->VSSetConstantBuffers(shader->vertexShader.frameUniformBufferLocation, 1, &gvs->perFrameConstants);
					}

					// Set per-frame pixel constants
					if(shader->pixelShader.frameUniformBufferLocation != -1) {
						context->Map(gps->perFrameConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
						SetPerFrameConstants(state, &shader->pixelShader, mapped);
						context->Unmap(gps->perFrameConstants, 0);
						context->PSSetConstantBuffers(shader->pixelShader.frameUniformBufferLocation, 1, &gps->perFrameConstants);
					}

					// Bind shadow map resources
					ID3D11ShaderResourceView *shadowViews[RenderState::MAX_SHADOW_LIGHTS];
					ID3D11SamplerState *shadowSamplers[RenderState::MAX_SHADOW_LIGHTS];
					for(uint8 i = 0; i < RenderState::MAX_SHADOW_LIGHTS; i++) {
						if(state.shadowMaps[i] != HANDLE_NONE) {
							GPUTexture *tex = (GPUTexture *)TextureManager::Get(state.shadowMaps[i])->handle;
							shadowViews[i] = tex->shaderResourceView;
							shadowSamplers[i] = tex->samplerState;
						} else {
							shadowViews[i] = nullptr;
							shadowSamplers[i] = nullptr;
						}
					}
					context->PSSetShaderResources(SHADOW_MAP_SLOT_INDEX_START, RenderState::MAX_SHADOW_LIGHTS, shadowViews);
					context->PSSetSamplers(SHADOW_MAP_SLOT_INDEX_START, RenderState::MAX_SHADOW_LIGHTS, shadowSamplers);

					currentMaterial = HANDLE_NONE;
					currentTextureSet = HANDLE_NONE;
					currentBuffer = nullptr;
					setLayout = true;
				}

				// Get or create the input layout for this vertexformat+vertexshader combination
				if(setLayout) {
					ID3D11InputLayout *layout = ((GPUVertexShader *)shader->vertexShader.handle)->GetOrCreateInputLayout(device, vf);
					context->IASetInputLayout(layout);
					setLayout = false;
				}

				if(currentMaterial != dc->material) {
					Material *mat = MaterialManager::Get(dc->material);

					if(shader->vertexShader.materialUniformBufferLocation != -1) {
						context->Map(gvs->materialConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
						BindMaterialConstants(&shader->vertexShader, true, mapped, mat);
						context->Unmap(gvs->materialConstants, 0);
						context->VSSetConstantBuffers(shader->vertexShader.materialUniformBufferLocation, 1, &gvs->materialConstants);
					}
					if(shader->pixelShader.materialUniformBufferLocation != -1) {
						context->Map(gps->materialConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
						BindMaterialConstants(&shader->pixelShader, false, mapped, mat);
						context->Unmap(gps->materialConstants, 0);
						context->PSSetConstantBuffers(shader->pixelShader.materialUniformBufferLocation, 1, &gps->materialConstants);
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
						context->Map(gvs->perObjectConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
						SetPerObjectConstants(&shader->vertexShader, mapped, matrix, mv, mvp);
						context->Unmap(gvs->perObjectConstants, 0);
						context->VSSetConstantBuffers(shader->vertexShader.objectUniformBufferLocation, 1, &gvs->perObjectConstants);
					}

					// Set per-object pixel constants
					if(shader->pixelShader.objectUniformBufferLocation != -1) {
						context->Map(gps->perObjectConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
						SetPerObjectConstants(&shader->pixelShader, mapped, matrix, mv, mvp);
						context->Unmap(gps->perObjectConstants, 0);
						context->PSSetConstantBuffers(shader->pixelShader.objectUniformBufferLocation, 1, &gps->perObjectConstants);
					}
				}

				// Bind buffers again if they have changed
				const Buffer *b = (Buffer *)MeshManager::Get(dc->mesh)->GetBuffer();
				if(currentBuffer != b) {
					uint32 stride = vf->GetStride();
					uint32 offset = 0;
					context->IASetVertexBuffers(0, 1, &b->vbos[0], &stride, &offset);
					context->IASetIndexBuffer(b->vbos[1], (DXGI_FORMAT)b->indexDataType, 0);
					currentBuffer = b;
				}

				context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)b->geometryType);
				context->DrawIndexed(b->indicesPerFace*b->faceCount, 0, 0);
			}

			// Unbind all textures from current shader
			if(currentShaderProgram != HANDLE_NONE) {
				context->PSSetShaderResources(0, SHADOW_MAP_SLOT_INDEX_START+RenderState::MAX_SHADOW_LIGHTS, (ID3D11ShaderResourceView **)nullArray);
			}

			currentRenderTargetView = nullptr;
			currentDepthStencilView = nullptr;
			context->OMSetRenderTargets(0, nullptr, nullptr);
		}

		void D3DRenderCore::SetDepthState(RenderState::DepthTest test, bool write)
		{
			switch(test) {
			case RenderState::DepthTest_Less:
				context->OMSetDepthStencilState(write ? depthStateLessWrite : depthStateLess, 1);
				break;
			case RenderState::DepthTest_Equal:
				context->OMSetDepthStencilState(write ? depthStateEqualWrite : depthStateEqual, 1);
				break;
			case RenderState::DepthTest_LessEqual:
				context->OMSetDepthStencilState(write ? depthStateLessEqualWrite : depthStateLessEqual, 1);
				break;
			case RenderState::DepthTest_Disabled:
			default:
				context->OMSetDepthStencilState(write ? depthStateWrite : depthState, 1);
				break;
			}
		}

		void D3DRenderCore::SetRasterizerState(RenderState::CullMode cullMode, bool wireFrame)
		{
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

		void D3DRenderCore::SetRenderTargetAndDepthStencil(RenderState::RenderTarget renderTargetType, Handle renderTarget, RenderState::DepthStencil depthStencilType, Handle depthStencil)
		{
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

		void D3DRenderCore::SetPerFrameConstants(const RenderState &state, const Shader *s, D3D11_MAPPED_SUBRESOURCE &mapped)
		{
			int32 location = s->engineFrameUniformLocations[Shader::FrameUniform_View];
			if(location != -1) {
				memcpy(((char *)mapped.pData) + location, state.view.vals, 16*sizeof(float));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_Projection];
			if(location != -1) {
				memcpy(((char *)mapped.pData) + location, state.projection.vals, sizeof(state.projection));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_CameraWithHeightNearFar];
			if(location != -1) {
				memcpy(((char *)mapped.pData) + location, &state.cameraWidthHeightNearFar, sizeof(state.cameraWidthHeightNearFar));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_CameraSplitDistances];
			if(location != -1) {
				memcpy(((char *)mapped.pData) + location, &state.cameraSplitDistances, sizeof(state.cameraSplitDistances));
			}


			location = s->engineFrameUniformLocations[Shader::FrameUniform_LightViewProj];
			if(location != -1) {
				memcpy(((char *)mapped.pData) + location, state.lightViewProj, sizeof(state.lightViewProj));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_LightPositions];
			if(location != -1) {
				memcpy(((char *)mapped.pData) + location, state.lightPositions, sizeof(state.lightPositions));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_LightDirections];
			if(location != -1) {
				memcpy(((char *)mapped.pData) + location, state.lightDirections, sizeof(state.lightDirections));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_LightProperties];
			if(location != -1) {
				// Set all lighting slots here so that lights which are no longer in use get effectively turned off
				memcpy(((char *)mapped.pData) + location, state.lightProperties, sizeof(state.lightProperties));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_ShadowMapProperties];
			if(location != -1) {
				memcpy(((char *)mapped.pData) + location, state.shadowMapProperties, state.shadowLightCount*sizeof(RenderState::ShadowMapProperties));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_LightSplitRegions];
			if(location != -1) {
				memcpy(((char *)mapped.pData) + location, state.lightSplitRegions, state.cascadedShadowLightCount*RenderState::MAX_CASCADES*sizeof(RenderState::LightSplitRegion));
			}
		
			location = s->engineFrameUniformLocations[Shader::FrameUniform_GlobalAmbientColor];
			if(location != -1) {
				memcpy(((char *)mapped.pData) + location, &state.globalAmbientColor.x, sizeof(state.globalAmbientColor));
			}
		}

		void D3DRenderCore::SetPerObjectConstants(const Shader *s, D3D11_MAPPED_SUBRESOURCE &mapped, const Matrix44 &model, const Matrix44 &modelView, const Matrix44 &modelViewProjection)
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
		}

		void D3DRenderCore::BindMaterialConstants(const Shader *s, bool isVertexShader, D3D11_MAPPED_SUBRESOURCE &mapped, const Material *mat)
		{
			for(uint8 i = 0; i < mat->uniformCount; i++) {
				const Material::UniformValue &val = mat->uniformValues[i];
				int32 location = isVertexShader ? val.vsLocation : val.psLocation;
				if(location != -1) {
					memcpy(((char *)mapped.pData) + location, val.data, val.bytes);
				}
			}
		}

		void D3DRenderCore::BindTextures(const ShaderProgram *shader, const TextureSet *ts)
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
		}









		// Resource creation, deletion, modification:

		void *D3DRenderCore::UploadBuffer(void *buffer, VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic)
		{
			std::lock_guard<std::mutex> lock(mutex);

			Buffer *b = (Buffer *)buffer;
			if(b == nullptr) {
				b = new Buffer();
				memset(b, 0, sizeof(Buffer));
				buffer = b;
			} else {
				SAFE_RELEASE(b->vbos[0]);
				SAFE_RELEASE(b->vbos[1]);
			}

			b->vertexCount = vertexCount;
			b->faceCount = faceCount;
			b->indicesPerFace = indicesPerFace;
			b->bytesPerIndex = bytesPerIndex;

			assert(indicesPerFace > 0 && indicesPerFace <= 3);
			b->geometryType = indicesPerFaceToGeometryType[indicesPerFace];
		
			assert(bytesPerIndex > 0 && bytesPerIndex <= 4 && bytesPerIndex != 3);
			b->indexDataType = bytesPerIndexToFormat[bytesPerIndex];

			// Create vertex buffer
			D3D11_SUBRESOURCE_DATA srd;
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			ZeroMemory(&srd, sizeof(srd));
			bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
			bd.ByteWidth = vf->GetStride()*vertexCount;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			if(dynamic) {
				bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}
			srd.pSysMem = vertexData;
			if(MAKI_D3D_FAILED(device->CreateBuffer(&bd, &srd, &b->vbos[0]))) {
				Console::Error("Failed to create d3d vertex buffer");
			}
		
			// Create index buffer
			ZeroMemory(&bd, sizeof(bd));
			ZeroMemory(&srd, sizeof(srd));
			bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
			bd.ByteWidth = bytesPerIndex*indicesPerFace*faceCount;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			if(dynamic) {
				bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}
			srd.pSysMem = indexData;
			if(MAKI_D3D_FAILED(device->CreateBuffer(&bd, &srd, &b->vbos[1]))) {
				Console::Error("Failed to create d3d index buffer");
			}

			return buffer;
		}

		void D3DRenderCore::FreeBuffer(void *buffer)
		{
			std::lock_guard<std::mutex> lock(mutex);

			if(buffer != nullptr) {
				Buffer *b = (Buffer *)buffer;
				SAFE_RELEASE(b->vbos[0]);
				SAFE_RELEASE(b->vbos[1]);
				delete b;
			}
		}

		bool D3DRenderCore::CreatePixelShader(Shader *ps)
		{
			GPUPixelShader *gps = new GPUPixelShader();

			if(MAKI_D3D_FAILED(device->CreatePixelShader(ps->programData, ps->programDataBytes, nullptr, &gps->ps))) {
				goto failed;
			}

			// Allocate buffers for constants
			D3D11_BUFFER_DESC bufferDesc;
			ZeroMemory(&bufferDesc, sizeof(bufferDesc));
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;

	#		define ROUND_SIXTEEN(x) ((x) & 0x7) != 0 ? ((x) & ~0x7)+0x10 : (x)
			if(ps->frameUniformBufferLocation != -1) {
				bufferDesc.ByteWidth = ROUND_SIXTEEN(ps->engineFrameUniformBytes);
				if(MAKI_D3D_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &gps->perFrameConstants))) {
					goto failed;
				}
			}
			if(ps->objectUniformBufferLocation != -1) {
				bufferDesc.ByteWidth = ROUND_SIXTEEN(ps->engineObjectUniformBytes);
				if(MAKI_D3D_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &gps->perObjectConstants))) {
					goto failed;
				}
			}
			if(ps->materialUniformBufferLocation != -1) {
				bufferDesc.ByteWidth = ROUND_SIXTEEN(ps->materialUniformBytes);
				if(MAKI_D3D_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &gps->materialConstants))) {
					goto failed;
				}
			}
	#		undef ROUND_SIXTEEN

			ps->handle = (intptr_t)gps;
			return true;

	failed:
			SAFE_DELETE(gps);
			return false;
		}

		bool D3DRenderCore::CreateVertexShader(Shader *vs)
		{
			GPUVertexShader *gvs = new GPUVertexShader(maxVertexFormatsPerVertexShader);

			// Since vertex shader keeps a pointer to this data, we will not delete the buffer as we exit the function
			char *blob = (char *)Allocator::Malloc(vs->programDataBytes);
			memcpy(blob, vs->programData, vs->programDataBytes);
			gvs->blob = blob;
			gvs->blobSize = vs->programDataBytes;
		
			if(MAKI_D3D_FAILED(device->CreateVertexShader(gvs->blob, gvs->blobSize, nullptr, &gvs->vs))) {
				goto failed;
			}

			// Allocate buffers for constants
			D3D11_BUFFER_DESC bufferDesc;
			ZeroMemory(&bufferDesc, sizeof(bufferDesc));
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;

	#		define ROUND_SIXTEEN(x) ((x) & 0x7) != 0 ? ((x) & ~0x7)+0x10 : (x)
			if(vs->frameUniformBufferLocation != -1) {
				bufferDesc.ByteWidth = ROUND_SIXTEEN(vs->engineFrameUniformBytes);
				if(MAKI_D3D_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &gvs->perFrameConstants))) {
					goto failed;
				}
			}
			if(vs->objectUniformBufferLocation != -1) {
				bufferDesc.ByteWidth = ROUND_SIXTEEN(vs->engineObjectUniformBytes);
				if(MAKI_D3D_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &gvs->perObjectConstants))) {
					goto failed;
				}
			}
			if(vs->materialUniformBufferLocation != -1) {
				bufferDesc.ByteWidth = ROUND_SIXTEEN(vs->materialUniformBytes);
				if(MAKI_D3D_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &gvs->materialConstants))) {
					goto failed;
				}
			}
	#		undef ROUND_SIXTEEN

			vs->handle = (intptr_t)gvs;
			return true;

	failed:
			SAFE_DELETE(gvs);
			return false;
		}

		bool D3DRenderCore::CreateShaderProgram(ShaderProgram *s)
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

	

		bool D3DRenderCore::CreateEmptyTexture(Texture *t, uint8 channels)
		{
			std::lock_guard<std::mutex> lock(mutex);

			ID3D11Texture2D *tex = nullptr;
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
			SAFE_RELEASE(samplerState);
			return false;
		}

		bool D3DRenderCore::CreateRenderTarget(Texture *t)
		{
			std::lock_guard<std::mutex> lock(mutex);

			ID3D11SamplerState *samplerState = nullptr;
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
			SAFE_RELEASE(renderTargetView);
			return false;
		}

		bool D3DRenderCore::CreateDepthTexture(Texture *t)
		{
			std::lock_guard<std::mutex> lock(mutex);

			ID3D11SamplerState *samplerState = nullptr;
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
			SAFE_RELEASE(depthStencilView);
			return false;
		}

		bool D3DRenderCore::CreateTexture(Texture *t, char *data, uint32 dataLength)
		{
			std::lock_guard<std::mutex> lock(mutex);

			ID3D11Texture2D *tex = nullptr;
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
			SAFE_RELEASE(samplerState);
			return false;
		}

		void D3DRenderCore::WriteToTexture(Texture *t, int32 dstX, int32 dstY, int32 srcX, int32 srcY, uint32 srcWidth, uint32 srcHeight, uint32 srcPitch, uint8 channels, char *srcData)
		{
			std::lock_guard<std::mutex> lock(mutex);
		
			GPUTexture *tex = (GPUTexture *)t->handle;

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
			SAFE_RELEASE(res);
		}

		void D3DRenderCore::DeleteShaderProgram(ShaderProgram *s)
		{
			std::lock_guard<std::mutex> lock(mutex);

			GPUVertexShader *gvs = (GPUVertexShader *)s->vertexShader.handle;
			SAFE_DELETE(gvs);
			s->vertexShader.handle = (intptr_t)nullptr;

			GPUPixelShader *gps = (GPUPixelShader *)s->pixelShader.handle;
			SAFE_DELETE(gps);
			s->pixelShader.handle = (intptr_t)nullptr;
		}

		void D3DRenderCore::DeleteTexture(Texture *t)
		{
			std::lock_guard<std::mutex> lock(mutex);

			GPUTexture *gtex = (GPUTexture *)t->handle;
			SAFE_DELETE(gtex);
			t->handle = (intptr_t)nullptr;
		}







	#if 0
		void D3DRenderCore::GetAdapterInfo(uint32 windowWidth, uint32 windowHeight) {
			IDXGIFactory *factory = nullptr;
			IDXGIAdapter *adapter = nullptr;
			IDXGIOutput *adapterOutput = nullptr;
			uint32 modeCount;
			DXGI_MODE_DESC *modeList = nullptr;
			uint32 refreshNumer = 0, refreshDenom = 0;
			DXGI_ADAPTER_DESC adapterDesc;

			if(MAKI_D3D_FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory))) {
				Console::Error("Failed to create dxgi factory");
				goto done;
			}
			if(MAKI_D3D_FAILED(factory->EnumAdapters(0, &adapter))) {
				Console::Error("Failed to enum adapters");
				goto done;
			}
			if(MAKI_D3D_FAILED(adapter->EnumOutputs(0, &adapterOutput))) {
				Console::Error("Failed to enum outputs");
				goto done;
			}
			if(MAKI_D3D_FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &modeCount, nullptr))) {
				Console::Error("Failed to get display mode list size");
				goto done;
			}
			modeList = new DXGI_MODE_DESC[modeCount];
			if(MAKI_D3D_FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &modeCount, modeList))) {
				Console::Error("Failed to get display mode list");
				goto done;
			}
				
			bool found = false;
			for(uint32 i = 0; i < modeCount; i++) {
				if(modeList[i].Width == windowWidth && modeList[i].Height == windowHeight)
				{
					refreshNumer = modeList[i].RefreshRate.Numerator;
					refreshDenom = modeList[i].RefreshRate.Denominator;
					Console::Info("Supported mode: %dx%d @ %f Hz", windowWidth, windowHeight, refreshDenom != 0 ? refreshNumer / (float)refreshDenom : 0.0f); 
					found = true;
				}
			}
			if(!found) {
				Console::Warning("Could not find desired resolution in display mode list");
			}

			if(MAKI_D3D_FAILED(adapter->GetDesc(&adapterDesc))) {
				Console::Error("Failed to get adapter descriptor");
				goto done;
			}

			Console::Info("VRAM: %dmb", adapterDesc.DedicatedVideoMemory / 1024 / 1024);
		
			// Convert the name of the video card to a character array and store it.
			size_t cardDescriptionLength;
			char cardDescription[128];
			if(wcstombs_s(&cardDescriptionLength, cardDescription, 128, adapterDesc.Description, 128) != 0) {
				Console::Info("Could not convert adapter description to multibyte string");
				goto done;
			}

			Console::Info("Adapter: %s", cardDescription);

		done:
			SAFE_DELETE_ARRAY(modeList);
			SAFE_RELEASE(adapterOutput);
			SAFE_RELEASE(adapter);
			SAFE_RELEASE(factory);
		}
	#endif

	} // namespace D3D
	
} // namespace Maki
