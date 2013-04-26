#include "framework/framework_stdafx.h"
#include "MakiPhysicsWorld.h"
#include "framework/MakiMeshEntity.h"
#include "framework/MakiDebugPhysicsRenderer.h"

namespace Maki
{

	PhysicsWorld::PhysicsWorld(Rid debugRenderMaterialRid)
		: PseudoSingleton<PhysicsWorld>(),
		broadphase(nullptr),
		collisionConfig(nullptr),
		dispatcher(nullptr),
		solver(nullptr),
		world(nullptr),
		debugRenderer(nullptr)
	{
#if _DEBUG
		debugRenderer = new DebugPhysicsRenderer(debugRenderMaterialRid);
#endif
		Create();
	}

	PhysicsWorld::~PhysicsWorld()
	{
		Destroy();
		SAFE_DELETE(debugRenderer);
	}

	void PhysicsWorld::Update(float dt)
	{
		world->stepSimulation(dt, 8);
	}

	void PhysicsWorld::Draw()
	{
#if _DEBUG
		debugRenderer->Draw(world);
#endif
	}

	int32 PhysicsWorld::GetDebugMode() const
	{
#if _DEBUG
		return debugRenderer->getDebugMode();
#endif
		return 0;
	}

	void PhysicsWorld::SetDebugMode(int32 mode)
	{
#if _DEBUG
		debugRenderer->setDebugMode(mode);
#endif
	}

	void PhysicsWorld::AddRigidBody(btRigidBody *body)
	{
		world->addRigidBody(body);
	}

	void PhysicsWorld::AddCollisionEntity(MeshEntity *me)
	{
		Mesh *m = MeshManager::Get(me->mesh);
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
		btBvhTriangleMeshShape *meshShape = Track(new btBvhTriangleMeshShape(vertexArray, true));

		btCollisionObject *obj = Track(new btCollisionObject());
		obj->setWorldTransform(btTransform(TO_BTQUAT(me->GetOrientation()), TO_BTVEC3(me->GetPosition())));
		obj->setCollisionShape(meshShape);
		world->addCollisionObject(obj);
	}

	void PhysicsWorld::Create()
	{
		broadphase = new btDbvtBroadphase();
		collisionConfig = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(collisionConfig);
		solver = new btSequentialImpulseConstraintSolver;
		world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
		world->setGravity(btVector3(0.0f, 0.0f, -9.81f));
	}

	void PhysicsWorld::Destroy()
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

	void PhysicsWorld::Reset()
	{
		Console::Info("Resetting physics world");
		Destroy();
		Create();
	}
	

} // namespace Maki