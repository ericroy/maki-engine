#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	TransformComponent::TransformComponent()
		: Component(COMPONENT_TYPE),
		position(0.0f),
		orientation(),
		matrix(true),
		world(true),
		useWorldMatrix(false)
	{
	}

	TransformComponent::~TransformComponent()
	{
	}

	bool TransformComponent::Init(Document::Node *node)
	{
		Vector4 pos(0.0f);
		Vector4 angles(0.0f);

		node->ResolveAsVectorN("pos", 3, pos.vals);
		node->ResolveAsVectorN("angles", 3, angles.vals);
		
		SetMatrix(pos, Quaternion(angles));

		return true;
	}


}