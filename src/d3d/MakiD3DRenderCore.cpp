#include "d3d/MakiD3DRenderCore.h"
#include "d3d/MakiD3DCommon.h"
#include "d3d/MakiD3DGPUTypes.h"
#include "d3d/DDSTextureLoader.h"
#include "SDL_syswm.h"

using namespace std;
using namespace maki::core;

namespace maki
{
	namespace d3d
	{

		d3d_render_core_t::d3d_render_core_t(window_t *window, const config_t *config) : render_core_t() {
			vsync_ = config->get_bool("engine.vsync", vsync_);
			max_vertex_formats_per_vertex_shader_ = config->get_uint("d3d.max_vertex_formats_per_vertex_shader", max_vertex_formats_per_vertex_shader_);
			
			// Check support_ed resolutions
			uint32_t refresh_numer = 0;
			uint32_t refresh_denom = 0;
			if(!is_mode_supported(window->width(), window->height(), &refresh_numer, &refresh_denom))
				console_t::warning("Mode not supported: %dx%d", window->width(), window->height());

			// get hwnd from SDL window
			SDL_SysWMinfo sdlInfo;
			SDL_version sdlVer;
			SDL_VERSION(&sdlVer);
			sdlInfo.version = sdlVer;
			SDL_GetWindowWMInfo(static_cast<SDL_Window *>(window->handle()), &sdlInfo);
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
			scd.Windowed = !window->fullscreen();
			scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


			UINT flags = 0;
			
			if(config->get_bool("d3d.debug_context", false)) {
				console_t::info("Requesting Direct3D debug context");
				flags |= D3D11_CREATE_DEVICE_DEBUG;
			}

			uint16_t major = (uint16_t)config->get_uint("d3d.major_version", 10);
			uint16_t minor = (uint16_t)config->get_uint("d3d.minor_version", 0);
			int64_t version = (major << 16) | minor;
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

			console_t::info("Creating Direct3D %d.%d context", major, minor);

			HRESULT ret = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags|D3D11_CREATE_DEVICE_SINGLETHREADED, &featureLevel, 1,
				D3D11_SDK_VERSION, &scd, &swap_chain_, &device_, nullptr, &context_);
			if(__failed(ret)) {
				console_t::error("Failed to create device and swap chain");
			}
		}

		d3d_render_core_t::~d3d_render_core_t() {
			if(swap_chain_ != nullptr)
				swap_chain_->SetFullscreenState(false, nullptr);

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

		void d3d_render_core_t::init() {
			lock_guard<mutex> lock(mutex_);

			// Setup rasterizer state
			D3D11_RASTERIZER_DESC rasterizer_desc = {};
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
			if(__failed(device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_)))
				console_t::error("Failed to create rasterizer state");
			
			rasterizer_desc.CullMode = D3D11_CULL_BACK;
			if(__failed(device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_cull_back_)))
				console_t::error("Failed to create rasterizer state (cull back)");
			
