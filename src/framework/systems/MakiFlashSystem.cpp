#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiFlashMovieManager.h"
#include "framework/MakiMessageHub.h"
#include "framework/systems/MakiFlashSystem.h"

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{

			FlashSystem::FlashSystem(uint32 messageQueueSize)
				: System(Component::TypeFlag_Flash, 0, messageQueueSize, "FlashSystem")
			{
			}

			FlashSystem::~FlashSystem()
			{
			}

			void FlashSystem::Update(float dt)
			{
				const uint32 nodeCount = nodes.size();
				for(uint32 i = 0; i < nodeCount; i++) {
					Components::Flash *flashComp = nodes[i];
					FlashMovie *mov = FlashMovieManager::Get(flashComp->movie);
					mov->AdvanceState(dt, flashComp->state, 1.0f);
				}
			}

			void FlashSystem::ProcessMessages()
			{
				MessageHub *hub = MessageHub::Get();
				const uint32 messageCount = hub->GetMessageCount();
				if(messageCount == 0) {
					return;
				}
				Message *messages = hub->GetMessages();

				const uint32 count = nodes.size();
				for(uint32 i = 0; i < count; i++) {
					Components::Flash *flashComp = nodes[i];
					const uint64 &uid = flashComp->owner->GetUid();
					for(uint32 j = 0; j < messageCount; j++) {
						const Message &msg = messages[j];
						if(msg.from != uid && (msg.to == 0 || msg.to == uid)) {
							if(msg.msg == Message_PlayTrack) {
								int32 index = FlashMovieManager::Get(flashComp->movie)->GetTrackIndexByNameHash((uint32)msg.arg1);
								flashComp->state.PlayTrack(index);
							}
						}
					}
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