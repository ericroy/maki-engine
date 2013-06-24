#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiSceneNodeComponent.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			SceneNode::SceneNode()
				: Component(TYPE),
				parent(nullptr)
			{
			}

			SceneNode::~SceneNode()
			{
				const int32 childCount = children.size();
				for(int32 i = 0; i < childCount; i++) {
					SAFE_DELETE(children[i]);
				}
			}

			bool SceneNode::Init(Document::Node *props)
			{
				Vector4 target(0.0f);

				/*if(node->ResolveAsVectorN("target", 3, target.vals)) {
					Matrix44 lookAt;
					Matrix44::LookAt(GetPosition(), target, Vector4::UnitZ, lookAt);
					Matrix44::Inverse(lookAt, lookAt);
					SetWorldMatrix(lookAt);
				}*/
		
				return true;
			}

			void SceneNode::AddChild(Entity *e)
			{
				children.push_back(e);
				e->Get<SceneNode>()->parent = owner;

				// Now that this entity is a part of a scene graph, it must be positioned in a relative manner
				e->Get<Transform>()->SetTransformRelative(true);
			}

			void SceneNode::RemoveChild(Entity *e)
			{
				auto iter = std::find(std::begin(children), std::end(children), e);
				if(iter != children.end()) {
					children.erase(iter);
					e->Get<SceneNode>()->parent = nullptr;
				}
			}

			void SceneNode::SetWorldMatrix(const Matrix44 &world)
			{
				Entity *parent = owner->Get<SceneNode>()->parent;
				if(parent == nullptr) {
					// There is no parent - we are the root of the scene graph, therefore, our relative matrix IS our world matrix.
					owner->Get<Transform>()->SetMatrix(world);
				} else {
					Matrix44 parentWorldInverse;
					Matrix44::AffineInverse(parent->Get<Transform>()->GetWorldMatrix(), parentWorldInverse);

					Transform *transComp = owner->Get<Transform>();
					transComp->SetMatrix(parentWorldInverse * transComp->GetMatrix());
				}
			}

		} // namespace Components

	} // namespace Framework

} // namesspace Maki