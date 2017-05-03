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
				if(payload.cmd == render_payload_t::command_init) {
					console_t::info("Initializing render core");
					init();
					console_t::info("Render core initialized");
					output.put(payload);
				} else if(payload.cmd == render_payload_t::command_abort) {
					output.put(payload);
					break;
				} else if(payload.cmd == render_payload_t::command_draw) {
#if MAKI_SORT_DRAW_COMMANDS_IN_RENDER_THREAD
					payload.commands_->sort();
#endif
					draw(*payload.state, *payload.commands);
					output.put(payload);
				} else if(payload.cmd == render_payload_t::command_present) {
					present();
					output.put(payload);
				}
			}

			console_t::info("Render core thread stopped");
		}


		void render_core_t::set_per_frame_constants(const render_state_t &state, const shader_t *s, char *buffer) {
			using namespace core;

			int32_t location = s->get_engine_frame_constant_location(frame_constant_view);
			if(location != -1)
				memcpy(buffer + location, state.view.vals, 16 * sizeof(float));

			location = s->get_engine_frame_constant_location(frame_constant_projection);
			if(location != -1)
				memcpy(buffer + location, state.projection.vals, sizeof(state.projection));

			location = s->get_engine_frame_constant_location(frame_constant_camera_with_height_hear_far);
			if(location != -1)
				memcpy(buffer + location, &state.camera_width_height_near_far, sizeof(state.camera_width_height_near_far));

			location = s->get_engine_frame_constant_location(frame_constant_camera_split_distances);
			if(location != -1)
				memcpy(buffer + location, &state.camera_split_distances, sizeof(state.camera_split_distances));


			location = s->get_engine_frame_constant_location(frame_constant_light_view_proj);
			if(location != -1)
				memcpy(buffer + location, state.light_view_proj, sizeof(state.light_view_proj));

			location = s->get_engine_frame_constant_location(frame_constant_light_positions);
			if(location != -1)
				memcpy(buffer + location, state.light_positions, sizeof(state.light_positions));

			location = s->get_engine_frame_constant_location(frame_constant_light_directions);
			if(location != -1)
				memcpy(buffer + location, state.light_directions, sizeof(state.light_directions));

			// set all lighting slots here so that lights which are no longer in use get effectively turned off
			location = s->get_engine_frame_constant_location(frame_constant_light_properties);
			if(location != -1)
				memcpy(buffer + location, state.light_properties, sizeof(state.light_properties));

			location = s->get_engine_frame_constant_location(frame_constant_shadow_map_properties);
			if(location != -1)
				memcpy(buffer + location, state.shadow_map_properties, state.shadow_light_count * sizeof(render_state_t::shadow_map_properties_t));

			location = s->get_engine_frame_constant_location(frame_constant_light_split_regions);
			if(location != -1)
				memcpy(buffer + location, state.light_split_regions, state.cascaded_shadow_light_count * render_state_t::max_cascades * sizeof(render_state_t::light_split_region_t));


			location = s->get_engine_frame_constant_location(frame_constant_global_ambient_color);
			if(location != -1)
				memcpy(buffer + location, &state.global_ambient_color.x, sizeof(state.global_ambient_color));
		}


		void render_core_t::set_per_object_constants(const shader_t *s, char *buffer, const matrix44_t &model, const matrix44_t &model_view, const matrix44_t &model_view_projection) {
			using namespace core;

			int32_t location = s->get_engine_object_constant_location(object_constant_model);
			if(location != -1)
				memcpy(buffer + location, model.vals, sizeof(model));

			location = s->get_engine_object_constant_location(object_constant_model_view);
			if(location != -1)
				memcpy(buffer + location, model_view.vals, sizeof(model_view));

			location = s->get_engine_object_constant_location(object_constant_model_view_projection);
			if(location != -1)
				memcpy(buffer + location, model_view_projection.vals, sizeof(model_view_projection));
		}

		void render_core_t::bind_material_constants(const shader_t *s, bool is_vertex_shader, char *buffer, const material_t *mat) {
			using namespace core;

			if(is_vertex_shader) {
				for(uint8_t i = 0; i < mat->constant_count(); i++) {
					const auto &val = mat->get_constant(i);
					if(val.vs_location != -1)
						memcpy(buffer + val.vs_location, val.data.data(), val.data.length());
				}
			} else {
				for(uint8_t i = 0; i < mat->constant_count(); i++) {
					const auto &val = mat->get_constant(i);
					if(val.ps_location != -1)
						memcpy(buffer + val.ps_location, val.data.data(), val.data.length());
				}
			}
		}


	} // namespace core
} // namespace maki
