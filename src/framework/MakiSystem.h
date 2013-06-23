#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{
	class Entity;

	class System
	{
	private:
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
				: from(from), msg(msg), arg2(nullptr)
			{
				this->arg1 = reinterpret_cast<uintptr_t>(arg1);
			}

			template<class T>
			inline T *GetArg1() const { return reinterpret_cast<T>(arg1); }

			template<class T>
			inline T *GetArg2() const { return reinterpret_cast<T>(arg1); }

		public:
			Component *from;
			uint64 msg;
			uintptr_t arg1;
			uintptr_t arg2;
		};

	public:
		static void ComponentMakeupChanged(Entity *e, uint64 oldFlags, uint64 newFlags)
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

		template<class T, class U>
		static void PostMessage(Component *from, Component::Message msg, T *arg1, U *arg2)
		{
			messages.push_back(Message(from, msg, arg1, arg2));
		}

		template<class T>
		static void PostMessage(Component *from, Component::Message msg, T *arg1)
		{
			messages.push_back(Message(from, msg, arg1));
		}

		static void ClearMessages()
		{
			messages.clear();
		}

	protected:
		static std::vector<Message> messages;

	private:
		static std::vector<System *> systems;


	public:
		System(uint64 componentMask)
			: componentMask(componentMask)
		{
			systems.push_back(this);
		}

		virtual ~System()
		{
			systems.erase(std::find(std::begin(systems), std::end(systems), this));
		}

		inline bool CompatibleWith(uint64 componentFlags) const
		{
			return (componentMask & componentFlags) == componentMask;
		}

		virtual void ProcessMessages() {}

	protected:
		virtual void Add(Entity *e) = 0;
		virtual void Remove(Entity *e) = 0;
		
	private:
		uint64 componentMask;
	};


} // namespace Maki