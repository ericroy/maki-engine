#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiMessage.h"

namespace Maki
{
	namespace Framework
	{
	
		class Entity;

		class SystemBase
		{
		public:
			static uint32 GetSystemCount();
			static void ComponentMakeupChanged(Entity *e, uint64 oldFlags, uint64 newFlags);
			static void DispatchMessages();

		private:
			static std::vector<SystemBase *> systems;

		public:
			SystemBase(uint64 requiredComponentMask, uint64 anyOfComponentMask, uint32 messageQueueSize, const char *typeName);
			virtual ~SystemBase();

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

			virtual void Add(Entity *e) = 0;
			virtual void Remove(Entity *e) = 0;
			virtual void ProcessMessages() {}

		private:
			uint64 requiredComponentMask;
			uint64 anyOfComponentMask;
			uint32 outgoingMessageCount;
			Array<Message> outgoingMessages;
			std::string typeName;
		};


		template<typename... Types>
		class System : public SystemBase
		{
		protected:
			typedef std::tuple<Types *...> TupleType;
			typedef typename std::tuple_element<0, std::tuple<Types...>>::type ZerothType;

		public:
			System(uint64 requiredComponentMask, uint64 anyOfComponentMask, uint32 messageQueueSize, const char *typeName)
				: SystemBase(requiredComponentMask, anyOfComponentMask, messageQueueSize, typeName)
			{

			}
			
			virtual ~System()
			{
			}
			
		protected:
			virtual void Add(Entity *e)
			{
				nodes.push_back(std::make_tuple(e->Get<Types>()...));
			}

			virtual void Remove(Entity *e)
			{
				using namespace std;

				ZerothType *ptr = e->Get<ZerothType>();
				nodes.erase(find_if(begin(nodes), end(nodes), [ptr](const TupleType &item) {
					return ptr == std::get<0>(item);
				}));
			}

		protected:
			std::vector<TupleType> nodes;
		};
		

	} // namespace Framework

} // namespace Maki