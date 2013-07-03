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
				struct Node : public btMotionState
				{
					Node() {}
					virtual ~Node() {}

					Components::Physics *physicsComp;
					Components::Transform *transComp;

					inline bool operator==(const Node &other) const { return physicsComp == other.physicsComp; }

					// btMotionState interface
					void getWorldTransform(btTransform &worldTransform) const;
					void setWorldTransform(const btTransform &worldTransform);
				};

			public:
				PhysicsSystem();
				~PhysicsSystem();

				void Update(float dt);

				void Draw();
				int32 GetDebugMode() const;
				void SetDebugMode(int32 mode);

				template<class T> inline T Track(T p) { static_assert(false, "PhysicsWorld can't track this data type, add a specialization"); return nullptr; }
				template<> inline btCollisionObject *Track<btCollisionObject *>(btCollisionObject *p) { collisionObjects.push_back(p); return p; }
				template<> inline btRigidBody *Track<btRigidBody *>(btRigidBody *p) { collisionObjects.push_back(p); return p; }
				template<> inline btCollisionShape *Track<btCollisionShape *>(btCollisionShape *p) { collisionShapes.push_back(p); return p; }
				template<> inline btCompoundShape *Track<btCompoundShape *>(btCompoundShape *p) { collisionShapes.push_back(p); return p; }
				template<> inline btCapsuleShape *Track<btCapsuleShape *>(btCapsuleShape *p) { collisionShapes.push_back(p); return p; }
				template<> inline btBvhTriangleMeshShape *Track<btBvhTriangleMeshShape *>(btBvhTriangleMeshShape *p) { collisionShapes.push_back(p); return p; }
				template<> inline btTriangleIndexVertexArray *Track<btTriangleIndexVertexArray *>(btTriangleIndexVertexArray *p) { stridingMeshInterfaces.push_back(p); return p; }

			private:
				void Add(Entity *e);
				void Remove(Entity *e);
				void AddRigidBody(btRigidBody *body);
				void AddCollisionMesh(Node &n);
				void AddRigidBody(Node &n);
				template<class T> inline void DeletePointers(std::vector<T> &pointers);

			public:
				btBroadphaseInterface *broadphase;
				btDefaultCollisionConfiguration *collisionConfig;
				btCollisionDispatcher *dispatcher;
				btSequentialImpulseConstraintSolver *solver;
				btDiscreteDynamicsWorld *world;

			private:
				std::vector<btCollisionObject *> collisionObjects;
				std::vector<btCollisionShape *> collisionShapes;
				std::vector<btStridingMeshInterface *> stridingMeshInterfaces;
				std::vector<Node> nodes;
			};



			template<class T>
			inline void PhysicsSystem::DeletePointers(std::vector<T> &pointers)
			{
				const int32 count = pointers.size();
				for(int32 i = count-1; i >= 0; i--) {
					SAFE_DELETE(pointers[i]);
				}
				pointers.clear();
			}

		} // namespace Systems

	} // namespace Framework

} // namespace Maki