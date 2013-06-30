#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{
	namespace Framework
	{
	
		class Entity;

		class System
		{
		public:
			struct Message
			{
			public:
				template<class T, class U>
				Message(Component *from, Component::Message msg, T *arg1, U *arg2)
					: from(from), msg(msg)
				{
					this->arg1 = reinterpret_cast<uintptr_t>(arg1);
					this->arg2 = reinterpret_cast<uintptr_t>(arg2);
				}

				template<class T>
				Message(Component *from, Component::Message msg, T *arg1)
					: from(from), msg(msg), arg2(0)
				{
					this->arg1 = reinterpret_cast<uintptr_t>(arg1);
				}

				Message(Component *from, Component::Message msg)
					: from(from), msg(msg), arg1(0), arg2(0)
				{
				}

				Message(const Message &other)
					: from(other.from), msg(other.msg), arg1(other.arg1), arg2(other.arg2)
				{
				}

				inline void operator=(const Message &other)
				{
					from = other.from;
					msg = other.msg;
					arg1 = other.arg1;
					arg2 = other.arg2;
				}

				template<class T>
				inline T *GetArg1() const { return reinterpret_cast<T *>(arg1); }

				template<class T>
				inline T *GetArg2() const { return reinterpret_cast<T *>(arg2); }

			public:
				Component *from;
				Component::Message msg;
				uintptr_t arg1;
				uintptr_t arg2;
			};
			
		public:
			static void ComponentMakeupChanged(Entity *e, uint64 oldFlags, uint64 newFlags);
			static void ProcessMessages();

		private:
			static std::vector<System *> systems;
			static std::vector<Message> messages;

		public:
			System(uint64 componentMask);
			virtual ~System();
			inline bool CompatibleWith(uint64 componentFlags) const { return (componentMask & componentFlags) == componentMask; }
			inline void PostMessage(const Message &m) { outgoingMessages.push_back(m); }

		protected:
			virtual void ProcessMessages(const std::vector<Message> &messages) {}

		protected:
			virtual void Add(Entity *e) = 0;
			virtual void Remove(Entity *e) = 0;
		
		private:
			uint64 componentMask;
			std::vector<Message> outgoingMessages;
		};


	} // namespace Framework

} // namespace Maki