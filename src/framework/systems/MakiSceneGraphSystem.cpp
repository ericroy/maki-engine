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

			SceneGraphSystem::SceneGraphSystem()
				: System(Component::TypeFlag_SceneNode)
			{
			}

			SceneGraphSystem::~SceneGraphSystem()
			{
			}

			void SceneGraphSystem::Add(Entity *e)
			{
				nodes.push_back(e->Get<Components::SceneNode>());
			}

			void SceneGraphSystem::Remove(Entity *e)
			{
				nodes.erase(std::find(std::begin(nodes), std::end(nodes), e->Get<Components::SceneNode>()));
			}

		} // namespace Systems

	} // namespace Framework

} // namespace Maki