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
				static std::function<Entity *()> CreateDebugWidget;

			public:
				Camera();
				virtual ~Camera();
				virtual bool Init(Document::Node *node);
				virtual void Attach(Entity *owner);

			public:
				Frustum frustum;
			};

		} // namespace Components

	} // namespace Framework

} // namespace Maki