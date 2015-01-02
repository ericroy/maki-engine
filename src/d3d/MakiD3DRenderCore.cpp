#include "d3d/d3d_stdafx.h"
#include "d3d/MakiD3DRenderCore.h"
#include "d3d/MakiD3DCommon.h"
#include "d3d/MakiD3DGPUTypes.h"
#include "d3d/DDSTextureLoader.h"
#include "SDL_syswm.h"

using namespace maki::core;

namespace maki
{
	namespace d3d
	{

		d3d_render_core_t::d3d_render_core_t(window_t *window, const config_t *config)
			:	render_core_t(),
				swap_chain_(nullptr),
				device_(nullptr),
				context_(nullptr),
				default_render_target_view_(nullptr),
				default_depth_stencil_view_(nullptr),
				current_render_target_view_(nullptr),
				current_depth_stencil_view_(nullptr),
				rasterizer_state_(nullptr),
				rasterizer_state_cull_back_(nullptr),
				rasterizer_state_cull_front_(nullptr),
				rasterizer_state_wire_frame_(nullptr),
				rasterizer_state_wire_frame_cull_back_(nullptr),
				rasterizer_state_wire_frame_cull_front_(nullptr),
				depth_state_(nullptr),
				depth_state_write_(nullptr),
				depth_state_less_(nullptr),
				depth_state_less_write_(nullptr),
				depth_state_equal_(nullptr),
				depth_state_equal_write_(nullptr),
				depth_state_less_equal_(nullptr),
				depth_state_less_equal_write_(nullptr),
				blend_enabled_(nullptr),
				blend_disabled_(nullptr)
		{
			memset(null_array_, 0, sizeof(null_array_));

			vsync_ = config->get_bool("engine.vsync", false);
			max_vertex_formats_per_vertex_shader_ = config->get_uint("d3d.max_vertex_formats_per_vertex_shader", 6);
			
			// Check support_ed resolutions
			uint32 refresh_numer = 0;
			uint32 refresh_denom = 0;
			if(!is_mode_supported(window->width_, window->height_, &refresh_numer, &refresh_denom))
			{
				console_t::warning("Mode not support_ed: %dx%d", window->width_, window->height_);
			}

			// get hwnd from SDL window
			SDL_SysWMinfo sdlInfo;
			SDL_version sdlVer;
			SDL_VERSION(&sdlVer);
			sdlInfo.version = sdlVer;
			SDL_GetWindowWMInfo(window->window_, &sdlInfo);
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
			scd.Windowed = !window->fullscreen_;
			scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


			UINT flags = 0;
			
			if(config->get_bool("d3d.debug_context_t", false)) {
				console_t::info("Requesting Direct3D debug context_t");
				flags |= D3D11_CREATE_DEVICE_DEBUG;
			}

			uint16 major = (uint16)config->get_uint("d3d.major_version", 10);
			uint16 minor = (uint16)config->get_uint("d3d.minor_version", 0);
			int64 version = (major << 16) | minor;
			D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_10_0;
			switch(version) {
			case (10<<16)|0:
				featureLevel = D3D_FEATURE_LEVEL_10_0;
				break;
			case (10<<16)|1:
				featureLevel = D3D_FEATURE_LEVEL_10_1;
				break;
			case (11<<16)|0:
				featureLevel = D3D_FEATURE_LEVEL_11_0;
				break;
			case (11<<16)|1:
				featureLevel = D3D_FEATURE_LEVEL_11_1;
				break;
			default:
				console_t::error("Unsupport_ed Direct3D major/minor version, using defaults instead");
			}

			console_t::info("Creating Direct3D %d.%d context_t", major, minor);

			HRESULT ret = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags|D3D11_CREATE_DEVICE_SINGLETHREADED, &featureLevel, 1,
				D3D11_SDK_VERSION, &scd, &swap_chain_, &device_, nullptr, &context_);
			if(__failed(ret)) {
				console_t::error("Failed to create device and swap chain");
			}
		}

