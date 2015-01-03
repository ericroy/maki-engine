#pragma once
#include "framework/framework_stdafx.h"
#include "core/MakiPseudoSingleton.h"
#include "framework/MakiMessage.h"

namespace Maki
{
	namespace Framework
	{

		class MessageHub : public Core::PseudoSingleton<MessageHub>
		{
		public:
			MessageHub(uint32 messageCapacity);
			virtual ~MessageHub();

			bool Put(const Message *messages, uint32 count);
			inline void Clear() { messageCount = 0; }
			inline Message *GetMessages() const { return messages.data; }
			inline uint32 GetMessageCount() const { return messageCount; }

		private:
			uint32 messageCount;
			Array<Message> messages;
		};

	} // namespace Framework

} // namespace Maki