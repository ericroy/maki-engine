#include "core/core_stdafx.h"
#include "core/MakiRenderCore.h"
#include "core/MakiRenderer.h"
#include "core/MakiShaderProgram.h"

namespace maki
{
	namespace core
	{

		
		render_core_t::render_core_t()
			: thread_t()
		{
		}

		render_core_t::~render_core_t()
		{
		}

		void render_core_t::run()
		{
			console_t::info("Render core thread started");

			render_payload_t payload;
			while(true) {
				input.get(payload);
				if(payload.cmd == render_payload_t::command_init_) {
					console_t::info("Initializing render core");
					init();
					console_t::info("Render core initialized");
					output.put(payload);
				} else if(payload.cmd == render_payload_t::command_abort_) {
					output.put(payload);
					break;
				} else if(payload.cmd == render_payload_t::command_draw_) {
#if MAKI_SORT_DRAW_COMMANDS_IN_RENDER_THREAD
					payload.commands_->sort();
#endif
					draw(*payload.state_, *payload.commands_);
					output.put(payload);
				} else if(payload.cmd == render_payload_t::command_present_) {
					present();
					output.put(payload);
				}
			}

			console_t::info("Render core thread stopped");
		}


		void render_core_t::SetPerFrameConstants(const core::render_state_t &state, const core::shader_t *s, char *buffer)
		{
			using namespace core;

			int32 location = s->engine_frame_uniform_locations_[shader_t::frame_uniform_view_];
			if(location != -1) {
				memcpy(buffer + location, state.view_.vals, 16*sizeof(float));
			}
			location = s->engine_frame_uniform_locations_[shader_t::frame_uniform_projection_];
			if(location != -1) {
				memcpy(buffer + location, state.projection_.vals, sizeof(state.projection_));
			}
			location = s->engine_frame_uniform_locations_[shader_t::frame_uniform_camera_with_height_hear_far_];
			if(location != -1) {
				memcpy(buffer + location, &state.camera_width_height_near_far_, sizeof(state.camera_width_height_near_far_));
			}
			location = s->engine_frame_uniform_locations_[shader_t::frame_uniform_camera_split_distances_];
			if(location != -1) {
				memcpy(buffer + location, &state.camera_split_distances_, sizeof(state.camera_split_distances_));
			}


			location = s->engine_frame_uniform_locations_[shader_t::frame_uniform_light_view_proj_];
			if(location != -1) {
				memcpy(buffer + location, state.light_view_proj, sizeof(state.light_view_proj));
			}
			location = s->engine_frame_uniform_locations_[shader_t::frame_uniform_light_positions_];
			if(location != -1) {
				memcpy(buffer + location, state.light_positions, sizeof(state.light_positions));
			}
			location = s->engine_frame_uniform_locations_[shader_t::frame_uniform_light_directions_];
			if(location != -1) {
				memcpy(buffer + location, state.light_directions, sizeof(state.light_directions));
			}
			location = s->engine_frame_uniform_locations_[shader_t::frame_uniform_light_properties_];
			if(location != -1) {
				// set all lighting slots here so that lights which are no longer in use get effectively turned off
				memcpy(buffer + location, state.light_properties, sizeof(state.light_properties));
			}
			location = s->engine_frame_uniform_locations_[shader_t::frame_uniform_shadow_map_properties_];
			if(location != -1) {
				memcpy(buffer + location, state.shadow_map_properties, state.shadow_light_count_*sizeof(render_state_t::shadow_map_properties_t));
			}
			location = s->engine_frame_uniform_locations_[shader_t::frame_uniform_light_split_regions_];
			if(location != -1) {
				memcpy(buffer + location, state.light_split_regions, state.cascaded_shadow_light_count_*render_state_t::max_cascades_*sizeof(render_state_t::light_split_region_t));
			}
		

			location = s->engine_frame_uniform_locations_[shader_t::frame_uniform_global_ambient_color_];
			if(location != -1) {
				memcpy(buffer + location, &state.global_ambient_color_.x_, sizeof(state.global_ambient_color_));
			}
		}


		void render_core_t::SetPerObjectConstants(const core::shader_t *s, char *buffer, const core::matrix44_t &model, const core::matrix44_t &model_view, const core::matrix44_t &model_view_projection)
		{
			using namespace core;

			int32 location = s->engine_object_uniform_locations_[shader_t::object_uniform_model_];
			if(location != -1) {
				memcpy(buffer + location, model.vals, sizeof(model));
			}

			location = s->engine_object_uniform_locations_[shader_t::object_uniform_model_view_];
			if(location != -1) {
				memcpy(buffer + location, model_view.vals, sizeof(model_view));
			}

			location = s->engine_object_uniform_locations_[shader_t::object_uniform_model_view_projection_];
			if(location != -1) {
				memcpy(buffer + location, model_view_projection.vals, sizeof(model_view_projection));
			}
		}

		void render_core_t::BindMaterialConstants(const core::shader_t *s, bool is_vertex_shader, char *buffer, const core::material_t *mat)
		{
			using namespace core;

			if(is_vertex_shader) {
				for(uint8 i = 0; i < mat->uniform_count_; i++) {
					const material_t::uniform_value_t &val = mat->uniform_values_[i];
					if(val.vsLocation != -1) {
						memcpy(buffer + val.vsLocation, val.data_, val.bytes);
					}
				}
			} else {
				for(uint8 i = 0; i < mat->uniform_count_; i++) {
					const material_t::uniform_value_t &val = mat->uniform_values_[i];
					if(val.psLocation != -1) {
						memcpy(buffer + val.psLocation, val.data_, val.bytes);
					}
				}
			}
		}


	} // namespace core

} // namespace maki
