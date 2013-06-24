#include "core/core_stdafx.h"
#include "core/MakiSkeletonAnimation.h"
#include "core/MakiSkeleton.h"
#include "core/MakiDocument.h"

namespace Maki
{
	namespace Core
	{
	
		float SkeletonAnimation::debugRateCoeff = 1.0f;
	

		SkeletonAnimation::State::State(uint32 size)
			: currentFrame(0.0f)
		{
			SetSize(size);
		}
	
		SkeletonAnimation::State::State()
			: currentFrame(0.0f)
		{
		}
	
		void SkeletonAnimation::State::SetSize(uint32 size)
		{
			currentKeyFrames.SetSize(size);
			currentKeyFrames.Zero();
		}






		SkeletonAnimation::SkeletonAnimation()
			: Resource(),
			frameCount(0),
			frameRate(0.0f)
		{
		}

		SkeletonAnimation::~SkeletonAnimation()
		{
			for(uint32 i = 0; i < data.count; i++) {
				data[i].Delete();
			}
		}

		bool SkeletonAnimation::Load(Rid rid)
		{
			if(data.data != nullptr) {
				data.Delete();
			}
			frameCount = 0;
			frameRate = 0.0f;

			Document doc;
			if(!doc.Load(rid)) {
				Console::Error("Could not parse file as document <rid %d>", rid);
				return false;
			}

			uint32 count;
			if(!doc.root->ResolveAsUInt("bone_count.#0", &count)) {
				Console::Error("Could not find bone count in animation document");
				return false;
			}
			data.SetSize(count);
			data.Zero();

			if(!doc.root->ResolveAsFloat("frame_rate.#0", &frameRate)) {
				Console::Error("Could not find frame rate in animation document");
				return false;
			}
			if(!doc.root->ResolveAsUInt("frame_count.#0", &frameCount)) {
				Console::Error("Could not find frame count in animation document");
				return false;
			}
		
			for(uint32 i = 3; i < doc.root->count; i++) {
				Document::Node *boneNode = doc.root->children[i];
				uint32 bone = i-3;

				data[bone].SetSize(boneNode->count);
				data[bone].Zero();

				for(uint32 j = 0; j < boneNode->count; j++) {
					Document::Node *n = boneNode->children[j];
					KeyFrame &kf = data[bone][j];

					kf.frame = n->ValueAsUInt();
					kf.offset = Vector3(n->children[0]->ValueAsFloat(), n->children[1]->ValueAsFloat(), n->children[2]->ValueAsFloat());

					Vector3 eulerAngles = Vector3(n->children[3]->ValueAsFloat(), n->children[4]->ValueAsFloat(), n->children[5]->ValueAsFloat()) * MAKI_DEG_TO_RAD;
					kf.rot.FromEulerAngles(eulerAngles);
				}
			}

			this->rid = rid;
			return true;
		}

		void SkeletonAnimation::AdvanceState(float timeDelta, float rateCoeff, bool loop, State &state, Array<Skeleton::Joint> &pose)
		{
			state.currentFrame += timeDelta * rateCoeff * debugRateCoeff * frameRate;
			if(state.currentFrame >= frameCount) {
				if(loop) {
					state.currentFrame = state.currentFrame - ((uint32)state.currentFrame / frameCount)*frameCount;
				} else {
					state.currentFrame = (float)frameCount;
				}
			}

			for(uint32 i = 0; i < data.count; i++) {
				uint32 &currentIndex = state.currentKeyFrames[i];
				const Array<KeyFrame> &boneFrames = data[i];

				if(boneFrames.count == 0) {
					continue;
				}
				if(boneFrames.count == 1) {
					pose[i].offset = boneFrames[0].offset;
					pose[i].rot = boneFrames[0].rot;
					continue;
				}
			
				// Advance through the frames for this bone until we find the two keyframes
				// that bound the current frame time.
				uint32 nextFrameIndex = (currentIndex + 1) % frameCount;
				const KeyFrame *next = &boneFrames[nextFrameIndex];
				const KeyFrame *curr = &boneFrames[currentIndex];
				while(true) {
					// Current and next frame bound the playhead
					if(state.currentFrame >= curr->frame && state.currentFrame < next->frame) {
						break;
					}

					// Wrap-around case
					if(next->frame == 0 && state.currentFrame >= curr->frame) {
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

				float frac = (state.currentFrame - curr->frame) / distance;
				pose[i].offset = curr->offset * (1.0f - frac) + next->offset * frac;
				pose[i].rot = Quaternion::Nlerp(frac, curr->rot, next->rot);
			}
		}
	

	} // namespace Core

} // namespace Maki