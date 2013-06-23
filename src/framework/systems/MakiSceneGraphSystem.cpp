#pragma once
#include "framework/framework_stdafx.h"
#include "framework/systems/MakiSceneGraphSystem.h"
#include "framework/components/MakiSceneNodeComponent.h"

namespace Maki
{
	SceneGraphSystem::SceneGraphSystem()
		: System(Component::Type_SceneNode)
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
				m.from->owner->Get<SceneNodeComponent>()->AddChild(m.GetArg1<Entity>());
				break;
			}
		}
	}

	void SceneGraphSystem::Add(Entity *e)
	{
		items.push_back(e->Get<SceneNodeComponent>());
	}

	void SceneGraphSystem::Remove(Entity *e)
	{
		items.erase(std::find(std::begin(items), std::end(items), e->Get<SceneNodeComponent>()));
	}


} // namespace Maki