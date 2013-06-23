#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"
#include "framework/MakiComponent.h"
#include "framework/MakiSystem.h"

namespace Maki
{

	Entity::Entity()
		: flags(DEFAULT_FLAGS),
		updateFunc(nullptr),
		physicsLink(this),
		componentFlags(0)
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
		const int32 compCount = components.size();
		for(uint32 i = compCount-1; i >= 0; i++) {
			Component *c = DetachComponent(i);
			SAFE_DELETE(c);
		}
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
				Component *c = Component::Create(componentType->value);
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

	void Entity::AttachComponent(Component *c)
	{
		assert((componentFlags & c->componentType) == 0 && "entity already has this component");
		auto lowerBound = std::lower_bound(components.begin(), components.end(), c, Component::Comparator());
		components.insert(lowerBound, c);
		c->Attach(this);

		uint64 oldComponentFlags = componentFlags;
		componentFlags |= c->componentType;
		System::ComponentMakeupChanged(this, oldComponentFlags, componentFlags);
	}

	void Entity::DetachComponent(Component *c)
	{
		assert((componentFlags & c->componentType) != 0 && "entity doesn't have this component");
		auto iter = std::find(components.begin(), components.end(), c);
		assert(iter != components.end() && "entity doesn't have this component");
		components.erase(iter);
		c->Detach();

		uint64 oldComponentFlags = componentFlags;
		componentFlags &= ~c->componentType;
		System::ComponentMakeupChanged(this, oldComponentFlags, componentFlags);
	}

	Component *Entity::DetachComponent(uint32 index)
	{
		Component *c = components[index];
		components.erase(std::begin(components) + index);
		c->Detach();

		uint64 oldComponentFlags = componentFlags;
		componentFlags &= ~c->componentType;
		System::ComponentMakeupChanged(this, oldComponentFlags, componentFlags);

		return c;
	}


} // namespace Maki