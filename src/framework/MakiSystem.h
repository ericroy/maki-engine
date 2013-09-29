#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiMessage.h"

namespace Maki
{
	namespace Framework
	{
	
		class Entity;

		class System
		{
		public:
			static uint32 GetSystemCount();
			static void ComponentMakeupChanged(Entity *e, uint64 oldFlags, uint64 newFlags);
			static void DispatchMessages();

		private:
			static std::vector<System *> systems;

		public:
			System(uint64 requiredComponentMask, uint64 anyOfComponentMask, uint32 messageQueueSize, const char *typeName);
			virtual ~System();
			
			inline bool CompatibleWith(uint64 componentFlags) const
			{
				return (requiredComponentMask & componentFlags) == requiredComponentMask &&
					(anyOfComponentMask == 0 || (anyOfComponentMask & componentFlags) != 0);
			}

			inline void PostMessage(const Message &m)
			{
				if(outgoingMessageCount < outgoingMessages.count) {
					outgoingMessages[outgoingMessageCount++] = m;
				} else {
					Console::Error("Outgoing message queue full!  Message being dropped. (%s)", typeName.c_str());
				}
			}

		protected:
			virtual void ProcessMessages() {}

		protected:
			virtual void Add(Entity *e) = 0;
			virtual void Remove(Entity *e) = 0;
		
		private:
			uint64 requiredComponentMask;
			uint64 anyOfComponentMask;
			uint32 outgoingMessageCount;
			Array<Message> outgoingMessages;
			std::string typeName;
		};


	} // namespace Framework

} // namespace Maki