		d3d_render_core_t::~d3d_render_core_t() {
			if(swap_chain_ != nullptr) {
				swap_chain_->SetFullscreenState(false, nullptr);
			}

			MAKI_SAFE_RELEASE(rasterizer_state_);
			MAKI_SAFE_RELEASE(rasterizer_state_cull_back_);
			MAKI_SAFE_RELEASE(rasterizer_state_cull_front_);
			MAKI_SAFE_RELEASE(rasterizer_state_wire_frame_);
			MAKI_SAFE_RELEASE(rasterizer_state_wire_frame_cull_back_);
			MAKI_SAFE_RELEASE(rasterizer_state_wire_frame_cull_front_);

			MAKI_SAFE_RELEASE(depth_state_);
			MAKI_SAFE_RELEASE(depth_state_write_);
			MAKI_SAFE_RELEASE(depth_state_less_);
			MAKI_SAFE_RELEASE(depth_state_less_write_);
			MAKI_SAFE_RELEASE(depth_state_equal_);
			MAKI_SAFE_RELEASE(depth_state_equal_write_);
			MAKI_SAFE_RELEASE(depth_state_less_equal_);
			MAKI_SAFE_RELEASE(depth_state_less_equal_write_);

			MAKI_SAFE_RELEASE(blend_enabled_);
			MAKI_SAFE_RELEASE(blend_disabled_);

			MAKI_SAFE_RELEASE(default_render_target_view_);
			MAKI_SAFE_RELEASE(default_depth_stencil_view_);
			MAKI_SAFE_RELEASE(swap_chain_);		
			MAKI_SAFE_RELEASE(context_);
			MAKI_SAFE_RELEASE(device_);

			console_t::info("Direct3D renderer destroyed");
		}

