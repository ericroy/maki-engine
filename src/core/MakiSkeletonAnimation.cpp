#include "core/core_stdafx.h"
#include "core/MakiSkeletonAnimation.h"
#include "core/MakiSkeleton.h"
#include "core/MakiDocument.h"

namespace maki
{
	namespace core
	{
	
		float skeleton_animation_t::debug_rate_coeff_ = 1.0f;
	

		skeleton_animation_t::state_t::state_t(uint32 size)
			: current_frame_(0.0f)
		{
			set_size(size);
		}
	
		skeleton_animation_t::state_t::state_t()
			: current_frame_(0.0f)
		{
		}
	
		void skeleton_animation_t::state_t::set_size(uint32 size)
		{
			current_key_frames_.set_size(size);
			current_key_frames_.zero();
		}






		skeleton_animation_t::skeleton_animation_t()
			: resource_t(),
			frameCount(0),
			frameRate(0.0f)
		{
		}

		skeleton_animation_t::~skeleton_animation_t()
		{
			for(uint32 i = 0; i < data.count_; i++) {
				data[i].free();
			}
		}

		bool skeleton_animation_t::load(rid_t rid)
		{
			if(data.data_ != nullptr) {
				data.free();
			}
			frameCount = 0;
			frameRate = 0.0f;

			document_t doc;
			if(!doc.load(rid)) {
				console_t::error("Could not parse file as document <rid %d>", rid);
				return false;
			}

			uint32 count;
			if(!doc.root->resolve_as_uint("bone_count.#0", &count)) {
				console_t::error("Could not find bone count in animation document");
				return false;
			}
			data.set_size(count);
			data.zero();

			if(!doc.root->resolve_as_float("frame_rate.#0", &frameRate)) {
				console_t::error("Could not find frame rate in animation document");
				return false;
			}
			if(!doc.root->resolve_as_uint("frame_count.#0", &frameCount)) {
				console_t::error("Could not find frame count in animation document");
				return false;
			}
		
			for(uint32 i = 3; i < doc.root->count; i++) {
				document_t::node_t *boneNode = doc.root->children[i];
				uint32 bone = i-3;

				data[bone].set_size(boneNode->count);
				data[bone].zero();

				for(uint32 j = 0; j < boneNode->count; j++) {
					document_t::node_t *n = boneNode->children[j];
					key_frame_t &kf = data[bone][j];

					kf.frame_ = n->value_as_uint();
					kf.offset_ = vector3_t(n->children[0]->value_as_float(), n->children[1]->value_as_float(), n->children[2]->value_as_float());

					vector3_t euler_angles = vector3_t(n->children[3]->value_as_float(), n->children[4]->value_as_float(), n->children[5]->value_as_float()) * MAKI_DEG_TO_RAD;
					kf.rot_.from_euler_angles(euler_angles);
				}
			}

			this->rid_ = rid;
			return true;
		}

		void skeleton_animation_t::AdvanceState(float timeDelta, state_t &state, array_t<skeleton_t::joint_t> &pose, bool loop, float rateCoeff)
		{
			state.current_frame_ += timeDelta * rateCoeff * debug_rate_coeff_ * frameRate;
			if(state.current_frame_ >= frameCount) {
				if(loop) {
					state.current_frame_ = state.current_frame_ - ((uint32)state.current_frame_ / frameCount)*frameCount;
				} else {
					state.current_frame_ = (float)frameCount;
				}
			}

			for(uint32 i = 0; i < data.count_; i++) {
				uint32 &currentIndex = state.current_key_frames_[i];
				const array_t<key_frame_t> &boneFrames = data[i];

				if(boneFrames.count_ == 0) {
					continue;
				}
				if(boneFrames.count_ == 1) {
					pose[i].offset_ = boneFrames[0].offset_;
					pose[i].rot_ = boneFrames[0].rot_;
					continue;
				}
			
				// Advance through the frames for this bone until we find the two keyframes
				// that bound the current frame time.
				uint32 nextFrameIndex = (currentIndex + 1) % frameCount;
				const key_frame_t *next = &boneFrames[nextFrameIndex];
				const key_frame_t *curr = &boneFrames[currentIndex];
				while(true) {
					// Current and next frame bound the playhead
					if(state.current_frame_ >= curr->frame && state.current_frame_ < next->frame) {
						break;
					}

					// Wrap-around case
					if(next->frame == 0 && state.current_frame_ >= curr->frame) {
						break;
					}

					currentIndex = (currentIndex + 1) % frameCount;
					nextFrameIndex = (currentIndex + 1) % frameCount;
					next = &boneFrames[nextFrameIndex];
					curr = &boneFrames[currentIndex];
				}

				uint32 distance;
				if(nextFrameIndex < currentIndex) {
					// Wrapped
					distance = frameCount - curr->frame + next->frame;
				} else {
					distance = next->frame - curr->frame;
				}

				float frac = (state.current_frame_ - curr->frame) / distance;
				pose[i].offset_ = curr->offset * (1.0f - frac) + next->offset * frac;
				pose[i].rot_ = quaternion_t::nlerp(frac, curr->rot, next->rot);
			}
		}
	

	} // namespace core

} // namespace maki
