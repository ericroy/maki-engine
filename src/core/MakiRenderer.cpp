#include "core_/core_stdafx.h"
#include "core_/MakiRenderer.h"
#include "core_/MakiConfig.h"
#include "core_/MakiRenderPayload.h"
#include "core_/MakiRenderCore.h"

namespace maki
{
	namespace core_
	{

		renderer_t::renderer_t(window_t *window, render_core_t *core, const config_t *config)
		:	window_(window),
			core_(core),
			light_dirty_flags_(0),
			state_(nullptr),
			commands_(nullptr)
		{
			uint32 max_draw_commands = config->get_uint("engine.max_draw_commands_per_pass", default_max_draw_commands_per_pass_);
			for(uint32 i = 0; i < max_render_payloads_; i++) {
				render_states_.push_back(new render_state_t());
				command_lists_.push_back(new draw_command_list_t(max_draw_commands));
			}

			current_.window_height_ = window_->width_;
			current_.window_height_ = window_->height_;
			prepare_next_render_state();

			// Kickoff the core_ thread, and make sure that it is initialized before we continue
			core_->start();
			render_payload_t payload(render_payload_t::command_init_);
			core_->input.put(payload);
			core_->output.get(payload);
			assert(payload.cmd == render_payload_t::command_init_);

			if(config->get_bool("engine.wire_frame", false)) {
				set_wire_frame_enabled(true);
			}
		}
	
		renderer_t::~renderer_t()
		{
			if(core_ != nullptr) {
				core_->input.put(render_payload_t(render_payload_t::command_abort_));
				core_->join();
				sync_with_core();
			}

			for(uint32 i = 0; i < render_states_.size(); i++) {
				MAKI_SAFE_DELETE(render_states_[i]);
				MAKI_SAFE_DELETE(command_lists_[i]);
			}

			state_->clear();
			commands_->clear();

			MAKI_SAFE_DELETE(state_);
			MAKI_SAFE_DELETE(commands_);
		}

		bool renderer_t::begin()
		{
			if(core_ != nullptr) {
				sync_with_core();
				current_.window_height_ = window_->width_;
				current_.window_height_ = window_->height_;
				current_.clear_render_target_ = false;
				current_.clear_depth_stencil_ = false;
				return true;
			}
			return false;
		}

		void renderer_t::end()
		{
			render_payload_t payload(render_payload_t::command_present_);
			core_->input.put(payload);
		}

		void renderer_t::sync_with_core()
		{
			uint32 outstanding = max_render_payloads_ - render_states_.size() - 1;
			while(outstanding != 0) {
				render_payload_t temp;
				core_->output.get(temp);
				if(temp.state_ != nullptr) {
					temp.state_->clear();
					temp.commands_->clear();
					render_states_.push_back(temp.state_);
					command_lists_.push_back(temp.commands_);
					outstanding--;
				}
			}
		}

		void renderer_t::prepare_next_render_state()
		{
			assert(render_states_.size() > 0);

			state_ = render_states_.back();
			render_states_.pop_back();
			commands_ = command_lists_.back();
			command_lists_.pop_back();
		}

		void renderer_t::set_render_target_and_depth_stencil(render_state_t::render_target_t render_target_type, handle_t render_target, render_state_t::depth_stencil_t depth_stencil_type, handle_t depth_stencil)
		{
			current_.render_target_type_ = render_target_type;
			if(render_target_type == render_state_t::render_target_custom_) {
				texture_manager_t::add_ref(render_target);
				texture_manager_t::free(current_.render_target_);
				current_.render_target_ = render_target;
			} else {
				texture_manager_t::free(current_.render_target_);
			}

			current_.depth_stencil_type_ = depth_stencil_type;
			if(depth_stencil_type == render_state_t::depth_stencil_custom_) {
				texture_manager_t::add_ref(depth_stencil);
				texture_manager_t::free(current_.depth_stencil_);
				current_.depth_stencil_ = depth_stencil;
			} else {
				texture_manager_t::free(current_.depth_stencil_);
			}
		}

		void renderer_t::set_ortho_projection(const frustum_t &frustum)
		{
			current_.camera_width_height_near_far_ = vector4_t(frustum.get_width(), frustum.get_height(), frustum.near_plane_, frustum.far_plane_);
			matrix44_t::ortho(frustum, current_.projection_);
		}

		void renderer_t::set_perspective_projection(const frustum_t &frustum)
		{
			current_.camera_width_height_near_far_ = vector4_t(frustum.get_width(), frustum.get_height(), frustum.near_plane_, frustum.far_plane_);
			matrix44_t::perspective(frustum, current_.projection_);
		}

