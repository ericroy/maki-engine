#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{
	class Entity;

	class System
	{
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

	protected:
		virtual void Add(Entity *e) = 0;
		virtual void Remove(Entity *e) = 0;

	private:
		uint64 componentMask;
	};


} // namespace Maki