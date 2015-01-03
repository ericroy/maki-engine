#include "framework/framework_stdafx.h"
#include "framework/systems/MakiCameraSystem.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{

			CameraSystem::CameraSystem(uint32 messageQueueSize)
				: System(Component::TypeFlag_Transform|Component::TypeFlag_Camera, 0, messageQueueSize, "CameraSystem")
			{
			}

			CameraSystem::~CameraSystem()
			{
			}

		} // namespace Systems

	} // namespace Framework

} // namespace Maki