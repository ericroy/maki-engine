#include "core/core_stdafx.h"
#include "core/MakiRenderer.h"
#include "core/MakiConfig.h"
#include "core/MakiRenderPayload.h"
#include "core/MakiRenderCore.h"

namespace maki
{
	namespace core
	{

		renderer_t::renderer_t(window_t *window, render_core_t *core, const config_t *config)
		:	window(window),
			core(core),
			light_dirty_flags(0),
			state(nullptr),
			commands(nullptr)
		{
			uint32 maxDrawCommands = config->get_uint("engine.max_draw_commands_per_pass", default_max_draw_commands_per_pass_);
			for(uint32 i = 0; i < max_render_payloads_; i++) {
				render_states.push_back(new render_state_t());
				command_lists.push_back(new draw_command_list_t(maxDrawCommands));
			}

			current.window_height_ = window->width;
			current.window_height_ = window->height;
			prepare_next_render_state();

			// Kickoff the core thread, and make sure that it is initialized before we continue
			core->Start();
			render_payload_t payload(render_payload_t::command_init_);
			core->input.put(payload);
			core->output.get(payload);
			assert(payload.cmd == render_payload_t::command_init_);

			if(config->get_bool("engine.wire_frame", false)) {
				set_wire_frame_enabled(true);
			}
		}
	
		renderer_t::~renderer_t()
		{
			if(core != nullptr) {
				core->input.put(render_payload_t(render_payload_t::command_abort_));
				core->Join();
				sync_with_core();
			}

			for(uint32 i = 0; i < render_states.size(); i++) {
				MAKI_SAFE_DELETE(render_states[i]);
				MAKI_SAFE_DELETE(command_lists[i]);
			}

			state->clear();
			commands->clear();

			MAKI_SAFE_DELETE(state);
			MAKI_SAFE_DELETE(commands);
		}

		bool renderer_t::begin()
		{
			if(core != nullptr) {
				sync_with_core();
				current.window_height_ = window->width;
				current.window_height_ = window->height;
				current.clear_render_target_ = false;
				current.clear_depth_stencil_ = false;
				return true;
			}
			return false;
		}

		void renderer_t::end()
		{
			render_payload_t payload(render_payload_t::command_present_);
			core->input.put(payload);
		}

		void renderer_t::sync_with_core()
		{
			uint32 outstanding = max_render_payloads_ - render_states.size() - 1;
			while(outstanding != 0) {
				render_payload_t temp;
				core->output.get(temp);
				if(temp.state_ != nullptr) {
					temp.state_->clear();
					temp.commands_->clear();
					render_states.push_back(temp.state_);
					command_lists.push_back(temp.commands_);
					outstanding--;
				}
			}
		}

		void renderer_t::prepare_next_render_state()
		{
			assert(render_states.size() > 0);

			state = render_states.back();
			render_states.pop_back();
			commands = command_lists.back();
			command_lists.pop_back();
		}

		void renderer_t::set_render_target_and_depth_stencil(render_state_t::render_target_t render_target_type, handle_t render_target, render_state_t::depth_stencil_t depth_stencil_type, handle_t depth_stencil)
		{
			current.render_target_type_ = render_target_type;
			if(render_target_type == render_state_t::render_target_custom_) {
				texture_manager_t::add_ref(render_target);
				texture_manager_t::free(current.render_target_);
				current.render_target_ = render_target;
			} else {
				texture_manager_t::free(current.render_target_);
			}

			current.depth_stencil_type_ = depth_stencil_type;
			if(depth_stencil_type == render_state_t::depth_stencil_custom_) {
				texture_manager_t::add_ref(depth_stencil);
				texture_manager_t::free(current.depth_stencil_);
				current.depth_stencil_ = depth_stencil;
			} else {
				texture_manager_t::free(current.depth_stencil_);
			}
		}

		void renderer_t::set_ortho_projection(const frustum_t &frustum)
		{
			current.camera_width_height_near_far_ = vector4_t(frustum.get_width(), frustum.get_height(), frustum.nearPlane, frustum.farPlane);
			matrix44_t::ortho(frustum, current.projection_);
		}

		void renderer_t::set_perspective_projection(const frustum_t &frustum)
		{
			current.camera_width_height_near_far_ = vector4_t(frustum.get_width(), frustum.get_height(), frustum.nearPlane, frustum.farPlane);
			matrix44_t::perspective(frustum, current.projection_);
		}

		void renderer_t::set_light(uint32 light_index, const render_state_t::light_properties_t *props, const render_state_t::shadow_map_properties_t *shad_props, const matrix44_t *matrix, float fov, handle_t depth_buffer)
		{
			light_dirty_flags |= (1<<light_index);

			if(props == nullptr || (props->flags & render_state_t::light_flag_on_) == 0) {
				current.light_properties[light_index].flags &= ~render_state_t::light_flag_on_;
				memset(&current.light_properties[light_index], 0, sizeof(render_state_t::light_properties_t));
				texture_manager_t::free(current.shadow_maps[light_index]);
				return;
			}

			memcpy(&current.light_properties[light_index], props, sizeof(render_state_t::light_properties_t));
		
			assert(matrix != nullptr);
			current.light_world[light_index] = *matrix;
			matrix44_t::affine_inverse(*matrix, current.light_view[light_index]);

			if(fov == 0.0f) {
				float wo2 = props->width_height_near_far.x_ / 2.0f;
				float ho2 = props->width_height_near_far.y_ / 2.0f;
				matrix44_t::ortho(-wo2, wo2, -ho2, ho2, props->width_height_near_far.z_, props->width_height_near_far.w_, current.light_proj[light_index]);
			} else {
				matrix44_t::perspective(fov, props->width_height_near_far.x_ / props->width_height_near_far.y_, props->width_height_near_far.z_, props->width_height_near_far.w_, current.light_proj[light_index]);
			}
		
			if((props->flags & render_state_t::light_flag_shadow_) != 0 && depth_buffer != HANDLE_NONE) {
				assert(light_index < render_state_t::max_shadow_lights_);
				texture_manager_t::add_ref(depth_buffer);
				texture_manager_t::free(current.shadow_maps[light_index]);
				current.shadow_maps[light_index] = depth_buffer;
				current.shadow_map_properties[light_index] = *shad_props;
			} else {
				if(light_index < render_state_t::max_shadow_lights_) {
					current.shadow_map_properties[light_index].size = Vector2(0.0f);
					texture_manager_t::free(current.shadow_maps[light_index]);
				}
			}
		}

		void renderer_t::set_light_cascade(uint32 light_index, uint32 cascade_index, const frustum_t &frustum)
		{
			assert(light_index < current.cascaded_shadow_light_count_);
			assert(cascade_index < render_state_t::max_cascades_);

			render_state_t::light_split_region_t &region = current.light_split_regions[light_index][cascade_index];

			matrix44_t::ortho(frustum, region.view_proj_);

			// light_t must have been set already!!!!!!
			region.view_proj_ = region.view_proj_ * current.light_view[light_index];
		
			region.width_height_near_far_ = vector4_t(frustum.get_width(), frustum.get_height(), frustum.nearPlane, frustum.farPlane);
		}

		void renderer_t::submit()
		{

			// Finish preparing lighting info now that view matrix is certain
			//if(light_dirty_flags != 0) {
				const vector4_t pos(0.0f);
				const vector4_t dir(0.0f, 0.0f, -1.0f, 0.0f);
				for(uint32 i = 0; i < current.light_count_; i++) {
					//if((light_dirty_flags & (1<<i)) != 0 && (current.light_properties[i].flags & render_state_t::light_flag_on_) != 0) {
						matrix44_t toViewSpace = current.view_ * current.light_world[i];
						current.light_positions[i] = toViewSpace * pos;
						current.light_directions[i] = toViewSpace * dir;
						current.light_view_proj[i] = current.light_proj[i] * current.light_view[i];
					//}
				}
				light_dirty_flags = 0;
			//}

			// Duplicate our current renderstate (to be sent with the payload)
			state->copy(current);

	#if !MAKI_SORT_DRAW_COMMANDS_IN_RENDER_THREAD
			commands->sort();
	#endif

			// Send renderstate and commands to the core for processing
			render_payload_t payload(render_payload_t::command_draw_);
			payload.state_ = state;
			payload.commands_ = commands;
			core->input.put(payload);

			state = nullptr;
			commands = nullptr;

			current.clear_render_target_ = false;
			current.clear_depth_stencil_ = false;

			prepare_next_render_state();
		}

	} // namespace core

} // namespace maki