			rasterizer_desc.CullMode = D3D11_CULL_FRONT;
			if(__failed(device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_cull_front_)))
				console_t::error("Failed to create rasterizer state (cull front)");
			
			rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
			rasterizer_desc.CullMode = D3D11_CULL_NONE;
			if(__failed(device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_wire_frame_)))
				console_t::error("Failed to create rasterizer state (wire frame)");
			
			rasterizer_desc.CullMode = D3D11_CULL_BACK;
			if(__failed(device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_wire_frame_cull_back_)))
				console_t::error("Failed to create rasterizer state (wire frame, cull back)");
			
			rasterizer_desc.CullMode = D3D11_CULL_FRONT;
			if(__failed(device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_wire_frame_cull_front_)))
				console_t::error("Failed to create rasterizer state (wire frame, cull front)");

			// set up depth stencil state
			D3D11_DEPTH_STENCIL_DESC depth_desc = {};
			depth_desc.DepthEnable = false;
			depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depth_desc.DepthFunc = D3D11_COMPARISON_NEVER;
			depth_desc.StencilEnable = false;
			if(__failed(device_->CreateDepthStencilState(&depth_desc, &depth_state_)))
				console_t::error("Failed to create depth stencil state A");

			depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			if(__failed(device_->CreateDepthStencilState(&depth_desc, &depth_state_write_)))
				console_t::error("Failed to create depth stencil state B");

			depth_desc.DepthEnable = true;
			depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depth_desc.DepthFunc = D3D11_COMPARISON_LESS;
			if(__failed(device_->CreateDepthStencilState(&depth_desc, &depth_state_less_)))
				console_t::error("Failed to create depth stencil state C");

			depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			if(__failed(device_->CreateDepthStencilState(&depth_desc, &depth_state_less_write_)))
				console_t::error("Failed to create depth stencil state D");

			depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depth_desc.DepthFunc = D3D11_COMPARISON_EQUAL;
			if(__failed(device_->CreateDepthStencilState(&depth_desc, &depth_state_equal_)))
				console_t::error("Failed to create depth stencil state E");

			depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			if(__failed(device_->CreateDepthStencilState(&depth_desc, &depth_state_equal_write_)))
				console_t::error("Failed to create depth stencil state F");

			depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depth_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			if(__failed(device_->CreateDepthStencilState(&depth_desc, &depth_state_less_equal_)))
				console_t::error("Failed to create depth stencil state G");

			depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			if(__failed(device_->CreateDepthStencilState(&depth_desc, &depth_state_less_equal_write_)))
				console_t::error("Failed to create depth stencil state H");



			// Setup blend state
			D3D11_BLEND_DESC blend_desc = {};
			blend_desc.RenderTarget[0].BlendEnable = true;
			blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].RenderTargetWriteMask = 0x0f;
			if(__failed(device_->CreateBlendState(&blend_desc, &blend_enabled_)))
				console_t::error("Failed to create blend state (enabled)");

			blend_desc.RenderTarget[0].BlendEnable = false;
			if(__failed(device_->CreateBlendState(&blend_desc, &blend_disabled_)))
				console_t::error("Failed to create blend state (disabled)");

			// set initial state:
			context_->RSSetState(rasterizer_state_);
			context_->OMSetBlendState(blend_enabled_, nullptr, 0xffffffff);
			context_->OMSetDepthStencilState(depth_state_less_write_, 1);

			window_width_ = 1;
			window_height_ = 1;
			resized(window_width_, window_height_);

			// Render a blank frame so we don't see a flash of white on startup
			set_render_target_and_depth_stencil(render_state_t::render_target_default, nullptr, render_state_t::depth_stencil_null, nullptr);
			float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
			context_->ClearRenderTargetView(default_render_target_view_, color);
			swap_chain_->Present(0, 0);
		}

		void d3d_render_core_t::resized(uint32_t width, uint32_t height) {
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
			ID3D11Texture2D *render_buffer = nullptr;
			if(MAKI_D3D_FAILED(swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&render_buffer)))
				console_t::error("Failed to get buffer from swap chain for render target view");

			if(MAKI_D3D_FAILED(device_->CreateRenderTargetView(render_buffer, nullptr, &default_render_target_view_)))
				console_t::error("Failed to create render target view");

			MAKI_SAFE_RELEASE(render_buffer);
		

			// Setup depth buffer
			D3D11_TEXTURE2D_DESC depth_desc = {};
			depth_desc.Width = width;
			depth_desc.Height = height;
			depth_desc.MipLevels = 1;
			depth_desc.ArraySize = 1;
			depth_desc.Format = DXGI_FORMAT_D32_FLOAT;
			depth_desc.SampleDesc.Count = 1;
			depth_desc.SampleDesc.Quality = 0;
			depth_desc.Usage = D3D11_USAGE_DEFAULT;
			depth_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			depth_desc.CPUAccessFlags = 0;
			depth_desc.MiscFlags = 0;
		
			D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_desc;
			ZeroMemory(&depth_view_desc, sizeof(depth_view_desc));
			depth_view_desc.Format = depth_desc.Format;
			depth_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depth_view_desc.Texture2D.MipSlice = 0;

			ID3D11Texture2D *depth_buffer = nullptr;
			if(MAKI_D3D_FAILED(device_->CreateTexture2D(&depth_desc, nullptr, &depth_buffer)))
				console_t::error("Failed to create texture for depth buffer");

			if(MAKI_D3D_FAILED(device_->CreateDepthStencilView(depth_buffer, &depth_view_desc, &default_depth_stencil_view_)))
				console_t::error("Failed to create depth stencil view");

			MAKI_SAFE_RELEASE(depth_buffer);
		}

		void d3d_render_core_t::present() {
			lock_guard<mutex> lock(mutex_);
			swap_chain_->Present(vsync_ ? 1 : 0, 0);
		}


		// Resource creation, deletion, modification:

		void *d3d_render_core_t::upload_buffer(void *buffer, vertex_format_t *vf, char *vertex_data, uint32_t vertex_count_, char *index_data, uint32_t face_count_, uint8_t indices_per_face_, uint8_t bytes_per_index_, bool dynamic, bool length_changed) {
			lock_guard<mutex> lock(mutex_);

			buffer_t *b = (buffer_t *)buffer;
			if(b != nullptr) {
				MAKI_SAFE_RELEASE(b->vbos[0]);
				MAKI_SAFE_RELEASE(b->vbos[1]);
			} else {
				b = new buffer_t();
				memset(b, 0, sizeof(buffer_t));
			}

			b->vertex_count = vertex_count_;
			b->face_count = face_count_;
			b->indices_per_face = indices_per_face_;
			b->bytes_per_index = bytes_per_index_;

			assert(indices_per_face_ > 0 && indices_per_face_ <= 3);
			b->geometry_type = indices_per_face_to_geometry_type[indices_per_face_];
		
			assert(bytes_per_index_ > 0 && bytes_per_index_ <= 4 && bytes_per_index_ != 3);
			b->index_data_type = bytes_per_index_to_format[bytes_per_index_];

			// TODO:
			// In the case where the buffer lengths have not changed, we should probably just map this instead of recreating the buffers

			// Create vertex buffer
			D3D11_SUBRESOURCE_DATA srd;
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			ZeroMemory(&srd, sizeof(srd));
			bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
			bd.ByteWidth = vf->stride()*vertex_count_;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			if(dynamic) {
				bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}
			srd.pSysMem = vertex_data;
			if(MAKI_D3D_FAILED(device_->CreateBuffer(&bd, &srd, &b->vbos[0]))) {
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
			if(MAKI_D3D_FAILED(device_->CreateBuffer(&bd, &srd, &b->vbos[1]))) {
				console_t::error("Failed to create d3d index buffer");
			}

			return (void *)b;
		}

		void d3d_render_core_t::free_buffer(void *buffer) {
			lock_guard<mutex> lock(mutex_);

			if(buffer != nullptr) {
				buffer_t *b = (buffer_t *)buffer;
				MAKI_SAFE_RELEASE(b->vbos[0]);
				MAKI_SAFE_RELEASE(b->vbos[1]);
				delete b;
			}
		}

		bool d3d_render_core_t::create_fragment_shader(shader_t *fs) {
			gpu_fragment_shader_t *gps = new gpu_fragment_shader_t();

			if(MAKI_D3D_FAILED(device_->CreatePixelShader(fs->program_data().data(), fs->program_data().length(), nullptr, &gps->fs)))
				goto failed;

			// Allocate buffers for constants
			D3D11_BUFFER_DESC buffer_desc;
			ZeroMemory(&buffer_desc, sizeof(buffer_desc));
			buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
			buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			buffer_desc.MiscFlags = 0;
			buffer_desc.StructureByteStride = 0;

#define ROUND_SIXTEEN(x) ((x) & 0x7) != 0 ? ((x) & ~0x7)+0x10 : (x)
			if(fs->frame_constant_buffer_location() != -1) {
				buffer_desc.ByteWidth = ROUND_SIXTEEN(fs->engine_frame_constant_bytes());
				if(MAKI_D3D_FAILED(device_->CreateBuffer(&buffer_desc, nullptr, &gps->per_frame_constants))) {
					goto failed;
				}
			}
			if(fs->object_constant_buffer_location() != -1) {
				buffer_desc.ByteWidth = ROUND_SIXTEEN(fs->engine_object_constant_bytes());
				if(MAKI_D3D_FAILED(device_->CreateBuffer(&buffer_desc, nullptr, &gps->per_object_constants))) {
					goto failed;
				}
			}
			if(fs->material_constant_buffer_location() != -1) {
				buffer_desc.ByteWidth = ROUND_SIXTEEN(fs->material_constant_bytes());
				if(MAKI_D3D_FAILED(device_->CreateBuffer(&buffer_desc, nullptr, &gps->material_constants))) {
					goto failed;
				}
			}
#undef ROUND_SIXTEEN

			fs->set_handle(gps);
			return true;

failed:
			MAKI_SAFE_DELETE(gps);
			return false;
		}

		bool d3d_render_core_t::create_vertex_shader(shader_t *vs)
		{
			gpu_vertex_shader_t *gvs = new gpu_vertex_shader_t(max_vertex_formats_per_vertex_shader_);
			gvs->blob = vs->program_data();
			if(MAKI_D3D_FAILED(device_->CreateVertexShader(gvs->blob.data(), gvs->blob.length(), nullptr, &gvs->vs))) {
				MAKI_SAFE_DELETE(gvs);
				return false;
			}

			// Allocate buffers for constants
			D3D11_BUFFER_DESC buffer_desc = {};
			buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
			buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			buffer_desc.MiscFlags = 0;
			buffer_desc.StructureByteStride = 0;

#define ROUND_SIXTEEN(x) ((x) & 0x7) != 0 ? ((x) & ~0x7) + 0x10 : (x)
			if(vs->frame_constant_buffer_location() != -1) {
				buffer_desc.ByteWidth = ROUND_SIXTEEN(vs->engine_frame_constant_bytes());
				if (MAKI_D3D_FAILED(device_->CreateBuffer(&buffer_desc, nullptr, &gvs->per_frame_constants))) {
					MAKI_SAFE_DELETE(gvs);
					return false;
				}
			}
			if(vs->object_constant_buffer_location() != -1) {
				buffer_desc.ByteWidth = ROUND_SIXTEEN(vs->engine_object_constant_bytes());
				if (MAKI_D3D_FAILED(device_->CreateBuffer(&buffer_desc, nullptr, &gvs->per_object_constants))) {
					MAKI_SAFE_DELETE(gvs);
					return false;
				}
			}
			if(vs->material_constant_buffer_location() != -1) {
				buffer_desc.ByteWidth = ROUND_SIXTEEN(vs->material_constant_bytes());
				if (MAKI_D3D_FAILED(device_->CreateBuffer(&buffer_desc, nullptr, &gvs->material_constants))) {
					MAKI_SAFE_DELETE(gvs);
					return false;
				}
			}
#undef ROUND_SIXTEEN
			vs->set_handle(gvs);
			return true;
		}

		bool d3d_render_core_t::create_shader_program(shader_program_t *s) {
			lock_guard<mutex> lock(mutex_);
			if(!create_vertex_shader(&s->vertex_shader()))
				return false;
			if(!create_fragment_shader(&s->fragment_shader()))
				return false;
			return true;
		}

		bool d3d_render_core_t::create_empty_texture(texture_t *t, uint8_t channels) {
			lock_guard<mutex> lock(mutex_);

			gpu_texture_t *gtex = nullptr;
			ID3D11Texture2D *tex = nullptr;
			ID3D11ShaderResourceView *shader_resource_view = nullptr;
			ID3D11SamplerState *sampler_state = nullptr;

			D3D11_SAMPLER_DESC sampler_desc = {};
			D3D11_TEXTURE2D_DESC texture_desc = {};
			D3D11_SHADER_RESOURCE_VIEW_DESC shader_res_view_desc = {};

			if(channels == 0 || channels > 4 || channels == 3) {
				console_t::error("Unsupport_ed number of channels in image: %d", channels);
				goto failed;
			}

			sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			sampler_desc.MinLOD = 0;
			sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
			if(MAKI_D3D_FAILED(device_->CreateSamplerState(&sampler_desc, &sampler_state))) {
				console_t::error("CreateSamplerState failed");
				goto failed;
			}

			texture_desc.Width = t->width();
			texture_desc.Height = t->height();
			texture_desc.Format = channels_to_format[channels];
			texture_desc.MipLevels = 1;
			texture_desc.ArraySize = 1;
			texture_desc.SampleDesc.Count = 1;
			texture_desc.SampleDesc.Quality = 0;
			texture_desc.Usage = D3D11_USAGE_DEFAULT;
			texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texture_desc.CPUAccessFlags = 0;
			texture_desc.MiscFlags = 0;
			if(MAKI_D3D_FAILED(device_->CreateTexture2D(&texture_desc, nullptr, &tex))) {
				console_t::error("CreateTexture2D failed");
				goto failed;
			}

			shader_res_view_desc.Format = texture_desc.Format;
			shader_res_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shader_res_view_desc.Texture2D.MipLevels = texture_desc.MipLevels;
			if(MAKI_D3D_FAILED(device_->CreateShaderResourceView(tex, &shader_res_view_desc, &shader_resource_view))) {
				console_t::error("CreateShaderResourceView failed");
				goto failed;
			}

			gtex = new gpu_texture_t();
			gtex->shader_resource_view = shader_resource_view;
			gtex->sampler_state = sampler_state;
			t->set_handle(gtex);

			MAKI_SAFE_RELEASE(tex);
			return true;

failed:		
			MAKI_SAFE_RELEASE(tex);
			MAKI_SAFE_RELEASE(shader_resource_view);
			MAKI_SAFE_RELEASE(sampler_state);
			return false;
		}

		bool d3d_render_core_t::create_render_target(texture_t *t) {
			lock_guard<mutex> lock(mutex_);

			gpu_texture_t *gtex = nullptr;
			ID3D11SamplerState *sampler_state = nullptr;
			ID3D11Texture2D *tex = nullptr;
			ID3D11RenderTargetView *render_target_view = nullptr;
			ID3D11ShaderResourceView *shader_resource_view = nullptr;

			D3D11_SAMPLER_DESC sampler_desc = {};
			D3D11_TEXTURE2D_DESC texture_desc = {};
			D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc = {};
			D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};

			// TODO:
			// Should this really be using linear filtering?  Perhaps nearest would be better?
			sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			sampler_desc.MinLOD = 0;
			sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
			if(MAKI_D3D_FAILED(device_->CreateSamplerState(&sampler_desc, &sampler_state))) {
				console_t::error("CreateSamplerState failed");
				goto failed;
			}

			texture_desc.Width = t->width();
			texture_desc.Height = t->height();
			texture_desc.MipLevels = 1;
			texture_desc.ArraySize = 1;
			texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			texture_desc.SampleDesc.Count = 1;
			texture_desc.SampleDesc.Quality = 0;
			texture_desc.Usage = D3D11_USAGE_DEFAULT;
			texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
			texture_desc.CPUAccessFlags = 0;
			texture_desc.MiscFlags = 0;
			if(MAKI_D3D_FAILED(device_->CreateTexture2D(&texture_desc, nullptr, &tex))) {
				console_t::error("CreateTexture2D failed");
				goto failed;
			}

			render_target_view_desc.Format = texture_desc.Format;
			render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			if(MAKI_D3D_FAILED(device_->CreateRenderTargetView(tex, &render_target_view_desc, &render_target_view))) {
				console_t::error("CreateRenderTargetView failed");
				goto failed;
			}

			shader_resource_view_desc.Format = texture_desc.Format;
			shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shader_resource_view_desc.Texture2D.MipLevels = texture_desc.MipLevels;
			if(MAKI_D3D_FAILED(device_->CreateShaderResourceView(tex, &shader_resource_view_desc, &shader_resource_view))) {
				console_t::error("CreateShaderResourceView failed");
				goto failed;
			}
		
			gtex = new gpu_texture_t();
			gtex->shader_resource_view = shader_resource_view;
			gtex->sampler_state = sampler_state;
			gtex->render_target_view = render_target_view;
			t->set_handle(gtex);

			MAKI_SAFE_RELEASE(tex);
			return true;

failed:		
			MAKI_SAFE_RELEASE(tex);
			MAKI_SAFE_RELEASE(shader_resource_view);
			MAKI_SAFE_RELEASE(sampler_state);
			MAKI_SAFE_RELEASE(render_target_view);
			return false;
		}

		bool d3d_render_core_t::create_depth_texture(texture_t *t) {
			lock_guard<mutex> lock(mutex_);

			gpu_texture_t *gtex = nullptr;
			ID3D11SamplerState *sampler_state = nullptr;
			ID3D11Texture2D *tex = nullptr;
			ID3D11DepthStencilView *depth_stencil_view = nullptr;
			ID3D11ShaderResourceView *shader_resource_view = nullptr;

			D3D11_SAMPLER_DESC sampler_desc = {};
			D3D11_TEXTURE2D_DESC texture_desc = {};
			D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
			D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};

			sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			sampler_desc.BorderColor[0] = 1.0f;
			sampler_desc.BorderColor[1] = 1.0f;
			sampler_desc.BorderColor[2] = 1.0f;
			sampler_desc.BorderColor[3] = 1.0f;
			if(MAKI_D3D_FAILED(device_->CreateSamplerState(&sampler_desc, &sampler_state))) {
				console_t::error("CreateSamplerState failed");
				goto failed;
			}

			texture_desc.Width = t->width();
			texture_desc.Height = t->height();
			texture_desc.MipLevels = 1;
			texture_desc.ArraySize = 1;
			texture_desc.Format = DXGI_FORMAT_R32_TYPELESS;
			texture_desc.SampleDesc.Count = 1;
			texture_desc.SampleDesc.Quality = 0;
			texture_desc.Usage = D3D11_USAGE_DEFAULT;
			texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE;
			texture_desc.CPUAccessFlags = 0;
			texture_desc.MiscFlags = 0;
			if(MAKI_D3D_FAILED(device_->CreateTexture2D(&texture_desc, nullptr, &tex))) {
				console_t::error("CreateTexture2D failed");
				goto failed;
			}

			depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
			depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			if(MAKI_D3D_FAILED(device_->CreateDepthStencilView(tex, &depth_stencil_view_desc, &depth_stencil_view))) {
				console_t::error("CreateDepthStencilView failed");
				goto failed;
			}

			shader_resource_view_desc.Format = DXGI_FORMAT_R32_FLOAT;
			shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shader_resource_view_desc.Texture2D.MipLevels = 1;
			if(MAKI_D3D_FAILED(device_->CreateShaderResourceView(tex, &shader_resource_view_desc, &shader_resource_view))) {
				console_t::error("CreateShaderResourceView failed");
				goto failed;
			}
		
			gtex = new gpu_texture_t();
			gtex->shader_resource_view = shader_resource_view;
			gtex->sampler_state = sampler_state;
			gtex->depth_stencil_view = depth_stencil_view;
			t->set_handle(gtex);

			MAKI_SAFE_RELEASE(tex);
			return true;

