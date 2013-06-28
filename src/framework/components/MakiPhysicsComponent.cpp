#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiPhysicsComponent.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			Physics::Physics()
				: Component(TYPE, DEPENDENCIES),
				transComp(nullptr)
			{
			}

			Physics::~Physics()
			{
			}

			bool Physics::Init(Document::Node *props)
			{
				/*
				PhysicsWorld *world = PhysicsWorld::Get();
				const uint32 hullCount = gScene->physicsHulls.size();
				for(uint32 i = 0; i < hullCount; i++) {
					world->AddCollisionEntity((MeshEntity *)gScene->physicsHulls[i]);
				}
				*/

				/*
				PhysicsWorld *world = PhysicsWorld::Get();
				btCollisionShape *shape = world->Track(new btCapsuleShape(radius, height-2*radius));
				btCompoundShape *container = world->Track(new btCompoundShape());
				container->addChildShape(btTransform(TO_BTQUAT(Vector3(MAKI_PI/2.0f, 0.0f, 0.0f)), btVector3(0.0f, 0.0f, height/2)), shape);
				btRigidBody::btRigidBodyConstructionInfo info(80.0f, &physicsLink, container, btVector3(0,0,0));
				body = world->Track(new btRigidBody(info));
				body->setActivationState(DISABLE_DEACTIVATION);
				body->setAngularFactor(0.0f);
				world->AddRigidBody(body);
				*/

				return true;
			}

			void Physics::OnAttach()
			{
				transComp = owner->Get<Transform>();
			}
			
			void Physics::OnDetach()
			{
				transComp = nullptr;
			}

			void Physics::getWorldTransform(btTransform &worldTransform) const
			{
				if(transComp != nullptr) {
					const Matrix44 &m = transComp->GetWorldMatrix();
					worldTransform.setFromOpenGLMatrix(m.vals);
				}
			}

			void Physics::setWorldTransform(const btTransform &worldTransform)
			{
				if(transComp != nullptr) {
					Matrix44 m;
					worldTransform.getOpenGLMatrix(m.vals);
					transComp->SetWorldMatrix(m);
				}
			}


		} // namspace Components

	} // namespace Framework

} // namespace Maki