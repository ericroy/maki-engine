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
		
		inline uint32 GetFlags() const { return flags; }
		inline void SetFlags(uint32 f) { flags = f; }
		inline bool GetFlag(Flag f) const { return (f & flags) != 0; }
		inline void SetFlag(Flag f, bool on = true) { if(on) { flags |= f; } else { flags &= ~f; } }
		
		// Component stuff
		inline uint64 GetComponentFlags() const { return componentFlags; }
		inline bool HasComponent(uint64 componentType) const { return (componentFlags & componentType) != 0; }
		void AttachComponent(Component *c);
		void DetachComponent(Component *c);
		Component *DetachComponent(uint32 index);
		template<class T> inline T *Get() const;

	public:
		std::function<void(float)> updateFunc;
		PhysicsLink physicsLink;

	protected:
		uint32 flags;	

		// Component stuff
		uint64 componentFlags;
		std::vector<Component *> components;
	};



	template<class T>
	inline T *Entity::Get() const
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



} // namespace Maki