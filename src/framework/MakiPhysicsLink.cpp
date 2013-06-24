#include "framework/framework_stdafx.h"
#include "framework/MakiPhysicsLink.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
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
			/*TransformComponent *transComp = entity->Get<TransformComponent>();
			const Matrix44 &m = transComp->useWorldMatrix ? transComp->world : transComp->GetMatrix();
			worldTransform.setFromOpenGLMatrix(m.vals);*/
		}
	
		void PhysicsLink::setWorldTransform(const btTransform &worldTransform)
		{
			/*Matrix44 m;
			worldTransform.getOpenGLMatrix(m.vals);


			entity->SetWorldMatrix(m);*/
		}

	} // namespace Framework

} // namespace Maki