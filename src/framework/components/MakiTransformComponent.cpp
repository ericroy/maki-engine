#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			Transform::Transform()
				: Component(TYPE),
				position(0.0f),
				orientation(),
				matrix(true),
				world(true),
				relativeMode(false)
			{
			}

			Transform::~Transform()
			{
			}

			bool Transform::Init(Document::Node *node)
			{
				Vector4 pos(0.0f);
				Vector4 angles(0.0f);
				node->ResolveAsVectorN("pos", 3, pos.vals);
				node->ResolveAsVectorN("angles", 3, angles.vals);
				SetMatrix(pos, Quaternion(angles));

				Vector4 target(0.0f);
				if(node->ResolveAsVectorN("target", 3, target.vals)) {
					LookAt(target);
				}

				return true;
			}

		} // namespace Components

	} // namespace Framework

} // namespace Maki