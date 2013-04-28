#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"

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
		drawListNext(nullptr)
	{
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
		drawListNext(nullptr)
	{
	}

	Entity::~Entity() {
		const int32 size = children.size();
		for(int32 i = 0; i < size; i++) {
			SAFE_DELETE(children[i]);
		}
	}

	void Entity::RecursivelyUpdate(Entity **drawListHead, const Matrix44 &current, float dt) {
		
		world = current * matrix;
		
		if(GetFlag(Flag_Update) && !GetFlag(Flag_Physics)) {
			Update(dt);
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