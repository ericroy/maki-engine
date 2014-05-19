#pragma once
#include "framework/framework_stdafx.h"
#include "framework/systems/MakiLightingSystem.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{

			LightingSystem::LightingSystem(uint32 messageQueueSize)
				: System(Component::TypeFlag_Transform|Component::TypeFlag_Light, 0, messageQueueSize, "LightingSystem")
			{
			}

			LightingSystem::~LightingSystem()
			{
			}

		} // namespace Systems

	} // namespace Framework

} // namespace Maki