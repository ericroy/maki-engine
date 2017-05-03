#pragma once
#include <d3d11.h>

namespace maki {
	namespace core {
		class vertex_format_t;
		class array_t;
	} // namespace core

	namespace d3d {

		struct buffer_t {
			ID3D11Buffer *vbos[2];
			uint32_t vertex_count;
			uint32_t face_count;
			uint32_t indices_per_face;
			uint32_t bytes_per_index;
			int32_t index_data_type;
			int32_t geometry_type;
		};

		struct gpu_vertex_shader_t {
		private:
			struct input_layout_t {
				uint64_t vertex_format_key = 0;
				ID3D11InputLayout *input_layout = nullptr;
			};

		public:
			gpu_vertex_shader_t(uint32_t vertex_formats_per_vertex_shader);
			~gpu_vertex_shader_t();
			ID3D11InputLayout *get_or_create_input_layout(ID3D11Device *device, const core::vertex_format_t *vf);
		
			ID3D11VertexShader *vs = nullptr;
			ID3D11Buffer *per_frame_constants = nullptr;
			ID3D11Buffer *per_object_constants = nullptr;
			ID3D11Buffer *material_constants = nullptr;
			uint32_t input_layout_count = 0;
			core::array_t<input_layout_t> input_layouts;
			core::array_t<char> blob;
		};

		struct gpu_pixel_shader_t {
			~gpu_pixel_shader_t();
			ID3D11PixelShader *ps = nullptr;
			ID3D11Buffer *per_frame_constants = nullptr;
			ID3D11Buffer *per_object_constants = nullptr;
			ID3D11Buffer *material_constants = nullptr;
		};

		struct gpu_texture_t {
			~gpu_texture_t();
			ID3D11ShaderResourceView *shader_resource_view = nullptr;
			ID3D11SamplerState *sampler_state = nullptr;
			// Only set if this was created as a depth target
			ID3D11DepthStencilView *depth_stencil_view = nullptr;
			// Only set if this was created as a render target
			ID3D11RenderTargetView *render_target_view = nullptr;
		};

	} // namespace d3d

} // namespace maki
