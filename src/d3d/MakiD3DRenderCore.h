#pragma once
#include "d3d/MakiD3DCommon.h"
#include "d3d/MakiD3DGPUTypes.h"

#pragma comment(lib, "MakiCore.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace maki {
	namespace core {
		class window_t;
		class config_t;
		class vertex_format_t;
		class shader_program_t;
		class texture_set_t;
		class texture_manager_t;
		class material_t;
		class texture_t;
	} // namespace core


	namespace d3d {
	
		class d3d_render_core_t : public core::render_core_t {
		private:
			static const int32_t shadow_map_slot_index_start = 8;

		public:
			d3d_render_core_t(core::window_t *window, const core::config_t *config);
			virtual ~d3d_render_core_t();
			void init();
			inline void draw(const core::render_state_t &state, const core::draw_command_list_t &commands) { generic_draw<d3d_render_core_t>(state, commands); }
			void present();
			void resized(uint32_t width, uint32_t height);
			bool is_mode_supported(uint32_t window_width, uint32_t window_height, uint32_t *refresh_numer_out, uint32_t *refresh_denom_out);

			// GPU resource creation / destruction
			void *upload_buffer(void *buffer, core::vertex_format_t *vf, char *vertex_data, uint32_t vertex_count, char *index_data, uint32_t face_count, uint8_t indices_per_face, uint8_t bytes_per_index, bool dynamic, bool length_changed);
			void free_buffer(void *buffer);
			bool create_shader_program(core::shader_program_t *s);
			void delete_shader_program(core::shader_program_t *s);
			bool create_texture(core::texture_t *t, char *data, uint32_t data_length);
			bool create_empty_texture(core::texture_t *t, uint8_t channels);
			bool create_render_target(core::texture_t *t);
			bool create_depth_texture(core::texture_t *t);
			void write_to_texture(core::texture_t *t, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, uint32_t src_width, uint32_t src_height, uint32_t src_pitch, uint8_t channels, char *src_data);
			void delete_texture(core::texture_t *t);

			// Non-virtual interface
			inline void acquire_context();
			inline void release_context();
			inline void set_render_target_and_depth_stencil(core::render_state_t::render_target_t render_target_type, handle_t render_target, core::render_state_t::depth_stencil_t depth_stencil_type, handle_t depth_stencil);
			inline void set_viewport(const core::rect_t &view_port_rect);
			inline void clear(bool clear_render_target, const float clear_color_values[4], bool clear_depth_stencil, float clear_depth_value);
			inline void set_depth_state(core::render_state_t::depth_test_t depth_test, bool depth_write);
			inline void set_rasterizer_state(core::render_state_t::cull_mode_t cull_mode, bool wire_frame);
			inline void set_blend_state(bool enabled);
			inline void unbind_all_textures();
			inline void bind_shaders(const core::shader_program_t *shader);
			inline void set_per_frame_vertex_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader);
			inline void set_per_frame_pixel_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader);
			inline void bind_shadow_maps(const core::shader_program_t *shader, const core::render_state_t &state);
			inline void set_input_layout(const core::shader_program_t *shader, const core::vertex_format_t *vf);
			inline void set_material_vertex_shader_constants(const core::shader_program_t *shader, const core::material_t *mat);
			inline void set_material_pixel_shader_constants(const core::shader_program_t *shader, const core::material_t *mat);
			inline void bind_textures(const core::shader_program_t *shader, const core::texture_set_t *ts);
			inline void set_per_object_vertex_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader, const core::matrix44_t &matrix, const core::matrix44_t &mv, const core::matrix44_t &mvp);
			inline void set_per_object_pixel_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader, const core::matrix44_t &matrix, const core::matrix44_t &mv, const core::matrix44_t &mvp);
			inline void bind_buffer(void *buffer, const core::vertex_format_t *vf);
			inline void draw_buffer(void *buffer);

		private:
			bool create_pixel_shader(core::shader_t *ps);
			bool create_vertex_shader(core::shader_t *vs);

		private:
			IDXGISwapChain *swap_chain_;
			ID3D11Device *device_;
			ID3D11DeviceContext *context_;
		
			ID3D11RenderTargetView *default_render_target_view_;
			ID3D11DepthStencilView *default_depth_stencil_view_;

			ID3D11RenderTargetView *current_render_target_view_;
			ID3D11DepthStencilView *current_depth_stencil_view_;

			ID3D11RasterizerState *rasterizer_state_;
			ID3D11RasterizerState *rasterizer_state_cull_back_;
			ID3D11RasterizerState *rasterizer_state_cull_front_;
			ID3D11RasterizerState *rasterizer_state_wire_frame_;
			ID3D11RasterizerState *rasterizer_state_wire_frame_cull_back_;
			ID3D11RasterizerState *rasterizer_state_wire_frame_cull_front_;

			ID3D11DepthStencilState *depth_state_;
			ID3D11DepthStencilState *depth_state_write_;
			ID3D11DepthStencilState *depth_state_less_;
			ID3D11DepthStencilState *depth_state_less_write_;
			ID3D11DepthStencilState *depth_state_equal_;
			ID3D11DepthStencilState *depth_state_equal_write_;
			ID3D11DepthStencilState *depth_state_less_equal_;
			ID3D11DepthStencilState *depth_state_less_equal_write_;

			ID3D11BlendState *blend_enabled_;
			ID3D11BlendState *blend_disabled_;

			bool vsync_;
			uint32_t max_vertex_formats_per_vertex_shader_;
			void *null_array_[shadow_map_slot_index_start+core::render_state_t::max_lights_];
			std::mutex mutex_;
		};






		inline void d3d_render_core_t::acquire_context() {
			mutex_.lock();
		}

		inline void d3d_render_core_t::release_context() {
			mutex_.unlock();
		}

		inline void d3d_render_core_t::set_render_target_and_depth_stencil(core::render_state_t::render_target_t render_target_type, handle_t render_target, core::render_state_t::depth_stencil_t depth_stencil_type, handle_t depth_stencil) {
			using namespace core;

			current_render_target_view_ = nullptr;
			if(render_target_type == render_state_t::render_target_default_) {
				current_render_target_view_ = default_render_target_view_;
			} else if(render_target_type == render_state_t::render_target_custom_) {
				current_render_target_view_ = ((gpu_texture_t *)texture_manager_t::get(render_target)->handle_)->render_target_view_;
				if(current_render_target_view_ == nullptr) {
					console_t::error("Tried to set render target to an invalid texture");
				}
			}

			current_depth_stencil_view_ = nullptr;
			if(depth_stencil_type == render_state_t::depth_stencil_default_) {
				current_depth_stencil_view_ = default_depth_stencil_view_;
			} else if(depth_stencil_type == render_state_t::depth_stencil_custom_) {
				current_depth_stencil_view_ = ((gpu_texture_t *)texture_manager_t::get(depth_stencil)->handle_)->depth_stencil_view_;
				if(current_depth_stencil_view_ == nullptr) {
					console_t::error("Tried to set depth stencil to an invalid texture");
				}
			}

			context_->OMSetRenderTargets(1, &current_render_target_view_, current_depth_stencil_view_);
		}

		inline void d3d_render_core_t::set_viewport(const core::rect_t &view_port_rect) {
			D3D11_VIEWPORT viewport;
			memset(&viewport, 0, sizeof(D3D11_VIEWPORT));
			viewport.TopLeftX = view_port_rect.left_;
			viewport.TopLeftY = view_port_rect.top_;
			viewport.Width = view_port_rect.get_width();
			viewport.Height = view_port_rect.get_height();
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			context_->RSSetViewports(1, &viewport);
		}

		inline void d3d_render_core_t::clear(bool clear_render_target, const float clear_color_values[4], bool clear_depth_stencil, float clear_depth_value) {
			if(clear_render_target && current_render_target_view_ != nullptr)
				context_->ClearRenderTargetView(current_render_target_view_, clear_color_values);
			if(clear_depth_stencil && current_depth_stencil_view_ != nullptr)
				context_->ClearDepthStencilView(current_depth_stencil_view_, D3D11_CLEAR_DEPTH, clear_depth_value, 0);
		}

		inline void d3d_render_core_t::set_depth_state(core::render_state_t::depth_test_t depth_test, bool depth_write) {
			using namespace core;
			switch(depth_test) {
			case render_state_t::depth_test_less_:
				context_->OMSetDepthStencilState(depth_write ? depth_state_less_write_ : depth_state_less_, 1);
				break;
			case render_state_t::depth_test_equal_:
				context_->OMSetDepthStencilState(depth_write ? depth_state_equal_write_ : depth_state_equal_, 1);
				break;
			case render_state_t::depth_test_less_equal_:
				context_->OMSetDepthStencilState(depth_write ? depth_state_less_equal_write_ : depth_state_less_equal_, 1);
				break;
			case render_state_t::depth_test_disabled_:
			default:
				context_->OMSetDepthStencilState(depth_write ? depth_state_write_ : depth_state_, 1);
				break;
			}
		}

		inline void d3d_render_core_t::set_rasterizer_state(core::render_state_t::cull_mode_t cull_mode, bool wire_frame) {
			using namespace core;
			switch(cull_mode) {
			case render_state_t::cull_mode_front_:
				context_->RSSetState(wire_frame ? rasterizer_state_wire_frame_cull_front_ : rasterizer_state_cull_front_);
				break;
			case render_state_t::cull_mode_back_:
				context_->RSSetState(wire_frame ? rasterizer_state_wire_frame_cull_back_ : rasterizer_state_cull_back_);
				break;
			case render_state_t::cull_mode_none_:
			default:
				context_->RSSetState(wire_frame ? rasterizer_state_wire_frame_ : rasterizer_state_);
				break;
			}
		}

		inline void d3d_render_core_t::set_blend_state(bool enabled) {
			context_->OMSetBlendState(enabled ? blend_enabled_ : blend_disabled_, nullptr, 0xffffffff);
		}

		inline void d3d_render_core_t::unbind_all_textures() {
			context_->PSSetShaderResources(0, shadow_map_slot_index_start+core::render_state_t::max_shadow_lights_, (ID3D11ShaderResourceView **)null_array_);
		}

		inline void d3d_render_core_t::bind_shaders(const core::shader_program_t *shader) {
			if(shader != nullptr) {
				context_->VSSetShader(((gpu_vertex_shader_t *)shader->vertex_shader_.handle_)->vs_, nullptr, 0);
				context_->PSSetShader(((gpu_pixel_shader_t *)shader->pixel_shader_.handle_)->ps_, nullptr, 0);
			} else {
				context_->VSSetShader(nullptr, nullptr, 0);
				context_->PSSetShader(nullptr, nullptr, 0);
			}
		}

		inline void d3d_render_core_t::set_per_frame_vertex_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader) {
			D3D11_MAPPED_SUBRESOURCE mapped;
			const gpu_vertex_shader_t *gvs = (gpu_vertex_shader_t *)shader->vertex_shader_.handle_;
			context_->Map(gvs->per_frame_constants_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			set_per_frame_constants(state, &shader->vertex_shader_, (char *)mapped.pData);
			context_->Unmap(gvs->per_frame_constants_, 0);
			context_->VSSetConstantBuffers(shader->vertex_shader_.frame_uniform_buffer_location_, 1, &gvs->per_frame_constants_);
		}

		inline void d3d_render_core_t::set_per_frame_pixel_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader) {
			D3D11_MAPPED_SUBRESOURCE mapped;
			const gpu_pixel_shader_t *gps = (gpu_pixel_shader_t *)shader->pixel_shader_.handle_;
			context_->Map(gps->per_frame_constants_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			set_per_frame_constants(state, &shader->pixel_shader_, (char *)mapped.pData);
			context_->Unmap(gps->per_frame_constants_, 0);
			context_->PSSetConstantBuffers(shader->pixel_shader_.frame_uniform_buffer_location_, 1, &gps->per_frame_constants_);
		}

		inline void d3d_render_core_t::bind_shadow_maps(const core::shader_program_t *shader, const core::render_state_t &state) {
			using namespace core;
			ID3D11ShaderResourceView *shadow_views[render_state_t::max_shadow_lights_];
			ID3D11SamplerState *shadow_samplers[render_state_t::max_shadow_lights_];
			for(uint8_t i = 0; i < render_state_t::max_shadow_lights_; i++) {
				if(state.shadow_maps_[i] != HANDLE_NONE) {
					gpu_texture_t *gtex = (gpu_texture_t *)texture_manager_t::get(state.shadow_maps_[i])->handle_;
					shadow_views[i] = gtex->shader_resource_view_;
					shadow_samplers[i] = gtex->sampler_state_;
				} else {
					shadow_views[i] = nullptr;
					shadow_samplers[i] = nullptr;
				}
			}
			context_->PSSetShaderResources(shadow_map_slot_index_start, render_state_t::max_shadow_lights_, shadow_views);
			context_->PSSetSamplers(shadow_map_slot_index_start, render_state_t::max_shadow_lights_, shadow_samplers);
		}

		inline void d3d_render_core_t::set_input_layout(const core::shader_program_t *shader, const core::vertex_format_t *vf) {
			ID3D11InputLayout *layout = ((gpu_vertex_shader_t *)shader->vertex_shader_.handle_)->get_or_create_input_layout(device_, vf);
			context_->IASetInputLayout(layout);
		}

		inline void d3d_render_core_t::set_material_vertex_shader_constants(const core::shader_program_t *shader, const core::material_t *mat) {
			D3D11_MAPPED_SUBRESOURCE mapped;
			const gpu_vertex_shader_t *gvs = (gpu_vertex_shader_t *)shader->vertex_shader_.handle_;
			context_->Map(gvs->material_constants_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			bind_material_constants(&shader->vertex_shader_, true, (char *)mapped.pData, mat);
			context_->Unmap(gvs->material_constants_, 0);
			context_->VSSetConstantBuffers(shader->vertex_shader_.material_uniform_buffer_location_, 1, &gvs->material_constants_);
		}

		inline void d3d_render_core_t::set_material_pixel_shader_constants(const core::shader_program_t *shader, const core::material_t *mat) {
			D3D11_MAPPED_SUBRESOURCE mapped;
			const gpu_pixel_shader_t *gps = (gpu_pixel_shader_t *)shader->pixel_shader_.handle_;
			context_->Map(gps->material_constants_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			bind_material_constants(&shader->pixel_shader_, false, (char *)mapped.pData, mat);
			context_->Unmap(gps->material_constants_, 0);
			context_->PSSetConstantBuffers(shader->pixel_shader_.material_uniform_buffer_location_, 1, &gps->material_constants_);
		}

		inline void d3d_render_core_t::bind_textures(const core::shader_program_t *shader, const core::texture_set_t *ts) {
			using namespace core;

			ID3D11ShaderResourceView *views[texture_set_t::max_textures_per_set_];
			ID3D11SamplerState *samplers[texture_set_t::max_textures_per_set_];

			for(uint8_t i = 0; i < ts->texture_count_; i++) {
				const gpu_texture_t *tex_ = (gpu_texture_t *)texture_manager_t::get(ts->textures_[i])->handle_;
				views[i] = tex_->shader_resource_view_;
				samplers[i] = tex_->sampler_state_;
			}

			context_->PSSetShaderResources(0, ts->texture_count_, views);
			context_->PSSetSamplers(0, ts->texture_count_, samplers);
		}

		inline void d3d_render_core_t::set_per_object_vertex_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader, const core::matrix44_t &matrix, const core::matrix44_t &mv, const core::matrix44_t &mvp) {
			D3D11_MAPPED_SUBRESOURCE mapped;
			const gpu_vertex_shader_t *gvs = (gpu_vertex_shader_t *)shader->vertex_shader_.handle_;
			context_->Map(gvs->per_object_constants_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			set_per_object_constants(&shader->vertex_shader_, (char *)mapped.pData, matrix, mv, mvp);
			context_->Unmap(gvs->per_object_constants_, 0);
			context_->VSSetConstantBuffers(shader->vertex_shader_.object_uniform_buffer_location_, 1, &gvs->per_object_constants_);
		}

		inline void d3d_render_core_t::set_per_object_pixel_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader, const core::matrix44_t &matrix, const core::matrix44_t &mv, const core::matrix44_t &mvp) {
			D3D11_MAPPED_SUBRESOURCE mapped;
			const gpu_pixel_shader_t *gps = (gpu_pixel_shader_t *)shader->pixel_shader_.handle_;
			context_->Map(gps->per_object_constants_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			set_per_object_constants(&shader->pixel_shader_, (char *)mapped.pData, matrix, mv, mvp);
			context_->Unmap(gps->per_object_constants_, 0);
			context_->PSSetConstantBuffers(shader->pixel_shader_.object_uniform_buffer_location_, 1, &gps->per_object_constants_);
		}

		inline void d3d_render_core_t::bind_buffer(void *buffer, const core::vertex_format_t *vf) {
			if(buffer == nullptr)
				return;

			const buffer_t *b = (buffer_t *)buffer;
			uint32_t stride = vf->get_stride();
			uint32_t offset = 0;
			context_->IASetVertexBuffers(0, 1, &b->vbos_[0], &stride, &offset);
			context_->IASetIndexBuffer(b->vbos_[1], (DXGI_FORMAT)b->index_data_type_, 0);
		}

		inline void d3d_render_core_t::draw_buffer(void *buffer) {
			const buffer_t *b = (buffer_t *)buffer;
			context_->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)b->geometry_type_);
			context_->DrawIndexed(b->indices_per_face_*b->face_count_, 0, 0);
		}

		// End of non-virtual interface


	} // namespace d3d

} // namespace maki
