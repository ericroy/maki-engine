#include "core/MakiSkeletonAnimation.h"
#include "core/MakiSkeleton.h"
#include "core/MakiDocument.h"

namespace maki {
	namespace core {
	
		float skeleton_animation_t::debug_rate_coeff = 1.0f;
	

		skeleton_animation_t::state_t::state_t(uint64_t length) {
			set_length(length);
		}

		void skeleton_animation_t::state_t::set_size(uint64_t length) {
			current_key_frames.set_length(length);
			current_key_frames.zero();
		}



		skeleton_animation_t::~skeleton_animation_t() {
			for (auto &kf : data_)
				kf.free();
		}

		bool skeleton_animation_t::load(rid_t rid) {
			if(data_.data() != nullptr) 
				data_.free();

			frame_count_ = 0;
			frame_rate_ = 0.0f;

			document_t doc;
			if(!doc.load(rid)) {
				console_t::error("Could not parse file as document <rid %u>", rid);
				return false;
			}

			uint64_t count;
			if(!doc.root_->resolve_as_uint("bone_count.#0", &count)) {
				console_t::error("Could not find bone count in animation document");
				return false;
			}
			data_.set_length(count);
			data_.zero();

			if(!doc.root_->resolve_as_float("frame_rate.#0", &frame_rate_)) {
				console_t::error("Could not find frame rate in animation document");
				return false;
			}
			if(!doc.root_->resolve_as_uint("frame_count.#0", &frame_count_)) {
				console_t::error("Could not find frame count in animation document");
				return false;
			}
		
			for(uint64_t i = 3; i < doc.root().length(); i++) {
				const auto &bone_node = doc.root()[i];
				uint64_t bone = i - 3;

				data_[bone].set_size(bone_node->length());
				data_[bone].zero();

				for(uint64_t j = 0; j < bone_node.length(); j++) {
					const auto &n = bone_node[j];
					auto &kf = data_[bone][j];

					kf.frame = n.value_as_uint();
					kf.offset = vector3_t(n[0].value_as_float(), n[1].value_as_float(), n[2].value_as_float());

					vector3_t euler_angles = vector3_t(n[3].value_as_float(), n[4].value_as_float(), n[5].value_as_float()) * MAKI_DEG_TO_RAD;
					kf.rot.from_euler_angles(euler_angles);
				}
			}

			this->rid = rid;
			return true;
		}

		void skeleton_animation_t::advance_state(float time_delta, state_t &state, array_t<skeleton_t::joint_t> &pose, bool loop, float rate_coeff) {
			state.current_frame += time_delta * rate_coeff * debug_rate_coeff_ * frame_rate_;
			if(state.current_frame >= frame_count_) {
				if(loop)
					state.current_frame = state.current_frame - ((uint64_t)state.current_frame / frame_count_)*frame_count_;
				else
					state.current_frame = (float)frame_count_;
			}

			for(uint64_t i = 0; i < data_.length(); i++) {
				uint64_t &current_index = state.current_key_frames[i];
				const auto &bone_frames = data_[i];

				if(bone_frames.length() == 0)
					continue;

				if(bone_frames.length() == 1) {
					pose[i].offset = bone_frames[0].offset;
					pose[i].rot = bone_frames[0].rot;
					continue;
				}
			
				// Advance through the frames for this bone until we find the two keyframes
				// that bound the current frame time.
				uint64_t next_frame_index = (current_index + 1) % frame_count_;
				const key_frame_t *next = &bone_frames[next_frame_index];
				const key_frame_t *curr = &bone_frames[current_index];
				while(true) {
					// Current and next frame bound the playhead
					if(state.current_frame >= curr->frame && state.current_frame < next->frame)
						break;

					// Wrap-around case
					if(next->frame == 0 && state.current_frame >= curr->frame)
						break;

					current_index = (current_index + 1) % frame_count_;
					next_frame_index = (current_index + 1) % frame_count_;
					next = &bone_frames[next_frame_index];
					curr = &bone_frames[current_index];
				}

				uint64_t distance;
				if(next_frame_index < current_index) {
					// Wrapped
					distance = frame_count_ - curr->frame + next->frame;
				} else {
					distance = next->frame - curr->frame;
				}

				float frac = (state.current_frame - curr->frame) / distance;
				pose[i].offset = curr->offset * (1.0f - frac) + next->offset * frac;
				pose[i].rot = quaternion_t::nlerp(frac, curr->rot, next->rot);
			}
		}
	

	} // namespace core
} // namespace maki
