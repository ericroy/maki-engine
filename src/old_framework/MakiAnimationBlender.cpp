#include "framework/framework_stdafx.h"
#include "framework/MakiAnimationBlender.h"

namespace Maki
{
	namespace Framework
	{
	
		AnimationBlender::Node::Node(float rate)
			: rate(rate), boneCount(0)
		{
		}

		AnimationBlender::Node::~Node()
		{
		}

	




		AnimationBlender::Anim::Anim(float rate, bool loop, HandleOrRid animId)
			: Node(rate), loop(loop)
		{
			if(animId.isHandle) {
				SkeletonAnimationManager::AddRef(animId.handle);
				animation = animId.handle;
			} else {
				animation = CoreManagers::Get()->skeletonAnimationManager->Load(animId.rid);
			}

			boneCount = SkeletonAnimationManager::Get(animation)->GetBoneCount();
			state.SetSize(boneCount);
		}

		AnimationBlender::Anim::~Anim()
		{
			SkeletonAnimationManager::Free(animation);
		}

		void AnimationBlender::Anim::AdvanceState(float dt, Array<Skeleton::Joint> &pose, float rate)
		{
			SkeletonAnimationManager::Get(animation)->AdvanceState(dt, state, pose, loop, this->rate * rate);
		}

		void AnimationBlender::Anim::SetFrame(float frame)
		{
			float oldFrame = state.currentFrame;
			state.currentFrame = frame;
		}








		AnimationBlender::Blend::Blend(float rate, float balance, Node *first, Node *second)
			: Node(rate), balance(balance), first(first), second(second)
		{
			boneCount = first->boneCount;
			tempPose.SetSize(boneCount);
		}
			
		AnimationBlender::Blend::~Blend()
		{
			SAFE_DELETE(first);
			SAFE_DELETE(second);
		}

		void AnimationBlender::Blend::AdvanceState(float dt, Array<Skeleton::Joint> &pose, float rate)
		{
			if(balance.Get() == 0.0f) {
				first->AdvanceState(dt, pose, this->rate * rate);
			} else if(balance.Get() == 1.0f) {
				second->AdvanceState(dt, pose, this->rate * rate);
			} else {
				first->AdvanceState(dt, tempPose, this->rate * rate);
				second->AdvanceState(dt, pose, this->rate * rate);
				const float oneMinusBalance = 1.0f - balance.Get();
				for(uint32 i = 0; i < boneCount; i++) {
					pose[i].offset = pose[i].offset * balance.Get() + tempPose[i].offset * oneMinusBalance;
					pose[i].rot = Quaternion::Nlerp(balance.Get(), tempPose[i].rot, pose[i].rot);
				}
			}
		}
			






		AnimationBlender::AnimationBlender()
			: root(nullptr)
		{
		}

		AnimationBlender::~AnimationBlender()
		{
			SAFE_DELETE(root);
		}

		bool AnimationBlender::Load(Rid rid)
		{
			assert(root == nullptr);

			Document doc;
			if(!doc.Load(rid)) {
				Console::Error("Failed to parse blend tree <rid %d>", rid);
				return false;
			}
			if(doc.root->count != 1) {
				Console::Error("Blend tree must have exactly 1 root node <rid %d>", rid);
				return false;
			}
			root = LoadRecursive(doc.root->children[0]);
			return root != nullptr;
		}

		void AnimationBlender::AdvanceState(float dt, Array<Skeleton::Joint> &pose, float rate)
		{
			if(root == nullptr) {
				return;
			}
			
			// Update all tweens in the tree
			const uint32 count = tweens.size();
			for(uint32 i = 0; i < count; i++) {
				tweens[i]->Update(dt);
			}

			// Update the provided pose
			if(pose.count != root->boneCount) {
				pose.SetSize(root->boneCount);
			}
			root->AdvanceState(dt, pose, rate);
		}


		AnimationBlender::Node *AnimationBlender::LoadRecursive(Document::Node *n)
		{
			Engine *eng = Engine::Get();

			if(n->ValueEquals("blend")) {
				Node *subNodes[2] = {nullptr, nullptr};
				uint32 subNodeCount = 0;
				float rate = 1.0f;
				float balance = 0.5f;

				for(uint32 i = 0; i < n->count; i++) {
					if(n->children[i]->ValueEquals("blend") || n->children[i]->ValueEquals("anim")) {
						if(subNodeCount >= 2) {
							Console::Error("Blend node had too many subnodes");
							SAFE_DELETE(subNodes[0]);
							SAFE_DELETE(subNodes[1]);
							return nullptr;
						}
						subNodes[subNodeCount++] = LoadRecursive(n->children[i]);
					} else if(n->children[i]->ValueEquals("rate")) {
						rate = n->children[i]->children[0]->ValueAsFloat(1.0f);
					} else if(n->children[i]->ValueEquals("balance")) {
						balance = n->children[i]->children[0]->ValueAsFloat(1.0f);
					} else {
						Console::Warning("Unrecognized node in blend tree: %s", n->children[i]->value);
					}
				}
				if(subNodeCount != 2) {
					Console::Error("Blend node requires two subnodes");
					SAFE_DELETE(subNodes[0]);
					SAFE_DELETE(subNodes[1]);
					return nullptr;
				}
				Blend *b = new Blend(rate, balance, subNodes[0], subNodes[1]);
				tweens.push_back(&b->balance);
				return b;
			} else {
				float rate = n->ResolveAsFloat("rate.#0", 1.0f);
				bool loop = n->ResolveAsBool("loop.#0", false);
				Document::Node *dataPath = n->Resolve("data.#0");
				if(dataPath == nullptr) {
					Console::Error("'anim' node requires a child node called 'data'");
					return nullptr;
				}
				Rid rid = eng->assets->PathToRid(dataPath->value);
				return new Anim(rate, loop, rid);
			}
		}


	} // namespace Framework

} // namespace Maki