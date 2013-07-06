#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			class Camera : public Component
			{
			public:
				static const Type TYPE = Type_Camera;
				static const TypeFlag DEPENDENCIES = TypeFlag_Transform;

			public:
				Camera();
				virtual ~Camera();
				bool Init(Document::Node *props);
				Camera *Clone(bool prototype);

			public:
				bool active;
				Frustum frustum;
			};


		} // namspace Components

	} // namespace Framework

} // namespace Maki