#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiFlashMovieManager.h"
#include "framework/systems/MakiFlashSystem.h"

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{

			FlashSystem::FlashSystem(uint32 messageQueueSize)
				: System(Component::TypeFlag_Flash, messageQueueSize, "FlashSystem")
			{
			}

			FlashSystem::~FlashSystem()
			{
			}

			void FlashSystem::Update(float dt)
			{
				Engine *eng = Engine::Get();

				const uint32 nodeCount = nodes.size();
				for(uint32 i = 0; i < nodeCount; i++) {
					Components::Flash *flashComp = nodes[i];
					
					FlashMovie *mov = FlashMovieManager::Get(flashComp->movie);
					mov->AdvanceState(dt, flashComp->state, 1.0f);
				}
			}

			void FlashSystem::Add(Entity *e)
			{
				Components::Flash *flashComp = e->Get<Components::Flash>();
				assert(flashComp != nullptr);
				nodes.push_back(flashComp);
			}

			void FlashSystem::Remove(Entity *e)
			{
				Components::Flash *flashComp = e->Get<Components::Flash>();
				nodes.erase(std::find(std::begin(nodes), std::end(nodes), flashComp));
			}

		} // namespace Systems

	} // namespace Framework

} // namespace Maki