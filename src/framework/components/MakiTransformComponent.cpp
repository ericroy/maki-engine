#include "framework/framework_stdafx.h"
#include "framework/components/MakiTransformComponent.h"
#include "framework/MakiComponentPool.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			Transform::Transform()
				: Component(TYPE, DEPENDENCIES),
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

			bool Transform::Init(const Vector4 &pos, const Vector4 &eulerAngles)
			{
				SetMatrix(pos, Quaternion(eulerAngles));
				return true;
			}

			Transform *Transform::Clone(bool prototype)
			{
				Transform *c = ComponentPool<Transform>::Get()->Create();
				c->world = world;
				c->matrix = matrix;
				c->position = position;
				c->orientation = orientation;
				c->relativeMode = relativeMode;
				return c;
			}

		} // namespace Components

	} // namespace Framework

} // namespace Maki