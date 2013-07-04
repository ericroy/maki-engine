#include "framework/framework_stdafx.h"
#include "framework/systems/MakiPhysicsSystem.h"
#include "framework/components/MakiPhysicsComponent.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{

			PhysicsSystem::PhysicsSystem()
				: System(Component::TypeFlag_Physics|Component::TypeFlag_Transform),
				broadphase(nullptr),
				collisionConfig(nullptr),
				dispatcher(nullptr),
				solver(nullptr),
				world(nullptr)
			{
				broadphase = new btDbvtBroadphase();
				collisionConfig = new btDefaultCollisionConfiguration();
				dispatcher = new btCollisionDispatcher(collisionConfig);
				solver = new btSequentialImpulseConstraintSolver;
				world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
				world->setGravity(btVector3(0.0f, 0.0f, 0.0f));
			}

			PhysicsSystem::~PhysicsSystem()
			{
				if(world != nullptr) {
					btCollisionObjectArray &objArray = world->getCollisionObjectArray();
					for(int32 i = world->getNumCollisionObjects()-1; i >= 0; i--)
					{
						world->removeCollisionObject(objArray[i]);
					}
				}

				DeletePointers(collisionObjects);
				DeletePointers(collisionShapes);
				DeletePointers(stridingMeshInterfaces);

				SAFE_DELETE(world);
				SAFE_DELETE(solver);
				SAFE_DELETE(dispatcher);
				SAFE_DELETE(collisionConfig);
				SAFE_DELETE(broadphase);
			}

			void PhysicsSystem::Update(float dt)
			{
				world->stepSimulation(dt, 8);
			}

			void PhysicsSystem::Add(Entity *e)
			{
				Components::Physics *physicsComp = e->Get<Components::Physics>();
				Components::Transform *transComp = e->Get<Components::Transform>();

				Node n;
				n.physicsComp = physicsComp;
				n.transComp = transComp;
				nodes.push_back(n);

				btCollisionShape *shape = nullptr;
				Vector4 center(0.0f);

				if(physicsComp->objectShape == Components::Physics::ObjectShape_Box) {
					center = (physicsComp->minCorner + physicsComp->maxCorner) / 2.0f;
					Vector4 halfExtents = physicsComp->maxCorner - center;
					shape = Track(new btBoxShape(TO_BTVEC3(halfExtents)));
				} else if(physicsComp->objectShape == Components::Physics::ObjectShape_Mesh) {
					shape = MakeMeshShape(physicsComp);
				} else {
					assert(false && "Unhandled shape case");
				}

				if(physicsComp->objectType == Components::Physics::ObjectType_Dynamic) {
					// If the collision shape needs to be off-center relative to the body, then we must make the
					// collision shape a part of a compound shape which IS centered.
					if(center.LengthSquared() > 0.0000001f) {
						btCompoundShape *container = Track(new btCompoundShape());
						container->addChildShape(btTransform(TO_BTQUAT(Vector3(0.0f)), TO_BTVEC3(center)), shape);
						shape = container;
					}

					btRigidBody::btRigidBodyConstructionInfo info(physicsComp->mass, physicsComp, shape, TO_BTVEC3(physicsComp->inertia));
					btRigidBody *body = Track(new btRigidBody(info));
					world->addRigidBody(body);
				} else {
					btCollisionObject *obj = Track(new btCollisionObject());
					obj->setWorldTransform(btTransform(TO_BTQUAT(transComp->GetOrientation()), TO_BTVEC3(transComp->GetPosition())));
					obj->setCollisionShape(shape);
					world->addCollisionObject(obj);
				}
			}

			void PhysicsSystem::Remove(Entity *e)
			{
				Node n;
				n.physicsComp = e->Get<Components::Physics>();
				nodes.erase(std::find(std::begin(nodes), std::end(nodes), n));
			}

			btBvhTriangleMeshShape *PhysicsSystem::MakeMeshShape(Components::Physics *physicsComp)
			{
				assert(physicsComp->objectShape == Components::Physics::ObjectShape_Mesh && "Object shape must be 'mesh'");

				Mesh *m = MeshManager::Get(physicsComp->mesh);
				assert(m->GetIndicesPerFace() == 3);

				btIndexedMesh indexedMesh;
				indexedMesh.m_indexType = PHY_SHORT;
				if(m->GetBytesPerIndex() == 4) {
					indexedMesh.m_indexType = PHY_INTEGER;
				}
				indexedMesh.m_numTriangles = m->GetFaceCount();
				indexedMesh.m_numVertices = m->GetVertexCount();
				indexedMesh.m_triangleIndexBase = (uint8 *)m->GetIndexData();
				indexedMesh.m_triangleIndexStride = m->GetBytesPerIndex()*m->GetIndicesPerFace();
				indexedMesh.m_vertexBase = (uint8 *)m->GetVertexData();
				indexedMesh.m_vertexStride = m->GetVertexStride();
				indexedMesh.m_vertexType = PHY_FLOAT;

				btTriangleIndexVertexArray *vertexArray = Track(new btTriangleIndexVertexArray);
				vertexArray->addIndexedMesh(indexedMesh, PHY_SHORT);
				return Track(new btBvhTriangleMeshShape(vertexArray, true));
			}


		} // naemspace Systems	

	} // namespace Framework

} // namespace Maki