		void d3d_render_core_t::init()
		{
			std::lock_guard<std::mutex> lock(mutex_);

			// Setup rasterizer state
			D3D11_RASTERIZER_DESC rasterizer_desc;
			ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));
			rasterizer_desc.FillMode = D3D11_FILL_SOLID;
			rasterizer_desc.CullMode = D3D11_CULL_NONE;
			rasterizer_desc.FrontCounterClockwise = true;
			rasterizer_desc.DepthBias = 0;
			rasterizer_desc.DepthBiasClamp = 0.0f;
			rasterizer_desc.SlopeScaledDepthBias = 0.0f;
			rasterizer_desc.DepthClipEnable = true;
			rasterizer_desc.ScissorEnable = false;
			rasterizer_desc.MultisampleEnable = false;
			rasterizer_desc.AntialiasedLineEnable = false;
			if(__failed(device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_))) {
				console_t::error("Failed to create rasterizer state");
			}
			rasterizer_desc.CullMode = D3D11_CULL_BACK;
			if(__failed(device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_cull_back_))) {
				console_t::error("Failed to create rasterizer state (cull back)");
			}
			rasterizer_desc.CullMode = D3D11_CULL_FRONT;
			if(__failed(device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_cull_front_))) {
				console_t::error("Failed to create rasterizer state (cull front)");
			}
			rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
			rasterizer_desc.CullMode = D3D11_CULL_NONE;
			if(__failed(device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_wire_frame_))) {
				console_t::error("Failed to create rasterizer state (wire frame)");
			}
			rasterizer_desc.CullMode = D3D11_CULL_BACK;
			if(__failed(device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_wire_frame_cull_back_))) {
				console_t::error("Failed to create rasterizer state (wire frame, cull back)");
			}
			rasterizer_desc.CullMode = D3D11_CULL_FRONT;
			if(__failed(device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_wire_frame_cull_front_))) {
				console_t::error("Failed to create rasterizer state (wire frame, cull front)");
			}

			// set up depth stencil state
			D3D11_DEPTH_STENCIL_DESC depthDesc;
			ZeroMemory(&depthDesc, sizeof(depthDesc));
			depthDesc.DepthEnable = false;
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthDesc.DepthFunc = D3D11_COMPARISON_NEVER;
			depthDesc.StencilEnable = false;
			if(__failed(device_->CreateDepthStencilState(&depthDesc, &depth_state_))) {
				console_t::error("Failed to create depth stencil state A");
			}
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			if(__failed(device_->CreateDepthStencilState(&depthDesc, &depth_state_write_))) {
				console_t::error("Failed to create depth stencil state B");
			}
			depthDesc.DepthEnable = true;
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
			if(__failed(device_->CreateDepthStencilState(&depthDesc, &depth_state_less_))) {
				console_t::error("Failed to create depth stencil state C");
			}
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			if(__failed(device_->CreateDepthStencilState(&depthDesc, &depth_state_less_write_))) {
				console_t::error("Failed to create depth stencil state D");
			}
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
			if(__failed(device_->CreateDepthStencilState(&depthDesc, &depth_state_equal_))) {
				console_t::error("Failed to create depth stencil state E");
			}
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			if(__failed(device_->CreateDepthStencilState(&depthDesc, &depth_state_equal_write_))) {
				console_t::error("Failed to create depth stencil state F");
			}
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			if(__failed(device_->CreateDepthStencilState(&depthDesc, &depth_state_less_equal_))) {
				console_t::error("Failed to create depth stencil state G");
			}
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			if(__failed(device_->CreateDepthStencilState(&depthDesc, &depth_state_less_equal_write_))) {
				console_t::error("Failed to create depth stencil state H");
			}


			// Setup blend state
			D3D11_BLEND_DESC blend_desc;
			ZeroMemory(&blend_desc, sizeof(blend_desc));
			blend_desc.RenderTarget[0].BlendEnable = true;
			blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].RenderTargetWriteMask = 0x0f;
			if(__failed(device_->CreateBlendState(&blend_desc, &blend_enabled_))) {
				console_t::error("Failed to create blend state (enabled)");
			}
			blend_desc.RenderTarget[0].BlendEnable = false;
			if(__failed(device_->CreateBlendState(&blend_desc, &blend_disabled_))) {
				console_t::error("Failed to create blend state (disabled)");
			}
		

			// set initial state:
			context_->RSSetState(rasterizer_state_);
			context_->OMSetBlendState(blend_enabled_, nullptr, 0xffffffff);
			context_->OMSetDepthStencilState(depth_state_less_write_, 1);

			window_width_ = 1;
			window_height_ = 1;
			resized(window_width_, window_height_);

			// Render a blank frame so we don't see a flash of white on startup
			set_render_target_and_depth_stencil(render_state_t::render_target_default_, HANDLE_NONE, render_state_t::depth_stencil_null_, HANDLE_NONE);
			float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
			context_->ClearRenderTargetView(default_render_target_view_, color);
			swap_chain_->Present(0, 0);
		}

		void d3d_render_core_t::resized(uint32 width, uint32 height)
		{
			context_->OMSetRenderTargets(0, nullptr, nullptr);
			MAKI_SAFE_RELEASE(default_render_target_view_);
			MAKI_SAFE_RELEASE(default_depth_stencil_view_);

			// ResizeBuffers breaks the graphics diagnostics, so don't call it while using the graphics profiler
	#if !MAKI_PROFILING
			if(MAKI_D3D_FAILED(swap_chain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0))) {
				console_t::error("Failed to resize swap chain buffers");
			}
	#endif
		
			// Setup render target
			ID3D11Texture2D *renderBuffer = nullptr;
			if(MAKI_D3D_FAILED(swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&renderBuffer))) {
				console_t::error("Failed to get buffer from swap chain for render target view");
			}
			if(MAKI_D3D_FAILED(device_->CreateRenderTargetView(renderBuffer, nullptr, &default_render_target_view_))) {
				console_t::error("Failed to create render target view");
			}
			MAKI_SAFE_RELEASE(renderBuffer);
		

			// Setup depth buffer
			D3D11_TEXTURE2D_DESC depthDesc;
			ZeroMemory(&depthDesc, sizeof(depthDesc));
			depthDesc.Width = width;
			depthDesc.Height = height;
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
			if(MAKI_D3D_FAILED(device_->CreateTexture2D(&depthDesc, nullptr, &depthBuffer))) {
				console_t::error("Failed to create texture for depth buffer");
			}
			if(MAKI_D3D_FAILED(device_->CreateDepthStencilView(depthBuffer, &depthViewDesc, &default_depth_stencil_view_))) {
				console_t::error("Failed to create depth stencil view");
			}
			MAKI_SAFE_RELEASE(depthBuffer);
		}

		void d3d_render_core_t::present()
		{
			std::lock_guard<std::mutex> lock(mutex_);
			swap_chain_->Present(vsync_ ? 1 : 0, 0);
		}


		// Resource creation, deletion, modification:

		void *d3d_render_core_t::upload_buffer(void *buffer, vertex_format_t *vf, char *vertex_data, uint32 vertex_count_, char *index_data, uint32 face_count_, uint8 indices_per_face_, uint8 bytes_per_index_, bool dynamic, bool length_changed)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			buffer_t *b = (buffer_t *)buffer;
			if(b != nullptr) {
				MAKI_SAFE_RELEASE(b->vbos_[0]);
				MAKI_SAFE_RELEASE(b->vbos_[1]);
			} else {
				b = new buffer_t();
				memset(b, 0, sizeof(buffer_t));
			}

			b->vertex_count_ = vertex_count_;
			b->face_count_ = face_count_;
			b->indices_per_face_ = indices_per_face_;
			b->bytes_per_index_ = bytes_per_index_;

			assert(indices_per_face_ > 0 && indices_per_face_ <= 3);
			b->geometry_type_ = indices_per_face_to_geometry_type[indices_per_face_];
		
			assert(bytes_per_index_ > 0 && bytes_per_index_ <= 4 && bytes_per_index_ != 3);
			b->index_data_type_ = bytes_per_index_to_format[bytes_per_index_];

			// TODO:
			// In the case where the buffer lengths have not changed, we should probably just map this instead of recreating the buffers

			// Create vertex buffer
			D3D11_SUBRESOURCE_DATA srd;
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			ZeroMemory(&srd, sizeof(srd));
			bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
			bd.ByteWidth = vf->get_stride()*vertex_count_;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			if(dynamic) {
				bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}
			srd.pSysMem = vertex_data;
			if(MAKI_D3D_FAILED(device_->CreateBuffer(&bd, &srd, &b->vbos_[0]))) {
				console_t::error("Failed to create d3d vertex buffer");
			}
		
			// Create index buffer
			ZeroMemory(&bd, sizeof(bd));
			ZeroMemory(&srd, sizeof(srd));
			bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
			bd.ByteWidth = bytes_per_index_*indices_per_face_*face_count_;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			if(dynamic) {
				bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}
			srd.pSysMem = index_data;
			if(MAKI_D3D_FAILED(device_->CreateBuffer(&bd, &srd, &b->vbos_[1]))) {
				console_t::error("Failed to create d3d index buffer");
			}

			return (void *)b;
		}

		void d3d_render_core_t::free_buffer(void *buffer)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			if(buffer != nullptr) {
				buffer_t *b = (buffer_t *)buffer;
				MAKI_SAFE_RELEASE(b->vbos_[0]);
				MAKI_SAFE_RELEASE(b->vbos_[1]);
				delete b;
			}
		}

		bool d3d_render_core_t::create_pixel_shader(shader_t *ps)
		{
			gpu_pixel_shader_t *gps = new gpu_pixel_shader_t();

			if(MAKI_D3D_FAILED(device_->CreatePixelShader(ps->program_data_, ps->program_data_bytes_, nullptr, &gps->ps_))) {
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

#define ROUND_SIXTEEN(x) ((x) & 0x7) != 0 ? ((x) & ~0x7)+0x10 : (x)
			if(ps->frame_uniform_buffer_location_ != -1) {
				bufferDesc.ByteWidth = ROUND_SIXTEEN(ps->engine_frame_uniform_bytes_);
				if(MAKI_D3D_FAILED(device_->CreateBuffer(&bufferDesc, nullptr, &gps->per_frame_constants_))) {
					goto failed;
				}
			}
			if(ps->object_uniform_buffer_location_ != -1) {
				bufferDesc.ByteWidth = ROUND_SIXTEEN(ps->engine_object_uniform_bytes_);
				if(MAKI_D3D_FAILED(device_->CreateBuffer(&bufferDesc, nullptr, &gps->per_object_constants_))) {
					goto failed;
				}
			}
			if(ps->material_uniform_buffer_location_ != -1) {
				bufferDesc.ByteWidth = ROUND_SIXTEEN(ps->material_uniform_bytes_);
				if(MAKI_D3D_FAILED(device_->CreateBuffer(&bufferDesc, nullptr, &gps->material_constants_))) {
					goto failed;
				}
			}
#undef ROUND_SIXTEEN

			ps->handle_ = (intptr_t)gps;
			return true;

failed:
			MAKI_SAFE_DELETE(gps);
			return false;
		}

		bool d3d_render_core_t::create_vertex_shader(shader_t *vs)
		{
			gpu_vertex_shader_t *gvs = new gpu_vertex_shader_t(max_vertex_formats_per_vertex_shader_);

			// Since vertex shader keeps a pointer to this data, we will not delete the buffer as we exit the function
			char *blob_ = (char *)allocator_t::malloc(vs->program_data_bytes_);
			memcpy(blob_, vs->program_data_, vs->program_data_bytes_);
			gvs->blob_ = blob_;
			gvs->blob_size_ = vs->program_data_bytes_;
		
			if(MAKI_D3D_FAILED(device_->CreateVertexShader(gvs->blob_, gvs->blob_size_, nullptr, &gvs->vs_))) {
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

#define ROUND_SIXTEEN(x) ((x) & 0x7) != 0 ? ((x) & ~0x7)+0x10 : (x)
			if(vs->frame_uniform_buffer_location_ != -1) {
				bufferDesc.ByteWidth = ROUND_SIXTEEN(vs->engine_frame_uniform_bytes_);
				if(MAKI_D3D_FAILED(device_->CreateBuffer(&bufferDesc, nullptr, &gvs->per_frame_constants_))) {
					goto failed;
				}
			}
			if(vs->object_uniform_buffer_location_ != -1) {
				bufferDesc.ByteWidth = ROUND_SIXTEEN(vs->engine_object_uniform_bytes_);
				if(MAKI_D3D_FAILED(device_->CreateBuffer(&bufferDesc, nullptr, &gvs->per_object_constants_))) {
					goto failed;
				}
			}
			if(vs->material_uniform_buffer_location_ != -1) {
				bufferDesc.ByteWidth = ROUND_SIXTEEN(vs->material_uniform_bytes_);
				if(MAKI_D3D_FAILED(device_->CreateBuffer(&bufferDesc, nullptr, &gvs->material_constants_))) {
					goto failed;
				}
			}
#undef ROUND_SIXTEEN

			vs->handle_ = (intptr_t)gvs;
			return true;

failed:
			MAKI_SAFE_DELETE(gvs);
			return false;
		}

		bool d3d_render_core_t::create_shader_program(shader_program_t *s)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			if(!create_vertex_shader(&s->vertex_shader_)) {
				return false;
			}

			if(!create_pixel_shader(&s->pixel_shader_)) {
				return false;
			}

			return true;
		}

	

		bool d3d_render_core_t::create_empty_texture(texture_t *t, uint8 channels)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			ID3D11Texture2D *tex_ = nullptr;
			ID3D11ShaderResourceView *shader_resource_view_ = nullptr;
			ID3D11SamplerState *sampler_state_ = nullptr;

			if(channels == 0 || channels > 4 || channels == 3) {
				console_t::error("Unsupport_ed number of channels in image: %d", channels);
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
			if(MAKI_D3D_FAILED(device_->CreateSamplerState(&samplerDesc, &sampler_state_))) {
				console_t::error("CreateSamplerState failed");
				goto failed;
			}

			D3D11_TEXTURE2D_DESC textureDesc;
			ZeroMemory(&textureDesc, sizeof(textureDesc));
			textureDesc.Width = t->width_;
			textureDesc.Height = t->height_;
			textureDesc.Format = channels_to_format[channels];
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = 0;
			if(MAKI_D3D_FAILED(device_->CreateTexture2D(&textureDesc, nullptr, &tex_))) {
				console_t::error("CreateTexture2D failed");
				goto failed;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
			ZeroMemory(&shaderResViewDesc, sizeof(shaderResViewDesc));
			shaderResViewDesc.Format = textureDesc.Format;
			shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
			if(MAKI_D3D_FAILED(device_->CreateShaderResourceView(tex_, &shaderResViewDesc, &shader_resource_view_))) {
				console_t::error("CreateShaderResourceView failed");
				goto failed;
			}
		
		

			gpu_texture_t *gtex = new gpu_texture_t();
			gtex->shader_resource_view_ = shader_resource_view_;
			gtex->sampler_state_ = sampler_state_;
			t->handle_ = (intptr_t)gtex;

			MAKI_SAFE_RELEASE(tex_);
			return true;

failed:		
			MAKI_SAFE_RELEASE(tex_);
			MAKI_SAFE_RELEASE(shader_resource_view_);
			MAKI_SAFE_RELEASE(sampler_state_);
			return false;
		}

		bool d3d_render_core_t::create_render_target(texture_t *t)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			ID3D11SamplerState *sampler_state_ = nullptr;
			ID3D11Texture2D *tex_ = nullptr;
			ID3D11RenderTargetView *render_target_view_ = nullptr;
			ID3D11ShaderResourceView *shader_resource_view_ = nullptr;


			D3D11_SAMPLER_DESC samplerDesc;
			ZeroMemory(&samplerDesc, sizeof(samplerDesc));

			// TODO:
			// Should this really be using linear filtering?  Perhaps nearest would be better?
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
			if(MAKI_D3D_FAILED(device_->CreateSamplerState(&samplerDesc, &sampler_state_))) {
				console_t::error("CreateSamplerState failed");
				goto failed;
			}

			D3D11_TEXTURE2D_DESC textureDesc;
			ZeroMemory(&textureDesc, sizeof(textureDesc));
			textureDesc.Width = t->width_;
			textureDesc.Height = t->height_;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = 0;
			if(MAKI_D3D_FAILED(device_->CreateTexture2D(&textureDesc, nullptr, &tex_))) {
				console_t::error("CreateTexture2D failed");
				goto failed;
			}

			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
			renderTargetViewDesc.Format = textureDesc.Format;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			if(MAKI_D3D_FAILED(device_->CreateRenderTargetView(tex_, &renderTargetViewDesc, &render_target_view_))) {
				console_t::error("CreateRenderTargetView failed");
				goto failed;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
			ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
			shaderResourceViewDesc.Format = textureDesc.Format;
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
			if(MAKI_D3D_FAILED(device_->CreateShaderResourceView(tex_, &shaderResourceViewDesc, &shader_resource_view_))) {
				console_t::error("CreateShaderResourceView failed");
				goto failed;
			}
		
			gpu_texture_t *gtex = new gpu_texture_t();
			gtex->shader_resource_view_ = shader_resource_view_;
			gtex->sampler_state_ = sampler_state_;
			gtex->render_target_view_ = render_target_view_;
			t->handle_ = (intptr_t)gtex;

			MAKI_SAFE_RELEASE(tex_);
			return true;

failed:		
			MAKI_SAFE_RELEASE(tex_);
			MAKI_SAFE_RELEASE(shader_resource_view_);
			MAKI_SAFE_RELEASE(sampler_state_);
			MAKI_SAFE_RELEASE(render_target_view_);
			return false;
		}

		bool d3d_render_core_t::create_depth_texture(texture_t *t)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			ID3D11SamplerState *sampler_state_ = nullptr;
			ID3D11Texture2D *tex_ = nullptr;
			ID3D11DepthStencilView *depth_stencil_view_ = nullptr;
			ID3D11ShaderResourceView *shader_resource_view_ = nullptr;


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

			if(MAKI_D3D_FAILED(device_->CreateSamplerState(&samplerDesc, &sampler_state_))) {
				console_t::error("CreateSamplerState failed");
				goto failed;
			}

			D3D11_TEXTURE2D_DESC textureDesc;
			ZeroMemory(&textureDesc, sizeof(textureDesc));
			textureDesc.Width = t->width_;
			textureDesc.Height = t->height_;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = 0;
			if(MAKI_D3D_FAILED(device_->CreateTexture2D(&textureDesc, nullptr, &tex_))) {
				console_t::error("CreateTexture2D failed");
				goto failed;
			}

			D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
			ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
			depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			if(MAKI_D3D_FAILED(device_->CreateDepthStencilView(tex_, &depthStencilViewDesc, &depth_stencil_view_))) {
				console_t::error("CreateDepthStencilView failed");
				goto failed;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
			ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
			shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceViewDesc.Texture2D.MipLevels = 1;
			if(MAKI_D3D_FAILED(device_->CreateShaderResourceView(tex_, &shaderResourceViewDesc, &shader_resource_view_))) {
				console_t::error("CreateShaderResourceView failed");
				goto failed;
			}
		
			gpu_texture_t *gtex = new gpu_texture_t();
			gtex->shader_resource_view_ = shader_resource_view_;
			gtex->sampler_state_ = sampler_state_;
			gtex->depth_stencil_view_ = depth_stencil_view_;
			t->handle_ = (intptr_t)gtex;

			MAKI_SAFE_RELEASE(tex_);
			return true;

failed:		
			MAKI_SAFE_RELEASE(tex_);
			MAKI_SAFE_RELEASE(shader_resource_view_);
			MAKI_SAFE_RELEASE(sampler_state_);
			MAKI_SAFE_RELEASE(depth_stencil_view_);
			return false;
		}

		bool d3d_render_core_t::create_texture(texture_t *t, char *data, uint32 data_length)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			ID3D11Texture2D *tex_ = nullptr;
			ID3D11ShaderResourceView *shader_resource_view_ = nullptr;
			ID3D11SamplerState *sampler_state_ = nullptr;
		
			if(MAKI_D3D_FAILED(DirectX::CreateDDSTextureFromMemory(device_, (uint8 *)data, data_length, (ID3D11Resource **)&tex_, &shader_resource_view_))) {
				console_t::error("Failed to create texture from memory");
				goto failed;
			}

			D3D11_TEXTURE2D_DESC desc;
			tex_->GetDesc(&desc);
			t->width_ = desc.Width;
			t->height_ = desc.Height;

			D3D11_SAMPLER_DESC samplerDesc;
			ZeroMemory(&samplerDesc, sizeof(samplerDesc));
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			samplerDesc.MaxAnisotropy = 2;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
			if(MAKI_D3D_FAILED(device_->CreateSamplerState(&samplerDesc, &sampler_state_))) {
				console_t::error("CreateSamplerState failed");
				goto failed;
			}
		
			gpu_texture_t *gtex = new gpu_texture_t();
			gtex->shader_resource_view_ = shader_resource_view_;
			gtex->sampler_state_ = sampler_state_;
			t->handle_ = (intptr_t)gtex;

			MAKI_SAFE_RELEASE(tex_);
			return true;

failed:		
			MAKI_SAFE_RELEASE(tex_);
			MAKI_SAFE_RELEASE(shader_resource_view_);
			MAKI_SAFE_RELEASE(sampler_state_);
			return false;
		}

		void d3d_render_core_t::write_to_texture(texture_t *t, int32 dst_x, int32 dst_y, int32 src_x, int32 src_y, uint32 src_width, uint32 src_height, uint32 src_pitch, uint8 channels, char *src_data)
		{
			std::lock_guard<std::mutex> lock(mutex_);
		
			gpu_texture_t *tex_ = (gpu_texture_t *)t->handle_;

			D3D11_BOX box;
			box.left = dst_x;
			box.right = dst_x+src_width;
			box.top = dst_y;
			box.bottom = dst_y+src_height;
			box.front = 0;
			box.back = 1;

			char *p = src_data + (src_y*src_pitch) + (src_x*channels);

			ID3D11Resource *res = nullptr;
			tex_->shader_resource_view_->GetResource(&res);
			context_->UpdateSubresource(res, 0, &box, p, src_pitch, src_pitch);
			MAKI_SAFE_RELEASE(res);
		}

		void d3d_render_core_t::delete_shader_program(shader_program_t *s)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			gpu_vertex_shader_t *gvs = (gpu_vertex_shader_t *)s->vertex_shader_.handle_;
			MAKI_SAFE_DELETE(gvs);
			s->vertex_shader_.handle_ = (intptr_t)nullptr;

			gpu_pixel_shader_t *gps = (gpu_pixel_shader_t *)s->pixel_shader_.handle_;
			MAKI_SAFE_DELETE(gps);
			s->pixel_shader_.handle_ = (intptr_t)nullptr;
		}

		void d3d_render_core_t::delete_texture(texture_t *t)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			gpu_texture_t *gtex = (gpu_texture_t *)t->handle_;
			MAKI_SAFE_DELETE(gtex);
			t->handle_ = (intptr_t)nullptr;
		}








		bool d3d_render_core_t::is_mode_supported(uint32 window_width, uint32 window_height, uint32 *refresh_numer_out, uint32 *refresh_denom_out)
		{
			bool found = false;

			IDXGIFactory *factory = nullptr;
			IDXGIAdapter *adapter = nullptr;
			IDXGIOutput *adapter_output = nullptr;
			uint32 mode_count;
			DXGI_MODE_DESC *mode_list = nullptr;
			uint32 refresh_numer = 0, refresh_denom = 0;
			DXGI_ADAPTER_DESC adapter_desc;

			if(MAKI_D3D_FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory))) {
				console_t::error("Failed to create dxgi factory");
				goto done;
			}
			if(MAKI_D3D_FAILED(factory->EnumAdapters(0, &adapter))) {
				console_t::error("Failed to enum adapters");
				goto done;
			}
			if(MAKI_D3D_FAILED(adapter->EnumOutputs(0, &adapter_output))) {
				console_t::error("Failed to enum outputs");
				goto done;
			}
			if(MAKI_D3D_FAILED(adapter_output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &mode_count, nullptr))) {
				console_t::error("Failed to get display mode list size");
				goto done;
			}
			mode_list = new DXGI_MODE_DESC[mode_count];
			if(MAKI_D3D_FAILED(adapter_output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &mode_count, mode_list))) {
				console_t::error("Failed to get display mode list");
				goto done;
			}
				
			
			for(uint32 i = 0; i < mode_count; i++) {
				bool resMatch = mode_list[i].Width == window_width && mode_list[i].Height == window_height;

				console_t::info("Support_ed mode: %dx%d @ %f Hz %s",
					mode_list[i].Width,
					mode_list[i].Height,
					mode_list[i].RefreshRate.Denominator != 0 ? mode_list[i].RefreshRate.Numerator / (float)mode_list[i].RefreshRate.Denominator : 0.0f,
					resMatch ? "<<< Found!" : "");
				
				if(resMatch)
				{
					if(refresh_numer_out != nullptr) {
						*refresh_numer_out = mode_list[i].RefreshRate.Numerator;
					}
					if(refresh_denom_out != nullptr) {
						*refresh_denom_out = mode_list[i].RefreshRate.Denominator;
					}
					found = true;
				}
			}
			if(!found) {
				console_t::warning("Could not find desired resolution in display mode list");
			}

			if(MAKI_D3D_FAILED(adapter->GetDesc(&adapter_desc))) {
				console_t::error("Failed to get adapter descriptor");
				goto done;
			}

			console_t::info("VRAM: %dmb", adapter_desc.DedicatedVideoMemory / 1024 / 1024);
		
			// Convert the name of the video card to a character array and store it.
			size_t cardDescriptionLength;
			char cardDescription[128];
			if(wcstombs_s(&cardDescriptionLength, cardDescription, 128, adapter_desc.Description, 128) != 0) {
				console_t::info("Could not convert adapter description to multibyte string");
				goto done;
			}

			console_t::info("Adapter: %s", cardDescription);
			return found;

		done:
			MAKI_SAFE_DELETE_ARRAY(mode_list);
			MAKI_SAFE_RELEASE(adapter_output);
			MAKI_SAFE_RELEASE(adapter);
			MAKI_SAFE_RELEASE(factory);
			return found;
		}


	} // namespace d3d
	
} // namespace maki
