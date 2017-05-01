#include "core/MakiRenderCore.h"
#include "core/MakiConsole.h"
#include "core/MakiRenderer.h"
#include "core/MakiShaderProgram.h"

namespace maki {
	namespace core {

		void render_core_t::run() {
			console_t::info("Render core thread started");

			render_payload_t payload;
			while(true) {
				input.get(payload);
				if(payload.cmd_ == render_payload_t::command_init) {
					console_t::info("Initializing render core");
					init();
					console_t::info("Render core initialized");
					output.put(payload);
				} else if(payload.cmd_ == render_payload_t::command_abort) {
					output.put(payload);
					break;
				} else if(payload.cmd_ == render_payload_t::command_draw) {
#if MAKI_SORT_DRAW_COMMANDS_IN_RENDER_THREAD
					payload.commands_->sort();
#endif
					draw(*payload.state_, *payload.commands);
					output.put(payload);
				} else if(payload.cmd_ == render_payload_t::command_present) {
					present();
					output.put(payload);
				}
			}

			console_t::info("Render core thread stopped");
		}


		void render_core_t::set_per_frame_constants(const core::render_state_t &state, const core::shader_t *s, char *buffer) {
			using namespace core;

			int32_t location = s->engine_frame_uniform_locations[shader_t::frame_uniform_view];
			if(location != -1)
				memcpy(buffer + location, state.view_.vals, 16*sizeof(float));

			location = s->engine_frame_uniform_locations[shader_t::frame_uniform_projection];
			if(location != -1)
				memcpy(buffer + location, state.projection_.vals, sizeof(state.projection));

			location = s->engine_frame_uniform_locations[shader_t::frame_uniform_camera_with_height_hear_far];
			if(location != -1)
				memcpy(buffer + location, &state.camera_width_height_near_far_, sizeof(state.camera_width_height_near_far));

			location = s->engine_frame_uniform_locations[shader_t::frame_uniform_camera_split_distances];
			if(location != -1)
				memcpy(buffer + location, &state.camera_split_distances_, sizeof(state.camera_split_distances));


			location = s->engine_frame_uniform_locations[shader_t::frame_uniform_light_view_proj];
			if(location != -1)
				memcpy(buffer + location, state.light_view_proj_, sizeof(state.light_view_proj));

			location = s->engine_frame_uniform_locations[shader_t::frame_uniform_light_positions];
			if(location != -1)
				memcpy(buffer + location, state.light_positions_, sizeof(state.light_positions));

			location = s->engine_frame_uniform_locations[shader_t::frame_uniform_light_directions];
			if(location != -1)
				memcpy(buffer + location, state.light_directions_, sizeof(state.light_directions));

			// set all lighting slots here so that lights which are no longer in use get effectively turned off
			location = s->engine_frame_uniform_locations[shader_t::frame_uniform_light_properties];
			if(location != -1)
				memcpy(buffer + location, state.light_properties_, sizeof(state.light_properties));

			location = s->engine_frame_uniform_locations[shader_t::frame_uniform_shadow_map_properties];
			if(location != -1)
				memcpy(buffer + location, state.shadow_map_properties_, state.shadow_light_count_*sizeof(render_state_t::shadow_map_properties_t));

			location = s->engine_frame_uniform_locations[shader_t::frame_uniform_light_split_regions];
			if(location != -1)
				memcpy(buffer + location, state.light_split_regions_, state.cascaded_shadow_light_count_*render_state_t::max_cascades_*sizeof(render_state_t::light_split_region_t));


			location = s->engine_frame_uniform_locations[shader_t::frame_uniform_global_ambient_color];
			if(location != -1)
				memcpy(buffer + location, &state.global_ambient_color_.x, sizeof(state.global_ambient_color));
		}


		void render_core_t::set_per_object_constants(const core::shader_t *s, char *buffer, const core::matrix44_t &model, const core::matrix44_t &model_view, const core::matrix44_t &model_view_projection) {
			using namespace core;

			int32_t location = s->engine_object_uniform_locations[shader_t::object_uniform_model];
			if(location != -1)
				memcpy(buffer + location, model.vals, sizeof(model));

			location = s->engine_object_uniform_locations[shader_t::object_uniform_model_view];
			if(location != -1)
				memcpy(buffer + location, model_view.vals, sizeof(model_view));

			location = s->engine_object_uniform_locations[shader_t::object_uniform_model_view_projection];
			if(location != -1)
				memcpy(buffer + location, model_view_projection.vals, sizeof(model_view_projection));
		}

		void render_core_t::bind_material_constants(const core::shader_t *s, bool is_vertex_shader, char *buffer, const core::material_t *mat) {
			using namespace core;

			if(is_vertex_shader) {
				for(uint8_t i = 0; i < mat->uniform_count; i++) {
					const auto &val = mat->uniform_values[i];
					if(val.vs_location != -1)
						memcpy(buffer + val.vs_location, val.data, val.bytes);
				}
			} else {
				for(uint8_t i = 0; i < mat->uniform_count; i++) {
					const auto &val = mat->uniform_values[i];
					if(val.ps_location != -1)
						memcpy(buffer + val.ps_location, val.data, val.bytes);
				}
			}
		}


	} // namespace core
} // namespace maki
