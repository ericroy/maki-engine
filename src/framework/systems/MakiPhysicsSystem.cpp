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

			PhysicsSystem::PhysicsSystem(uint32 messageQueueSize)
				: SystemBase(Component::TypeFlag_Physics|Component::TypeFlag_Transform, 0, messageQueueSize, "PhysicsSystem"),
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
					int32 collisionObjectCount = world->getNumCollisionObjects();
					if(collisionObjectCount > 0) {
						Console::Warning("Warning, %d CollisionObjects were left in the physics world!", collisionObjectCount);
					}

					btCollisionObjectArray &objArray = world->getCollisionObjectArray();
					for(int32 i = collisionObjectCount-1; i >= 0; i--)
					{
						world->removeCollisionObject(objArray[i]);
					}
				}

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

				Vector4 center(0.0f);

				if(physicsComp->objectShape == Components::Physics::ObjectShape_Box) {
					center = (physicsComp->minCorner + physicsComp->maxCorner) / 2.0f;
					Vector4 halfExtents = physicsComp->maxCorner - center;
					physicsComp->shape = new btBoxShape(TO_BTVEC3(halfExtents));
				} else if(physicsComp->objectShape == Components::Physics::ObjectShape_Mesh) {
					MakeMeshShape(physicsComp);
				} else {
					assert(false && "Unhandled shape case");
				}

				if(physicsComp->objectType == Components::Physics::ObjectType_Dynamic) {
					// If the collision shape needs to be off-center relative to the body, then we must make the
					// collision shape a part of a compound shape which IS centered.
					if(center.LengthSquared() > 0.0000001f) {
						assert(physicsComp->subShape == nullptr);
						physicsComp->subShape = physicsComp->shape;
						physicsComp->shape = new btCompoundShape();
						((btCompoundShape *)physicsComp->shape)->addChildShape(btTransform(TO_BTQUAT(Vector3(0.0f)), TO_BTVEC3(center)), physicsComp->subShape);
					}

					btRigidBody::btRigidBodyConstructionInfo info(physicsComp->mass, physicsComp, physicsComp->shape, TO_BTVEC3(physicsComp->inertia));
					assert(physicsComp->body == nullptr);
					physicsComp->body = new btRigidBody(info);
					physicsComp->body->setDamping(0.2f, 0.2f);
					physicsComp->body->setSleepingThresholds(0.02f, 0.02f);
					world->addRigidBody(physicsComp->body);
				} else {
					assert(physicsComp->collisionObject == nullptr);
					physicsComp->collisionObject = new btCollisionObject();
					physicsComp->collisionObject->setWorldTransform(btTransform(TO_BTQUAT(transComp->GetOrientation()), TO_BTVEC3(transComp->GetPosition())));
					physicsComp->collisionObject->setCollisionShape(physicsComp->shape);
					world->addCollisionObject(physicsComp->collisionObject);
				}
			}

			void PhysicsSystem::Remove(Entity *e)
			{
				Node n;
				n.physicsComp = e->Get<Components::Physics>();
				std::vector<Node>::iterator iter = std::find(std::begin(nodes), std::end(nodes), n);
				
				if(n.physicsComp->objectType == Components::Physics::ObjectType_Static) {
					world->removeCollisionObject(n.physicsComp->collisionObject);
				} else {
					world->removeRigidBody(n.physicsComp->body);
				}
				
				nodes.erase(iter);
			}

			void PhysicsSystem::MakeMeshShape(Components::Physics *physicsComp)
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

				assert(physicsComp->vertexArray == nullptr);
				physicsComp->vertexArray = new btTriangleIndexVertexArray;
				physicsComp->vertexArray->addIndexedMesh(indexedMesh, PHY_SHORT);
				
				assert(physicsComp->shape == nullptr);
				physicsComp->shape = new btBvhTriangleMeshShape(physicsComp->vertexArray, true);
			}


		} // naemspace Systems	

	} // namespace Framework

} // namespace Maki