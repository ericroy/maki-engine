#include "framework/framework_stdafx.h"
#include "framework/MakiMessageHub.h"

namespace Maki
{
	namespace Framework
	{

		MessageHub::MessageHub(uint32 messageCapacity)
			: PseudoSingleton<MessageHub>(), messageCount(0)
		{
			messages.SetSize(messageCapacity);
		}

		MessageHub::~MessageHub()
		{
		}

		bool MessageHub::Put(const Message *msgs, uint32 count)
		{
			if(messageCount + count <= messages.count) {
				std::copy(msgs, &msgs[count], &messages[messageCount]);
				messageCount += count;
				return true;
			}
			return false;
		}

	} // namespace Core

} // namespace Maki