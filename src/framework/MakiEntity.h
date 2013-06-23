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

	private:
		struct ComponentEntry
		{
			Component::Type componentType;
			Component *component;

			inline bool operator<(const ComponentEntry &other) const { return componentType < other.componentType; }
			inline bool operator==(const ComponentEntry &other) const { return componentType == other.componentType; }
		};

	public:
		static const uint32 DEFAULT_FLAGS = Flag_Update|Flag_ProcessChildren;
		static const uint32 MAX_COMPONENTS = 16;
		
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
		inline bool HasComponent(Component::Type componentType) const { return (componentFlags & (1LL << componentType)) != 0; }
		void AttachComponent(Component *component);
		Component *DetachComponent(Component::Type componentType);
		template<class T> inline T *Get() const;

	protected:
		// Table for storing handles to components
		ComponentEntry components[MAX_COMPONENTS];

		// Number of entries used in the components array above
		uint32 componentCount;

		// Flags indicating which components are present in this entity
		uint64 componentFlags;
		
		// Entity flags (not related to components)
		uint32 flags;

	public:
		std::function<void(float)> updateFunc;
		PhysicsLink physicsLink;
	};



	template<class T>
	inline T *Entity::Get() const
	{
		if((T::COMPONENT_TYPE & componentFlags) == 0) {
			return nullptr;
		}
		for(uint32 i = 0; i < componentCount; i++) {
			const ComponentEntry &entry = components[i];
			if(entry.componentType == T::COMPONENT_TYPE) {
#if _DEBUG
				return dynamic_cast<T *>(entry.component);
#else
				return static_cast<T *>(entry.component);
#endif
			}
		}
		assert(false && "expected to find component");
		return nullptr;
	}



} // namespace Maki