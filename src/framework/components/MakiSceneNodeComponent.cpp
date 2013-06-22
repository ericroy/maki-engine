#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiSceneNodeComponent.h"

namespace Maki
{
	SceneNodeComponent::SceneNodeComponent()
		: Component(COMPONENT_TYPE),
		position(0.0f),
		orientation(),
		matrix(true),
		world(true),
		parent(nullptr)
	{
		messageHandler = [&](Component *sender, Message message, uintptr_t arg1, uintptr_t arg2) {
			switch(message) {
			case Message_MeshBoundsChanged:
				BoundingBox *meshBox = reinterpret_cast<BoundingBox *>(arg1);
				float meshScale = *reinterpret_cast<float *>(arg2);
				bounds.Reset();
				bounds.Merge(BoundingBox(meshBox->pos, meshBox->radii * meshScale));
				break;

			case Message_DebugWidgetCreated:
				Entity *e = reinterpret_cast<Entity *>(arg1);
				AddChild(e);
				return true;
			}

			return false;
		};
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
		Vector4 pos(0.0f);
		Vector4 angles(0.0f);
		Vector4 target(0.0f);

		node->ResolveAsVectorN("pos", 3, pos.vals);
		
		if(node->ResolveAsVectorN("angles", 3, angles.vals)) {	
			SetMatrix(pos, Quaternion(angles));
		}

		if(node->ResolveAsVectorN("target", 3, target.vals)) {
			Matrix44 lookAt;
			Matrix44::LookAt(GetPosition(), target, Vector4::UnitZ, lookAt);
			Matrix44::Inverse(lookAt, lookAt);
			SetWorldMatrix(lookAt);
		}
		
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