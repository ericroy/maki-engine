#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/MakiMessageHub.h"

namespace Maki
{
	namespace Framework
	{

		std::vector<System *> System::systems;	

		uint32 System::GetSystemCount()
		{
			return systems.size();
		}

		void System::ComponentMakeupChanged(Entity *e, uint64 oldFlags, uint64 newFlags)
		{
			const uint32 count = systems.size();
			for(uint32 i = 0; i < count; i++) {
				System *s = systems[i];
				if(s->CompatibleWith(newFlags)) {
					if(!s->CompatibleWith(oldFlags)) {
						// Became compatible
						s->Add(e);
					}
				} else {
					if(s->CompatibleWith(oldFlags)) {
						// Became incompatible
						s->Remove(e);
					}
				}
			}
		}

		void System::DispatchMessages()
		{
			const uint32 count = systems.size();
			uint32 iterations = 0;
			bool done = false;

			MessageHub *hub = MessageHub::Get();

			while(!done) {
				done = true;

				// Amalgamate messages from all systems into global hub
				for(uint32 i = 0; i < count; i++) {
					if(systems[i]->outgoingMessageCount > 0) {
						hub->Put(systems[i]->outgoingMessages.data, systems[i]->outgoingMessageCount);
						systems[i]->outgoingMessageCount = 0;
					}
				}
				
				// Tell each system to process the messages in the hub
				if(hub->GetMessageCount() > 0) {
					for(uint32 i = 0; i < count; i++) {
						systems[i]->ProcessMessages();

						// Processing the message queue could actually result in more messages being dispatched
						// If this is the case, we'll have to run another iteration
						if(systems[i]->outgoingMessageCount > 0) {
							done = false;
						}
					}
					hub->Clear();
				}
								
				iterations++;
				assert(iterations < 20 && "Message dispatch iterations getting out of hand - is there a message cycle?");
			}
		}

	
	



		System::System(uint64 requiredComponentMask, uint64 anyOfComponentMask, uint32 messageQueueSize, const char *typeName)
			: typeName(typeName), requiredComponentMask(requiredComponentMask), anyOfComponentMask(anyOfComponentMask), outgoingMessageCount(0)
		{
			outgoingMessages.SetSize(messageQueueSize);
			systems.push_back(this);
		}

		System::~System()
		{
			systems.erase(std::find(std::begin(systems), std::end(systems), this));
		}



	} // namespace Framework

} // namespace Maki