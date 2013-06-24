#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"
#include "framework/MakiComponent.h"
#include "framework/MakiComponentPool.h"
#include "framework/MakiSystem.h"


namespace Maki
{
	namespace Framework
	{

		Entity::Entity()
			: flags(0),
			componentCount(0)
		{	
		}

		Entity::~Entity() {
			uint64 oldFlags = flags;

			for(uint32 i = componentCount-1; i >= 0; i--) {
				ComponentPoolBase *pool = ComponentPoolBase::PoolForType(components[i].type);
				pool->Destroy(components[i].c);
			}
				
			flags = 0;
			componentCount = 0;
			System::ComponentMakeupChanged(this, oldFlags, flags);
		}

		void Entity::AddComponent(Component *component)
		{
			assert((flags & (1ULL << component->type)) == 0 && "entity already has this component");
			assert(componentCount < MAX_COMPONENTS && "entity exhausted component slots");
		
			component->owner = this;

			Entry ce;
			ce.type = component->type;
			ce.c = component;

			Entry *iter = std::lower_bound(components, &components[componentCount], ce);
			uint32 index = iter - components;

			memmove(&components[index+1], &components[index], componentCount-index);
			components[index] = ce;
			componentCount++;
		
			uint64 oldFlags = flags;
			flags |= (1ULL << component->type);
			System::ComponentMakeupChanged(this, oldFlags, flags);
		}

		Component *Entity::RemoveComponent(Component::Type type)
		{
			assert((flags & (1ULL << type)) != 0 && "entity doesn't have this component");
		
			Entry ce;
			ce.type = type;

			Entry *iter = std::find(components, &components[componentCount], ce);
			uint32 index = iter - components;
			assert(index != componentCount && "entity expected to have this component");

			Component *c = iter->c;
		
			memmove(&components[index], &components[index+1], componentCount-index-1);
			componentCount--;
		
			uint64 oldFlags = flags;
			flags &= ~(1ULL << type);
			System::ComponentMakeupChanged(this, oldFlags, flags);

			c->owner = nullptr;
			return c;
		}


	} // namespace Framework

} // namespace Maki