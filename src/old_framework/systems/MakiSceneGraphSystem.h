#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			class SceneNode;
		}

		namespace Systems
		{
			class SceneGraphSystem : public System<Components::SceneNode>
			{
			public:
				SceneGraphSystem(uint32 messageQueueSize);
				virtual ~SceneGraphSystem();
			};
		
		} // namespace Systems

	} // namespace Framework

} // namespace Maki