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
				ComponentPoolBase::Register(typeName, T::TYPE, this);
			}

			virtual ~ComponentPool()
			{
				ComponentPoolBase::Unregister(T::TYPE);
			}

			// This is a little strange, but we don't want to manage these objects with handles.
			// Instead, we will give out a pointer directly to the data, and leak the handle.
			// When the data wants to be deleted, we'll calculate what the handle was, and then free it properly.
			// I should really just write a memory allocator.
			T *Create()
			{
				Handle h = pool.Alloc();
				assert(h != HANDLE_NONE);
				T *c = pool.Get(h);
				new(c) T();
				return c;
			}

			void Destroy(Component *c)
			{
				Handle h = (T *)c - pool.GetBaseAddr();
				pool.Free(h);
			}

			Component::Type GetType()
			{
				return T::TYPE;
			}

		private:
			ResourcePool<T> pool;
			std::string typeName;
		};


	} // namespace Framework

} // namespace Maki