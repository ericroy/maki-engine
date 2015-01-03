#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/MakiComponent.h"
#include "framework/components/MakiCameraComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			class Transform;
			class Camera;
		}

		namespace Systems
		{

			class FlyCamSystem : public System<Components::Transform, Components::Camera>
			{
			public:
				FlyCamSystem(uint32 messageQueueSize);
				virtual ~FlyCamSystem();
				void Update(float dt);
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki