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
				static const Component::Type TYPE = Type_Camera;

			public:
				Camera();
				virtual ~Camera();
				bool Init(Document::Node *props);

			public:
				bool active;
				Frustum frustum;
			};


		} // namspace Components

	} // namespace Framework

} // namespace Maki