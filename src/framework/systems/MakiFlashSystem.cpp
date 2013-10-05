#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiFlashMovieManager.h"
#include "framework/MakiMessageHub.h"
#include "framework/MakiEntityPool.h"
#include "framework/systems/MakiFlashSystem.h"

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{

			FlashSystem::FlashSystem(uint32 messageQueueSize)
				: System1(Component::TypeFlag_Flash, 0, messageQueueSize, "FlashSystem")
			{
			}

			FlashSystem::~FlashSystem()
			{
			}

			void FlashSystem::Update(float dt)
			{
				const uint32 nodeCount = nodes.size();
				for(uint32 i = 0; i < nodeCount; i++) {
					Components::Flash *flashComp = nodes[i].Get<Components::Flash>();
					FlashMovie *mov = FlashMovieManager::Get(flashComp->movie);

					bool wasFinished = flashComp->state.IsFinished();
					mov->AdvanceState(dt, flashComp->state, 1.0f);
					if(!wasFinished && flashComp->state.IsFinished()) {
						uint32 trackNameHash = mov->tracks[flashComp->state.GetTrackIndex()].nameHash;
						Systems::FlashSystem::PostMessage(Message(flashComp->owner->GetUid(), 0, Message_TrackComplete, trackNameHash));
					}
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

				EntityPool *pool = EntityPool::Get();

				for(uint32 i = 0; i < messageCount; i++) {
					const Message &msg = messages[i];
					if(msg.msg == Message_PlayTrack) {
						if(msg.to != 0) {
							Components::Flash *flashComp = pool->GetEntity(msg.to)->Get<Components::Flash>();
							if(flashComp != nullptr) {
								int32 index = FlashMovieManager::Get(flashComp->movie)->GetTrackIndexByNameHash((uint32)msg.arg1);
								flashComp->state.PlayTrack(index);
							}
						} else {
							const uint32 count = nodes.size();
							for(uint32 j = 0; j < count; j++) {
								Components::Flash *flashComp = nodes[j].Get<Components::Flash>();
								int32 index = FlashMovieManager::Get(flashComp->movie)->GetTrackIndexByNameHash((uint32)msg.arg1);
								flashComp->state.PlayTrack(index);
							}
						}
					}
				}
			}

		} // namespace Systems

	} // namespace Framework

} // namespace Maki