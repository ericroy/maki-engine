#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			class Physics;
			class Transform;
		}

		namespace Systems
		{

			class PhysicsSystem : public System
			{
			private:
				struct  Node
				{
					Components::Physics *physicsComp;
					Components::Transform *transComp;

					inline bool operator==(const Node &other) const { return physicsComp == other.physicsComp; }
				};

			public:
				PhysicsSystem(uint32 messageQueueSize);
				~PhysicsSystem();
				void Update(float dt);

			private:
				void Add(Entity *e);
				void Remove(Entity *e);
				void MakeMeshShape(Components::Physics *physicsComp);

			public:
				btBroadphaseInterface *broadphase;
				btDefaultCollisionConfiguration *collisionConfig;
				btCollisionDispatcher *dispatcher;
				btSequentialImpulseConstraintSolver *solver;
				btDiscreteDynamicsWorld *world;

			private:
				std::vector<Node> nodes;
			};


		} // namespace Systems

	} // namespace Framework

} // namespace Maki