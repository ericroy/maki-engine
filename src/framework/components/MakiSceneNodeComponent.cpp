#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiSceneNodeComponent.h"

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

			void SceneNode::RemoveChild(Entity *e)
			{
				auto iter = std::find(std::begin(children), std::end(children), e);
				if(iter != children.end()) {
					children.erase(iter);
					e->Get<SceneNode>()->parent = nullptr;
				}
			}

		} // namespace Components

	} // namespace Framework

} // namesspace Maki