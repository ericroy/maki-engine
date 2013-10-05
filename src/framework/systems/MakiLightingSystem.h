#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/MakiComponent.h"
#include "framework/components/MakiLightComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			class Transform;
			class Light;
		}

		namespace Systems
		{

			class LightingSystem : public System2<Components::Transform, Components::Light>
			{
			public:
				LightingSystem(uint32 messageQueueSize);
				virtual ~LightingSystem();
		
				inline uint32 GetLightCount() const { return nodes.size(); }
				inline Entity *GetLight(uint32 index) { return nodes[index].Get<Components::Light>()->owner; }
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki