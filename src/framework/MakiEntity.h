#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiPhysicsLink.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	class Renderer;
	class Component;

	class Entity : public Aligned<SIMD_ALIGN>
	{
	public:
		enum Flag
		{
			Flag_None = 0,
			Flag_Draw = 1<<0,
			Flag_Update = 1<<1,
			Flag_Physics = 1<<2,
			Flag_ProcessChildren = 1<<3,
			Flag_CastShadow = 1<<4,
		};

	public:
		static const uint32 DEFAULT_FLAGS = Flag_Update|Flag_ProcessChildren;
		
	public:
		Entity();
		Entity(uint32 flags);
		virtual ~Entity();

		bool Init(Document::Node *node);

		void RecursivelyUpdate(Entity **drawListHead, const Matrix44 &current, float dt);
		void Draw(Renderer *renderer);
		
		inline uint32 GetFlags() const { return flags; }
		inline void SetFlags(uint32 f) { flags = f; }
		inline bool GetFlag(Flag f) const { return (f & flags) != 0; }
		inline void SetFlag(Flag f, bool on = true) { if(on) { flags |= f; } else { flags &= ~f; } }
		
		// Component system interface
		inline uint64 GetComponentFlags() const { return componentFlags; }
		inline bool HasComponent(Component::Type componentType) const { return (componentFlags & componentType) != 0; }
		void AttachComponent(Component *c);
		void DetachComponent(Component *c);
		Component *DetachComponent(uint32 index);
		template<class T> T *Get() const;
		template<class T, class U> bool SendMessage(Component *from, Component::Message message, T *arg1, U *arg2);

	public:
		std::function<void(float)> updateFunc;
		PhysicsLink physicsLink;

	protected:
		uint32 flags;	

		// Component system
		uint64 componentFlags;
		std::vector<Component *> components;
	};





	

	template<class T>
	T *Entity::Get() const
	{
		if((T::COMPONENT_TYPE & componentFlags) == 0) {
			return nullptr;
		}
		const uint32 count = components.size();
		for(uint32 i = 0; i < count; i++) {
			Component *c = components[i];
			if(c->componentType == T::COMPONENT_TYPE) {
#if _DEBUG
				return dynamic_cast<T *>(c);
#else
				return static_cast<T *>(c);
#endif
			}
		}
		assert(false && "expected to find component");
		return nullptr;
	}



	template<class T, class U>
	bool Entity::SendMessage(Component *from, Component::Message message, T *arg1, U *arg2)
	{
		uintptr_t a1 = reinterpret_cast<uintptr_t>(arg1);
		uintptr_t a2 = reinterpret_cast<uintptr_t>(arg2);

		uint32 count = components.size();
		for(uint32 i = 0; i < count; i++) {
			Component *c = components[i];
			if(c != from && c->messageHandler != nullptr) {
				if(c->messageHandler(from, message, a1, a2)) {
					return true;
				}
			}
		}
		return false;
	}


} // namespace Maki