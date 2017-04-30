#include "core/core_stdafx.h"
#include "core/MakiSkeletonAnimation.h"
#include "core/MakiSkeleton.h"
#include "core/MakiDocument.h"

namespace maki
{
	namespace core
	{
	
		float skeleton_animation_t::debug_rate_coeff_ = 1.0f;
	

		skeleton_animation_t::state_t::state_t(uint32_t size)
			: current_frame_(0.0f)
		{
			set_size(size);
		}
	
		skeleton_animation_t::state_t::state_t()
			: current_frame_(0.0f)
		{
		}
	
		void skeleton_animation_t::state_t::set_size(uint32_t size)
		{
			current_key_frames_.set_size(size);
			current_key_frames_.zero();
		}






		skeleton_animation_t::skeleton_animation_t()
			: resource_t(),
			frame_count_(0),
			frame_rate_(0.0f)
		{
		}

		skeleton_animation_t::~skeleton_animation_t()
		{
			for(uint32_t i = 0; i < data_.count_; i++) {
				data_[i].free();
			}
		}

		bool skeleton_animation_t::load(rid_t rid)
		{
			if(data_.data_ != nullptr) {
				data_.free();
			}
			frame_count_ = 0;
			frame_rate_ = 0.0f;

			document_t doc;
			if(!doc.load(rid)) {
				console_t::error("Could not parse file as document <rid %d>", rid);
				return false;
			}

			uint32_t count;
			if(!doc.root_->resolve_as_uint("bone_count.#0", &count)) {
				console_t::error("Could not find bone count in animation document");
				return false;
			}
			data_.set_size(count);
			data_.zero();

			if(!doc.root_->resolve_as_float("frame_rate.#0", &frame_rate_)) {
				console_t::error("Could not find frame rate in animation document");
				return false;
			}
			if(!doc.root_->resolve_as_uint("frame_count.#0", &frame_count_)) {
				console_t::error("Could not find frame count in animation document");
				return false;
			}
		
			for(uint32_t i = 3; i < doc.root_->count_; i++) {
				document_t::node_t *bone_node = doc.root_->children_[i];
				uint32_t bone = i-3;

				data_[bone].set_size(bone_node->count_);
				data_[bone].zero();

				for(uint32_t j = 0; j < bone_node->count_; j++) {
					document_t::node_t *n = bone_node->children_[j];
					key_frame_t &kf = data_[bone][j];

					kf.frame_ = n->value_as_uint();
					kf.offset_ = vector3_t(n->children_[0]->value_as_float(), n->children_[1]->value_as_float(), n->children_[2]->value_as_float());

					vector3_t euler_angles = vector3_t(n->children_[3]->value_as_float(), n->children_[4]->value_as_float(), n->children_[5]->value_as_float()) * MAKI_DEG_TO_RAD;
					kf.rot_.from_euler_angles(euler_angles);
				}
			}

			this->rid_ = rid;
			return true;
		}

		void skeleton_animation_t::advance_state(float time_delta, state_t &state, array_t<skeleton_t::joint_t> &pose, bool loop, float rate_coeff)
		{
			state.current_frame_ += time_delta * rate_coeff * debug_rate_coeff_ * frame_rate_;
			if(state.current_frame_ >= frame_count_) {
				if(loop) {
					state.current_frame_ = state.current_frame_ - ((uint32_t)state.current_frame_ / frame_count_)*frame_count_;
				} else {
					state.current_frame_ = (float)frame_count_;
				}
			}

			for(uint32_t i = 0; i < data_.count_; i++) {
				uint32_t &current_index = state.current_key_frames_[i];
				const array_t<key_frame_t> &bone_frames = data_[i];

				if(bone_frames.count_ == 0) {
					continue;
				}
				if(bone_frames.count_ == 1) {
					pose[i].offset_ = bone_frames[0].offset_;
					pose[i].rot_ = bone_frames[0].rot_;
					continue;
				}
			
				// Advance through the frames for this bone until we find the two keyframes
				// that bound the current frame time.
				uint32_t next_frame_index = (current_index + 1) % frame_count_;
				const key_frame_t *next = &bone_frames[next_frame_index];
				const key_frame_t *curr = &bone_frames[current_index];
				while(true) {
					// Current and next frame bound the playhead
					if(state.current_frame_ >= curr->frame_ && state.current_frame_ < next->frame_) {
						break;
					}

					// Wrap-around case
					if(next->frame_ == 0 && state.current_frame_ >= curr->frame_) {
						break;
					}

					current_index = (current_index + 1) % frame_count_;
					next_frame_index = (current_index + 1) % frame_count_;
					next = &bone_frames[next_frame_index];
					curr = &bone_frames[current_index];
				}

				uint32_t distance;
				if(next_frame_index < current_index) {
					// Wrapped
					distance = frame_count_ - curr->frame_ + next->frame_;
				} else {
					distance = next->frame_ - curr->frame_;
				}

				float frac = (state.current_frame_ - curr->frame_) / distance;
				pose[i].offset_ = curr->offset_ * (1.0f - frac) + next->offset_ * frac;
				pose[i].rot_ = quaternion_t::nlerp(frac, curr->rot_, next->rot_);
			}
		}
	

	} // namespace core

} // namespace maki