failed:		
			MAKI_SAFE_RELEASE(tex);
			MAKI_SAFE_RELEASE(shader_resource_view);
			MAKI_SAFE_RELEASE(sampler_state);
			MAKI_SAFE_RELEASE(depth_stencil_view);
			return false;
		}

		bool d3d_render_core_t::create_texture(texture_t *t, char *data, uint32_t data_length) {
			lock_guard<mutex> lock(mutex_);

			gpu_texture_t *gtex = nullptr;
			ID3D11Texture2D *tex = nullptr;
			ID3D11ShaderResourceView *shader_resource_view = nullptr;
			ID3D11SamplerState *sampler_state = nullptr;
			D3D11_TEXTURE2D_DESC desc = {};
			D3D11_SAMPLER_DESC sampler_desc = {};

			if(MAKI_D3D_FAILED(DirectX::CreateDDSTextureFromMemory(device_, (uint8_t *)data, data_length, (ID3D11Resource **)&tex, &shader_resource_view))) {
				console_t::error("Failed to create texture from memory");
				goto failed;
			}

			tex->GetDesc(&desc);
			//t->width() = desc.Width;
			//t->height() = desc.Height;
			MAKI_ASSERT(desc.Width == t->width());
			MAKI_ASSERT(desc.Height == t->height());

			sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
			sampler_desc.MaxAnisotropy = 2;
			sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			sampler_desc.MinLOD = 0;
			sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
			if(MAKI_D3D_FAILED(device_->CreateSamplerState(&sampler_desc, &sampler_state))) {
				console_t::error("CreateSamplerState failed");
				goto failed;
			}
		
			gtex = new gpu_texture_t();
			gtex->shader_resource_view = shader_resource_view;
			gtex->sampler_state = sampler_state;
			t->set_handle(gtex);

			MAKI_SAFE_RELEASE(tex);
			return true;

failed:		
			MAKI_SAFE_RELEASE(tex);
			MAKI_SAFE_RELEASE(shader_resource_view);
			MAKI_SAFE_RELEASE(sampler_state);
			return false;
		}

		void d3d_render_core_t::write_to_texture(texture_t *t, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, uint32_t src_width, uint32_t src_height, uint32_t src_pitch, uint8_t channels, char *src_data) {
			lock_guard<mutex> lock(mutex_);
		
			auto tex = (gpu_texture_t *)t->handle();

			D3D11_BOX box;
			box.left = dst_x;
			box.right = dst_x + src_width;
			box.top = dst_y;
			box.bottom = dst_y + src_height;
			box.front = 0;
			box.back = 1;

			char *p = src_data + (src_y * src_pitch) + (src_x * channels);

			ID3D11Resource *res = nullptr;
			tex->shader_resource_view->GetResource(&res);
			context_->UpdateSubresource(res, 0, &box, p, src_pitch, src_pitch);
			MAKI_SAFE_RELEASE(res);
		}

		void d3d_render_core_t::delete_shader_program(shader_program_t *s) {
			lock_guard<mutex> lock(mutex_);

			auto gvs = (gpu_vertex_shader_t *)s->vertex_shader().handle();
			MAKI_SAFE_DELETE(gvs);
			s->vertex_shader().set_handle(nullptr);

			auto gps = (gpu_fragment_shader_t *)s->fragment_shader().handle();
			MAKI_SAFE_DELETE(gps);
			s->fragment_shader().set_handle(nullptr);
		}

		void d3d_render_core_t::delete_texture(texture_t *t) {
			lock_guard<mutex> lock(mutex_);

			auto gtex = (gpu_texture_t *)t->handle();
			MAKI_SAFE_DELETE(gtex);
			t->set_handle(nullptr);
		}








		bool d3d_render_core_t::is_mode_supported(uint32_t window_width, uint32_t window_height, uint32_t *refresh_numer_out, uint32_t *refresh_denom_out) {
			bool found = false;

			IDXGIFactory *factory = nullptr;
			IDXGIAdapter *adapter = nullptr;
			IDXGIOutput *adapter_output = nullptr;
			uint32_t mode_count;
			DXGI_MODE_DESC *mode_list = nullptr;
			uint32_t refresh_numer = 0, refresh_denom = 0;
			DXGI_ADAPTER_DESC adapter_desc = {};

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
				
			
			for(uint32_t i = 0; i < mode_count; i++) {
				bool res_match = mode_list[i].Width == window_width && mode_list[i].Height == window_height;

				console_t::info("Supported mode: %dx%d @ %f Hz %s",
					mode_list[i].Width,
					mode_list[i].Height,
					mode_list[i].RefreshRate.Denominator != 0 ? mode_list[i].RefreshRate.Numerator / (float)mode_list[i].RefreshRate.Denominator : 0.0f,
					res_match ? "<<< Found!" : "");
				
				if(res_match) {
					if(refresh_numer_out != nullptr)
						*refresh_numer_out = mode_list[i].RefreshRate.Numerator;
					if(refresh_denom_out != nullptr)
						*refresh_denom_out = mode_list[i].RefreshRate.Denominator;
					found = true;
				}
			}
			if(!found)
				console_t::warning("Could not find desired resolution in display mode list");

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
