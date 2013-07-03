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
				bool Init(HandleOrRid meshId);
				bool Init(const Vector4 &minCorner, const Vector4 &maxCorner);

			public:
				ObjectType objectType;
				ObjectShape objectShape;
				union
				{
					Handle mesh;
					struct {
						Vector4 minCorner;
						Vector4 maxCorner;
					};
				};
			};


		} // namspace Components

	} // namespace Framework

} // namespace Maki