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
				static const Type TYPE = Type_Physics;
				static const TypeFlag DEPENDENCIES = Type_Transform;

			public:
				Physics();
				virtual ~Physics();

				// Component interface
				bool Init(Document::Node *props);
				void OnAttach();
				void OnDetach();
				
				// btMotionState interface
				void getWorldTransform(btTransform &worldTransform) const;
				void setWorldTransform(const btTransform &worldTransform);

			private:
				Transform *transComp;
			};


		} // namspace Components

	} // namespace Framework

} // namespace Maki