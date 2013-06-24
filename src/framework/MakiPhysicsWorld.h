#pragma once
#include "framework/framework_stdafx.h"
#include "core/MakiPseudoSingleton.h"

namespace Maki
{
	namespace Framework
	{
	
		class Entity;
		class DebugPhysicsRenderer;

		class PhysicsWorld : public Core::PseudoSingleton<PhysicsWorld>
		{
		public:
			PhysicsWorld(Rid debugRenderMaterialRid);
			~PhysicsWorld();

			template<class T> inline T Track(T p) { static_assert(false, "PhysicsWorld can't track this data type, add a specialization"); return nullptr; }
			template<> inline btCollisionObject *Track<btCollisionObject *>(btCollisionObject *p) { collisionObjects.push_back(p); return p; }
			template<> inline btRigidBody *Track<btRigidBody *>(btRigidBody *p) { collisionObjects.push_back(p); return p; }
			template<> inline btCollisionShape *Track<btCollisionShape *>(btCollisionShape *p) { collisionShapes.push_back(p); return p; }
			template<> inline btCompoundShape *Track<btCompoundShape *>(btCompoundShape *p) { collisionShapes.push_back(p); return p; }
			template<> inline btCapsuleShape *Track<btCapsuleShape *>(btCapsuleShape *p) { collisionShapes.push_back(p); return p; }
			template<> inline btBvhTriangleMeshShape *Track<btBvhTriangleMeshShape *>(btBvhTriangleMeshShape *p) { collisionShapes.push_back(p); return p; }
			template<> inline btTriangleIndexVertexArray *Track<btTriangleIndexVertexArray *>(btTriangleIndexVertexArray *p) { stridingMeshInterfaces.push_back(p); return p; }

			void Update(float dt);
			void Draw();
			void AddCollisionEntity(Entity *e);
			void AddRigidBody(btRigidBody *body);
			void Reset();
			int32 GetDebugMode() const;
			void SetDebugMode(int32 mode);


		private:
			void Create();
			void Destroy();
			template<class T> inline void DeletePointers(std::vector<T> &pointers);

		public:
			btBroadphaseInterface *broadphase;
			btDefaultCollisionConfiguration *collisionConfig;
			btCollisionDispatcher *dispatcher;
			btSequentialImpulseConstraintSolver *solver;
			btDiscreteDynamicsWorld *world;
			DebugPhysicsRenderer *debugRenderer;

		private:
			std::vector<btCollisionObject *> collisionObjects;
			std::vector<btCollisionShape *> collisionShapes;
			std::vector<btStridingMeshInterface *> stridingMeshInterfaces;
		};



		template<class T>
		inline void PhysicsWorld::DeletePointers(std::vector<T> &pointers)
		{
			const int32 count = pointers.size();
			for(int32 i = count-1; i >= 0; i--) {
				SAFE_DELETE(pointers[i]);
			}
			pointers.clear();
		}

	} // namespace Framework

} // namespace Maki