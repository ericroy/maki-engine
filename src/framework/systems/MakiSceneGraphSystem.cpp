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
				: System(1ULL << Component::Type_SceneNode)
			{
			}

			SceneGraphSystem::~SceneGraphSystem()
			{
			}

			void SceneGraphSystem::ProcessMessages(const std::vector<Message> &messages)
			{
				const uint32 count = messages.size();
				for(uint32 i = 0; i < count; i++) {
					const Message &m = messages[i];
			
					switch(m.msg)
					{
					case Component::Message_DebugWidgetCreated:
						m.from->owner->Get<Components::SceneNode>()->AddChild(m.GetArg1<Entity>());
						break;
					}
				}
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