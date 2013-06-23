#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"
#include "framework/MakiComponent.h"
#include "framework/MakiComponentPool.h"
#include "framework/MakiSystem.h"


namespace Maki
{

	Entity::Entity()
		: flags(DEFAULT_FLAGS),
		updateFunc(nullptr),
		physicsLink(this),
		componentFlags(0),
		componentCount(0)
	{
		
	}

	Entity::Entity(uint32 flags)
		: flags(flags),
		updateFunc(nullptr),
		physicsLink(this),
		componentFlags(0)
	{
	}

	Entity::~Entity() {
		uint64 oldComponentFlags = componentFlags;

		for(uint32 i = componentCount-1; i >= 0; i--) {
			components[i].component->Detach();
			
			ComponentPoolBase *pool = ComponentPoolBase::PoolForType(components[i].componentType);
			pool->Destroy(components[i].component);
		}
				
		componentFlags = 0;
		componentCount = 0;
		System::ComponentMakeupChanged(this, oldComponentFlags, componentFlags);
	}

	bool Entity::Init(Document::Node *node)
	{
		Document::Node *flagsNode = node->Resolve("flags");
		if(flagsNode != nullptr) {
			for(uint32 i = 0; i < flagsNode->count; i++) {
				char *value = flagsNode->children[i]->value;
				bool on = true;
				if(value[0] == '!') {
					on = false;
					value++;
				}
				if(strcmp(value, "draw") == 0) { if(on) { flags |= Entity::Flag_Draw; } else { flags &= ~Entity::Flag_Draw; } }
				else if(strcmp(value, "update") == 0) { if(on) { flags |= Entity::Flag_Update; } else { flags &= ~Entity::Flag_Update; } }
				else if(strcmp(value, "physics") == 0) { if(on) { flags |= Entity::Flag_Physics; } else { flags &= ~Entity::Flag_Physics; } }
				else if(strcmp(value, "process_children") == 0) { if(on) { flags |= Entity::Flag_ProcessChildren; } else { flags &= ~Entity::Flag_ProcessChildren; } }
				else if(strcmp(value, "cast_shadow") == 0) { if(on) { flags |= Entity::Flag_CastShadow; } else { flags &= ~Entity::Flag_CastShadow; } }
			}
		}
		SetFlags(flags);

		// Load each component
		Document::Node *componentList = node->Resolve("components");
		if(componentList != nullptr) {
			for(uint32 i = 0; i < componentList->count; i++) {
				Document::Node *componentType = componentList->children[i];
				
				ComponentPoolBase *pool = ComponentPoolBase::PoolForTypeName(componentType->value);
				if(pool == nullptr) {
					Console::Error("No component pool exists for component of type: %s", componentType->value);
					continue;
				}
				
				Component *c = pool->Create();
				if(c != nullptr) {
					AttachComponent(c);
					c->Init(node);
				}
			}
		}

		return true;
	}

	//void Entity::RecursivelyUpdate(Entity **drawListHead, const Matrix44 &current, float dt) {
	//	
	//	world = current * matrix;
	//	
	//	if(GetFlag(Flag_Update) && !GetFlag(Flag_Physics)) {
	//		const uint32 count = components.size();
	//		for(uint32 i = 0; i < count; i++) {
	//			components[i]->Update(dt);
	//		}
	//	
	//		if(updateFunc != nullptr) {
	//			updateFunc(dt);
	//		}
	//	}
	//	
	//	// Append to the draw list if we want to be drawn
	//	drawListNext = nullptr;
	//	if(GetFlag(Flag_Draw)) {
	//		if(*drawListHead == nullptr) {
	//			*drawListHead = this;
	//		} else {
	//			drawListNext = *drawListHead;
	//			*drawListHead = this;
	//		}
	//	}
	//	
	//	if(GetFlag(Flag_ProcessChildren)) {
	//		const int32 size = children.size();
	//		for(int32 i = 0; i < size; i++) {
	//			children[i]->RecursivelyUpdate(drawListHead, world, dt);
	//		}
	//	}
	//}

	void Entity::AttachComponent(Component *component)
	{
		assert((componentFlags & (1L << component->componentType)) == 0 && "entity already has this component");
		assert(componentCount < MAX_COMPONENTS && "entity exhausted component slots");
		
		ComponentEntry ce;
		ce.componentType = component->componentType;
		ce.component = component;

		ComponentEntry *iter = std::lower_bound(components, &components[componentCount], ce);
		uint32 index = iter - components;

		memmove(&components[index+1], &components[index], componentCount-index);
		components[index] = ce;
		componentCount++;
		
		uint64 oldComponentFlags = componentFlags;
		componentFlags |= component->componentType;
		System::ComponentMakeupChanged(this, oldComponentFlags, componentFlags);

		component->Attach(this);
	}

	Component *Entity::DetachComponent(Component::Type componentType)
	{
		assert((componentFlags & (1L << componentType)) != 0 && "entity doesn't have this component");
		
		ComponentEntry ce;
		ce.componentType = componentType;

		ComponentEntry *iter = std::find(components, &components[componentCount], ce);
		uint32 index = iter - components;
		assert(index != componentCount && "entity expected to have this component");

		Component *c = iter->component;

		memmove(&components[index], &components[index+1], componentCount-index-1);
		componentCount--;
		
		c->Detach();

		uint64 oldComponentFlags = componentFlags;
		componentFlags &= ~componentType;
		System::ComponentMakeupChanged(this, oldComponentFlags, componentFlags);

		return c;
	}


} // namespace Maki