#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"
#include "framework/MakiComponent.h"

namespace Maki
{

	Entity::Entity()
		: flags(DEFAULT_FLAGS),
		position(0.0f),
		orientation(),
		matrix(true),
		world(true),
		updateFunc(nullptr),
		parent(nullptr),
		physicsLink(this),
		drawListNext(nullptr),
		componentCount(0)
	{
		components.SetSize(Engine::Get()->config->GetUint("framework.entity_max_components", DEFAULT_MAX_COMPONENTS));
	}

	Entity::Entity(uint32 flags)
		: flags(flags),
		position(0.0f),
		orientation(),
		matrix(true),
		world(true),
		updateFunc(nullptr),
		parent(nullptr),
		physicsLink(this),
		drawListNext(nullptr),
		componentCount(0)
	{
		components.SetSize(Engine::Get()->config->GetUint("framework.entity_max_components", DEFAULT_MAX_COMPONENTS));
	}

	Entity::~Entity() {
		const int32 size = children.size();
		for(int32 i = 0; i < size; i++) {
			SAFE_DELETE(children[i]);
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

		Vector4 pos(0.0f);
		node->ResolveAsVectorN("pos", 3, pos.vals);

		Vector4 angles(0.0f);
		node->ResolveAsVectorN("angles", 3, angles.vals);
				
		SetMatrix(pos, Quaternion(angles));

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

	void Entity::RecursivelyUpdate(Entity **drawListHead, const Matrix44 &current, float dt) {
		
		world = current * matrix;
		
		if(GetFlag(Flag_Update) && !GetFlag(Flag_Physics)) {
			for(uint32 i = 0; i < componentCount; i++) {
				components[i]->Update(dt);
			}
		
			if(updateFunc != nullptr) {
				updateFunc(dt);
			}
		}
		
		// Append to the draw list if we want to be drawn
		drawListNext = nullptr;
		if(GetFlag(Flag_Draw)) {
			if(*drawListHead == nullptr) {
				*drawListHead = this;
			} else {
				drawListNext = *drawListHead;
				*drawListHead = this;
			}
		}
		
		if(GetFlag(Flag_ProcessChildren)) {
			const int32 size = children.size();
			for(int32 i = 0; i < size; i++) {
				children[i]->RecursivelyUpdate(drawListHead, world, dt);
			}
		}
	}

	void Entity::Draw(Renderer *renderer)
	{
		for(uint32 i = 0; i < componentCount; i++) {
			components[i]->Draw(renderer);
		}
	}

	void Entity::RemoveChild(Entity *e) {
		const int32 size = children.size();
		for(int32 i = 0; i < size; i++) {
			if(children[i] == e) {
				e->parent = nullptr;
				children.erase(children.begin()+i);
				return;
			}
		}
	}


	void Entity::AttachComponent(Component *c)
	{
		assert((componentFlags & c->type) == 0 && "entity already has this component");
		assert(componentCount < components.count && "entity ran out of room for components");
		for(uint32 i = 0; i < componentCount; i++) {
			if(*c < *components[i]) {
				memmove(&components[i+1], &components[i], componentCount - i);
				components[i] = c;
				componentCount++;
				componentFlags |= c->type;
				c->Attach(this);
				return;
			}
		}

		// Append to end
		components[componentCount++] = c;
		componentFlags |= c->type;
		c->Attach(this);
	}

	void Entity::DetachComponent(Component *c)
	{
		assert((componentFlags & c->type) != 0 && "entity did not have this component");
		for(uint32 i = 0; i < componentCount; i++) {
			if(components[i] == c) {
				memmove(&components[i], &components[i+1], componentCount-i-1);
				componentCount--;
				componentFlags &= ~c->type;
				c->Detach();
				return;
			}
		}
		assert(false && "expected to find component");
	}

	bool Entity::SendMessage(Component *from, uint32 message, uintptr_t arg1, uintptr_t arg2)
	{
		for(uint32 i = 0; i < componentCount; i++) {
			Component *c = components[i];
			if(c != from) {
				if(c->HandleMessage(from, message, arg1, arg2)) {
					return true;
				}
			}
		}
		return false;
	}
















	EntityFactory::EntityFactory()
		: flags(Entity::DEFAULT_FLAGS), pos(0.0f), angles(0.0f)
	{
	}

	EntityFactory::~EntityFactory()
	{
	}

	bool EntityFactory::PreCreate(Document::Node *node)
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
		node->ResolveAsVectorN("pos", 3, pos.vals);
		node->ResolveAsVectorN("angles", 3, angles.vals);
		return true;
	}

	Entity *EntityFactory::Create()
	{
		return new Entity(flags);
	}

	void EntityFactory::PostCreate(Entity *e)
	{
		e->SetFlags(flags);
		e->SetMatrix(pos, Quaternion(angles));
	}




} // namespace Maki