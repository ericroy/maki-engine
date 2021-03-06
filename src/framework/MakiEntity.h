#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

using namespace Maki::Core;

namespace Maki
{
	namespace Framework
	{
		class SceneLoader;
		class Component;

		class Entity : public Aligned<SIMD_ALIGN>
		{
			friend class SceneLoader;

		private:
			struct  Entry
			{
				Component::Type type;
				Component *c;
				inline bool operator<(const Entry &other) const { return type < other.type; }
				inline bool operator==(const Entry &other) const { return type == other.type; }
			};
		
		private:
			static const uint32 MAX_COMPONENTS = 16;

		public:
			Entity(uint64 uid, bool prototype = false);
			virtual ~Entity();

			inline bool IsPrototype() const { return prototype; }
			inline const uint64 &GetUid() const { return uid; }
			inline const uint64 &GetComponentFlags() const { return flags; }
			inline bool HasComponent(Component::Type type) const { return (flags & (1ULL << type)) != 0; }
			template<class T> inline T *Get() const;
			Component *Get(Component::Type) const;

			void AddComponent(Component *component);
			Component *RemoveComponent(Component::Type type);

			Entity *Clone(bool prototype = false);
		
		protected:
			// Unique identifier for this entity.
			// Can be used to efficiently lookup the entity pointer from the EntityPool.
			uint64 uid;

			// Flags indicating which components are present in this entity.
			// We'll have to do something more clever here once we have more than 64 component types
			uint64 flags;

			// Table for storing handles to components.
			int32 componentCount;
			Entry components[MAX_COMPONENTS];

			// Indicates that this entity is a prototype and will not be registered with
			// any systems.  The sole purpose of a prototype is to be cloned.
			bool prototype;
		};



		template<class T>
		inline T *Entity::Get() const
		{
			if(((1ULL << T::TYPE) & flags) == 0) {
				return nullptr;
			}
			for(int32 i = 0; i < componentCount; i++) {
				const Entry &entry = components[i];
				if(entry.type == T::TYPE) {
	#if _DEBUG
					return dynamic_cast<T *>(entry.c);
	#else
					return static_cast<T *>(entry.c);
	#endif
				}
			}
			assert(false && "expected to find component");
			return nullptr;
		}



	} // namespace Framework

} // namespace Maki