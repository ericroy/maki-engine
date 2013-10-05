#pragma once
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
				: System2(Component::TypeFlag_Camera|Component::TypeFlag_Transform, 0, messageQueueSize, "CameraSystem")
			{
			}

			CameraSystem::~CameraSystem()
			{
			}

		} // namespace Systems

	} // namespace Framework

} // namespace Maki