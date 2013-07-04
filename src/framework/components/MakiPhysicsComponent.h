#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

//#include "btBulletCollisionCommon.h"
//#include "btBulletDynamicsCommon.h"


namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			class Transform;

			class Physics : public Component, public btMotionState
			{
			public:
				enum ObjectShape
				{
					ObjectShape_Mesh = 0,
					ObjectShape_Box
				};

				enum ObjectType
				{
					ObjectType_Static = 0,
					ObjectType_Dynamic
				};

			public:
				static const Type TYPE = Type_Physics;
				static const TypeFlag DEPENDENCIES = Type_Transform;

			public:
				Physics();
				virtual ~Physics();

				// Component interface
				bool Init(Document::Node *props);
				bool InitMeshShape(ObjectType type, HandleOrRid meshId, float mass, const Vector3 &inertia);
				bool InitBoxShape(ObjectType type, const Vector4 &minCorner, const Vector4 &maxCorner, float mass, const Vector3 &inertia);

				void OnAttach();
				void OnDetach();

				// btMotionState interface
				void getWorldTransform(btTransform &worldTransform) const;
				void setWorldTransform(const btTransform &worldTransform);

			public:
				ObjectType objectType;
				ObjectShape objectShape;
				float mass;
				Vector3 inertia;
				
				union
				{
					Handle mesh;
					struct {
						Vector4 minCorner;
						Vector4 maxCorner;
					};
				};


				btCollisionShape *shape;
				union
				{
					// For mesh objects
					btTriangleIndexVertexArray *vertexArray;

					// For shape objects.  This may be null if the original shape did not have to be offset
					// from the body's origin
					btCollisionShape *subShape;
				};				

				union
				{
					// For static objects
					btCollisionObject *collisionObject;
					
					// For dynamic objects
					btRigidBody *body;
				};

				// Used by our motion state to efficiently update position
				Transform *transComp;
			};


		} // namspace Components

	} // namespace Framework

} // namespace Maki