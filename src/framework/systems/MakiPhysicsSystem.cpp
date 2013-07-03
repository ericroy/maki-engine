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

			void PhysicsSystem::Node::getWorldTransform(btTransform &worldTransform) const
			{
				if(transComp != nullptr) {
					const Matrix44 &m = transComp->GetMatrix();
					worldTransform.setFromOpenGLMatrix(m.vals);
				}
			}

			void PhysicsSystem::Node::setWorldTransform(const btTransform &worldTransform)
			{
				if(transComp != nullptr) {
					Matrix44 m;
					worldTransform.getOpenGLMatrix(m.vals);
					transComp->SetMatrix(m);
				}
			}



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
				Node n;
				n.physicsComp = e->Get<Components::Physics>();
				n.transComp = e->Get<Components::Transform>();
				nodes.push_back(n);

				Node &node = nodes.back();


				if(node.physicsComp->objectType == Components::Physics::ObjectType_Static) {
					AddStatic(node);
				} else if(node.physicsComp->objectType == Components::Physics::ObjectType_Dynamic) {
					AddDynamic(node);
				} else {
					assert(false && "Unhandled case");
				}
			}

			void PhysicsSystem::Remove(Entity *e)
			{
				Node n;
				n.physicsComp = e->Get<Components::Physics>();
				n.transComp = e->Get<Components::Transform>();
				nodes.erase(std::find(std::begin(nodes), std::end(nodes), n));
			}


			void PhysicsSystem::AddStatic(Node &n)
			{
				if(n.physicsComp->objectShape == Components::Physics::ObjectShape_Box) {
					assert(false && "case not handled");
				} else if(n.physicsComp->objectShape == Components::Physics::ObjectShape_Mesh) {
					assert(false && "case not handled");
				}
			}

			void PhysicsSystem::AddDynamic(Node &n)
			{
				btCollisionShape *shape = nullptr;
				Vector4 center(0.0f);

				if(n.physicsComp->objectShape == Components::Physics::ObjectShape_Box) {
					center = (n.physicsComp->minCorner + n.physicsComp->maxCorner) / 2.0f;
					Vector4 halfExtents = n.physicsComp->maxCorner - center;
					shape = Track(new btBoxShape(TO_BTVEC3(halfExtents)));
				} else if(n.physicsComp->objectShape == Components::Physics::ObjectShape_Mesh) {
					assert(false && "case not handled");
				}

				// If the body needs to be off-center, then we must make the collision shape a part of a 
				// compound shape which is centered at the origin
				if(center.LengthSquared() > 0.0000001f) {
					btCompoundShape *container = Track(new btCompoundShape());
					container->addChildShape(btTransform(TO_BTQUAT(Vector3(0.0f)), TO_BTVEC3(center)), shape);
					shape = container;
				}

				btRigidBody::btRigidBodyConstructionInfo info(80.0f, &n, shape, btVector3(0,0,0));
				btRigidBody *body = Track(new btRigidBody(info));
				world->addRigidBody(body);
			}

			//void PhysicsSystem::AddRigidBody(Node &n)
			//{
			//	// TODO: Create the right type of collision shape based on the properties of the physics component
			//	const float radius = 0.4f;
			//	const float height = 1.8f;
			//	btCollisionShape *shape = Track(new btCapsuleShape(radius, height-2*radius));

			//	btCompoundShape *container = Track(new btCompoundShape());
			//	container->addChildShape(btTransform(TO_BTQUAT(Vector3(MAKI_PI/2.0f, 0.0f, 0.0f)), btVector3(0.0f, 0.0f, height/2)), shape);

			//	btRigidBody::btRigidBodyConstructionInfo info(80.0f, &n, container, btVector3(0,0,0));
			//	btRigidBody *body = Track(new btRigidBody(info));
			//	body->setActivationState(DISABLE_DEACTIVATION);
			//	body->setAngularFactor(0.0f);
			//	world->addRigidBody(body);
			//}

			//void PhysicsSystem::AddCollisionMesh(Node &n)
			//{
			//	Mesh *m = MeshManager::Get(n.physicsComp->mesh);
			//	assert(m->GetIndicesPerFace() == 3);

			//	btIndexedMesh indexedMesh;
			//	indexedMesh.m_indexType = PHY_SHORT;
			//	if(m->GetBytesPerIndex() == 4) {
			//		indexedMesh.m_indexType = PHY_INTEGER;
			//	}
			//	indexedMesh.m_numTriangles = m->GetFaceCount();
			//	indexedMesh.m_numVertices = m->GetVertexCount();
			//	indexedMesh.m_triangleIndexBase = (uint8 *)m->GetIndexData();
			//	indexedMesh.m_triangleIndexStride = m->GetBytesPerIndex()*m->GetIndicesPerFace();
			//	indexedMesh.m_vertexBase = (uint8 *)m->GetVertexData();
			//	indexedMesh.m_vertexStride = m->GetVertexStride();
			//	indexedMesh.m_vertexType = PHY_FLOAT;

			//	btTriangleIndexVertexArray *vertexArray = Track(new btTriangleIndexVertexArray);
			//	vertexArray->addIndexedMesh(indexedMesh, PHY_SHORT);
			//	btBvhTriangleMeshShape *meshShape = Track(new btBvhTriangleMeshShape(vertexArray, true));

			//	btCollisionObject *obj = Track(new btCollisionObject());
			//	obj->setWorldTransform(btTransform(TO_BTQUAT(n.transComp->GetOrientation()), TO_BTVEC3(n.transComp->GetPosition())));
			//	obj->setCollisionShape(meshShape);
			//	world->addCollisionObject(obj);
			//}

		} // naemspace Systems	

	} // namespace Framework

} // namespace Maki