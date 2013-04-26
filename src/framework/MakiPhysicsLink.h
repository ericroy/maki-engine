#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{
	class Entity;

	class PhysicsLink : public btMotionState
	{
	public:
		PhysicsLink(Entity *entity);
		virtual ~PhysicsLink();
		virtual void getWorldTransform(btTransform &worldTransform) const;
		virtual void setWorldTransform(const btTransform &worldTransform);

	public:
		Entity *entity;
	};

} // namespace Maki