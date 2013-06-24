#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"
#include "framework/MakiComponentPoolBase.h"

namespace Maki
{
	namespace Framework
	{

		template<class T>
		class ComponentPool : public ComponentPoolBase, public PseudoSingleton< ComponentPool<T> >
		{
		public:
			ComponentPool(const char *typeName, uint32 size)
				: PseudoSingleton< ComponentPool<T> >(),
				pool(size),
				typeName(typeName)
			{
				assert(componentPoolRegistry[T::COMPONENT_TYPE] == nullptr && "This component pool already exists");
			
				poolForType[T::COMPONENT_TYPE] = this;
				poolForTypeName[typeName] = this;
			}

			virtual ~ComponentPool()
			{
				poolForType[T::COMPONENT_TYPE] = nullptr;
				poolForTypeName.erase(typeName);
			}

			Component *Create()
			{
				auto pool = (ResourcePool<T> *)poolForType[T::COMPONENT_TYPE];
				Handle h = pool->Alloc();
				assert(h != HANDLE_NONE);

				// This is a little strange, but we don't want to manage these objects with handles.
				// Instead, we will give out a pointer directly to the data, and leak the handle.
				// When the data wants to be deleted, we'll calculate what the handle was, and then free it properly.
				// I should really just write a memory allocator.
				T *c = pool->Get(h);
				new(c) T();
				return c;
			}

			void Destroy(Component *c)
			{
				auto pool = (ResourcePool<T> *)poolForType[T::COMPONENT_TYPE];
				Handle h = c - pool->GetBaseAddr();
				pool->Free(h);
			}

		private:
			ResourcePool<T> pool;
			std::string typeName;
		};


	} // namespace Framework

} // namespace Maki