		void renderer_t::set_light(uint32 light_index, const render_state_t::light_properties_t *props, const render_state_t::shadow_map_properties_t *shad_props, const matrix44_t *matrix, float fov, handle_t depth_buffer)
		{
			light_dirty_flags_ |= (1<<light_index);

			if(props == nullptr || (props->flags & render_state_t::light_flag_on_) == 0) {
				current_.light_properties_[light_index].flags &= ~render_state_t::light_flag_on_;
				memset(&current_.light_properties_[light_index], 0, sizeof(render_state_t::light_properties_t));
				texture_manager_t::free(current_.shadow_maps[light_index]);
				return;
			}

			memcpy(&current_.light_properties_[light_index], props, sizeof(render_state_t::light_properties_t));
		
			assert(matrix != nullptr);
			current_.light_world[light_index] = *matrix;
			matrix44_t::affine_inverse(*matrix, current_.light_view[light_index]);

			if(fov == 0.0f) {
				float wo2 = props->width_height_near_far.x_ / 2.0f;
				float ho2 = props->width_height_near_far.y_ / 2.0f;
				matrix44_t::ortho(-wo2, wo2, -ho2, ho2, props->width_height_near_far.z_, props->width_height_near_far.w_, current_.light_proj[light_index]);
			} else {
				matrix44_t::perspective(fov, props->width_height_near_far.x_ / props->width_height_near_far.y_, props->width_height_near_far.z_, props->width_height_near_far.w_, current_.light_proj[light_index]);
			}
		
			if((props->flags & render_state_t::light_flag_shadow_) != 0 && depth_buffer != HANDLE_NONE) {
				assert(light_index < render_state_t::max_shadow_lights_);
				texture_manager_t::add_ref(depth_buffer);
				texture_manager_t::free(current_.shadow_maps[light_index]);
				current_.shadow_maps[light_index] = depth_buffer;
				current_.shadow_map_properties_[light_index] = *shad_props;
			} else {
				if(light_index < render_state_t::max_shadow_lights_) {
					current_.shadow_map_properties_[light_index].size = vector2_t(0.0f);
					texture_manager_t::free(current_.shadow_maps[light_index]);
				}
			}
		}

		void renderer_t::set_light_cascade(uint32 light_index, uint32 cascade_index, const frustum_t &frustum)
		{
			assert(light_index < current_.cascaded_shadow_light_count_);
			assert(cascade_index < render_state_t::max_cascades_);

			render_state_t::light_split_region_t &region = current_.light_split_regions_[light_index][cascade_index];

			matrix44_t::ortho(frustum, region.view_proj_);

			// light_t must have been set already!!!!!!
			region.view_proj_ = region.view_proj_ * current_.light_view[light_index];
		
			region.width_height_near_far_ = vector4_t(frustum.get_width(), frustum.get_height(), frustum.near_plane_, frustum.far_plane_);
		}

		void renderer_t::submit()
		{

			// Finish preparing lighting info now that view matrix is certain
			//if(light_dirty_flags_ != 0) {
				const vector4_t pos(0.0f);
				const vector4_t dir(0.0f, 0.0f, -1.0f, 0.0f);
				for(uint32 i = 0; i < current_.light_count_; i++) {
					//if((light_dirty_flags_ & (1<<i)) != 0 && (current_.light_properties_[i].flags & render_state_t::light_flag_on_) != 0) {
						matrix44_t to_view_space = current_.view_ * current_.light_world[i];
						current_.light_positions[i] = to_view_space * pos;
						current_.light_directions_[i] = to_view_space * dir;
						current_.light_view_proj[i] = current_.light_proj[i] * current_.light_view[i];
					//}
				}
				light_dirty_flags_ = 0;
			//}

			// Duplicate our current_ renderstate (to be sent with the payload)
			state_->copy(current_);

	#if !MAKI_SORT_DRAW_COMMANDS_IN_RENDER_THREAD
			commands_->sort();
	#endif

			// Send renderstate and commands_ to the core_ for processing
			render_payload_t payload(render_payload_t::command_draw_);
			payload.state_ = state_;
			payload.commands_ = commands_;
			core_->input.put(payload);

			state_ = nullptr;
			commands_ = nullptr;

			current_.clear_render_target_ = false;
			current_.clear_depth_stencil_ = false;

			prepare_next_render_state();
		}

	} // namespace core_

} // namespace maki
