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

			LightingSystem::LightingSystem()
				: System(Component::TypeFlag_Light|Component::TypeFlag_Transform)
			{
			}

			LightingSystem::~LightingSystem()
			{
			}

			void LightingSystem::Add(Entity *e)
			{
				Node n;
				n.lightComp = e->Get<Components::Light>();
				n.transComp = e->Get<Components::Transform>();
				nodes.push_back(n);
			}

			void LightingSystem::Remove(Entity *e)
			{
				Node n;
				n.lightComp = e->Get<Components::Light>();
				n.transComp = e->Get<Components::Transform>();
				nodes.erase(std::find(std::begin(nodes), std::end(nodes), n));
			}

		} // namespace Systems

	} // namespace Framework

} // namespace Maki