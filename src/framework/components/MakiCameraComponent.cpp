#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiCameraComponent.h"

namespace Maki
{

	std::function<Entity *()> CameraComponent::CreateDebugWidget = nullptr;


	CameraComponent::CameraComponent()
		: Component(COMPONENT_TYPE), frustum(0.0f, 0.0f, 0.0f, 1.0f, 50.0f)
	{
	}
		
	CameraComponent::~CameraComponent()
	{
	}

	bool CameraComponent::Init(Document::Node *node)
	{
		Vector4 target(0.0f);
		if(node->ResolveAsVectorN("target", 3, target.vals)) {
			Matrix44 lookAt;
			Matrix44::LookAt(owner->GetPosition(), target, Vector4::UnitZ, lookAt);
			Matrix44::Inverse(lookAt, lookAt);
			owner->SetWorldMatrix(lookAt);
		}

		Document::Node *frustumNode = node->Resolve("frustum");
		if(frustumNode != nullptr) {
			frustum.Set(
				1.0f, 1.0f,
				frustumNode->children[1]->ValueAsFloat(),
				frustumNode->children[2]->ValueAsFloat(),
				frustumNode->children[0]->ValueAsFloat()
			);
		}
		return true;
	}

	void CameraComponent::Attach(Entity *owner)
	{
		Component::Attach(owner);
		owner->SetFlag(Entity::Flag_Draw, false);
	}
	
}