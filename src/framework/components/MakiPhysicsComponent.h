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

			class Physics : public Component
			{
			public:
				enum ObjectType
				{
					ObjectType_StaticMesh = 0,
					ObjectType_RigidBody
				};

			public:
				static const Type TYPE = Type_Physics;
				static const TypeFlag DEPENDENCIES = Type_Transform;

			public:
				Physics();
				virtual ~Physics();

				// Component interface
				bool Init(Document::Node *props);
				bool Init(HandleOrRid meshId);

			public:
				ObjectType objectType;
				union
				{
					Handle mesh;
				};
			};


		} // namspace Components

	} // namespace Framework

} // namespace Maki