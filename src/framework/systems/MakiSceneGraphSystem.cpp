#pragma once
#include "framework/framework_stdafx.h"
#include "framework/systems/MakiSceneGraphSystem.h"
#include "framework/components/MakiSceneNodeComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{

			SceneGraphSystem::SceneGraphSystem(uint32 messageQueueSize)
				: System(Component::TypeFlag_SceneNode, 0, messageQueueSize, "SceneGraphSystem")
			{
			}

			SceneGraphSystem::~SceneGraphSystem()
			{
			}

		} // namespace Systems

	} // namespace Framework

} // namespace Maki