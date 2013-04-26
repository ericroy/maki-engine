#include "framework/framework_stdafx.h"
#include "framework/MakiPhysicsLink.h"

namespace Maki
{
	PhysicsLink::PhysicsLink(Entity *entity)
		: entity(entity)
	{
	}

	PhysicsLink::~PhysicsLink()
	{
	}
	
	void PhysicsLink::getWorldTransform(btTransform &worldTransform) const
	{
		worldTransform.setFromOpenGLMatrix(entity->GetWorldMatrix().vals);
	}
	
	void PhysicsLink::setWorldTransform(const btTransform &worldTransform)
	{
		Matrix44 m;
		worldTransform.getOpenGLMatrix(m.vals);
		entity->SetWorldMatrix(m);
	}

} // namespace Maki