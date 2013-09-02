#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/MakiComponent.h"
#include "framework/components/MakiNameComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			class Name;
		}

		namespace Systems
		{

			class NameSystem : public System, public PseudoSingleton<NameSystem>
			{
			public:
				NameSystem(uint32 messageQueueSize) : System(Component::TypeFlag_Name, messageQueueSize, "NameSystem"), PseudoSingleton<NameSystem>() {}
				virtual ~NameSystem() {}
		
				inline Entity *GetEntityByName(const char *name) const
				{
					std::map<std::string, Components::Name *>::const_iterator iter = nameMap.find(name);
					if(iter != nameMap.end()) {
						return iter->second->owner;
					}
					return nullptr;
				}

			protected:
				void Add(Entity *e)
				{
					Components::Name *nameComp = e->Get<Components::Name>();
					nameMap[nameComp->name] = nameComp;
				}
				
				void Remove(Entity *e)
				{
					Components::Name *nameComp = e->Get<Components::Name>();
					nameMap.erase(nameComp->name);
				}

			private:
				std::map<std::string, Components::Name *> nameMap;
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki