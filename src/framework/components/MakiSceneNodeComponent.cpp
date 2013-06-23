#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiSceneNodeComponent.h"

namespace Maki
{
	SceneNodeComponent::SceneNodeComponent()
		: Component(COMPONENT_TYPE),
		parent(nullptr)
	{
	}

	SceneNodeComponent::~SceneNodeComponent()
	{
		const int32 childCount = children.size();
		for(int32 i = 0; i < childCount; i++) {
			SAFE_DELETE(children[i]);
		}
	}

	bool SceneNodeComponent::Init(Document::Node *node)
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

	void SceneNodeComponent::RemoveChild(Entity *e)
	{
		const int32 size = children.size();
		for(int32 i = 0; i < size; i++) {
			if(children[i] == e) {
				e->Get<SceneNodeComponent>()->parent = nullptr;
				children.erase(children.begin()+i);
				return;
			}
		}
	}

}