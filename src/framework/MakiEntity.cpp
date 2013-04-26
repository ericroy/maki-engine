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
	:	flags(flags),
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


} // namespace Maki