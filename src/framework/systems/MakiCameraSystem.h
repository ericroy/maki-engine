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

			class CameraSystem : public System2<Components::Transform, Components::Camera>
			{
			public:
				CameraSystem(uint32 messageQueueSize);
				virtual ~CameraSystem();
				inline uint32 GetCameraCount() const { return nodes.size(); }
				inline Entity *GetCamera(uint32 index) { return nodes[index].Get<Components::Camera>()->owner; }
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki