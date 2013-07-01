#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"

namespace Maki
{
	namespace Framework
	{

		std::vector<System *> System::systems;
		std::vector<System::Message> System::messages;
	

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

		void System::ProcessMessages()
		{
			const uint32 count = systems.size();
			uint32 iterations = 0;
			bool done = false;

			while(!done) {
				done = true;

				// Amalgamate messages from all systems into global queue
				for(uint32 i = 0; i < count; i++) {
					uint32 outgoingCount = systems[i]->outgoingMessages.size();
					if(outgoingCount > 0) {
						messages.reserve(messages.size() + outgoingCount);
						std::copy(std::begin(systems[i]->outgoingMessages), std::end(systems[i]->outgoingMessages), std::back_inserter(messages));
						systems[i]->outgoingMessages.clear();
					}
				}
				
				// Allow each system to process the global queue
				if(messages.size()) {
					for(uint32 i = 0; i < count; i++) {
						systems[i]->ProcessMessages(messages);

						// Processing the message queue could actually result in more messages being dispatched
						// If this is the case, we'll have to run another iteration
						if(systems[i]->outgoingMessages.size()) {
							done = false;
						}
					}
					messages.clear();
				}
								
				iterations++;
				assert(iterations < 20 && "Message dispatch iterations getting out of hand - is there a message cycle?");

			}
		}

	
	



		System::System(uint64 componentMask)
			: componentMask(componentMask)
		{
			systems.push_back(this);
		}

		System::~System()
		{
			systems.erase(std::find(std::begin(systems), std::end(systems), this));
		}



	} // namespace Framework

} // namespace Maki