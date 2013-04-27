#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResourcePool.h"

namespace Maki
{

	template<class T, class SubClass>
	class Manager
	{
	public:
		static inline SubClass *GetOwner(Handle handle) { return managers[(handle & MANAGER_ID_MASK)>>MANAGER_ID_SHIFT]; }

		static inline T *Get(Handle handle)
		{
			if(handle == HANDLE_NONE) {
				return nullptr;
			}
			return managers[handle>>MANAGER_ID_SHIFT]->resPool->Get(handle & HANDLE_VALUE_MASK);
		}

		static inline void AddRef(Handle handle)
		{
			if(handle == HANDLE_NONE) {
				return;
			}
			managers[handle>>MANAGER_ID_SHIFT]->resPool->AddRef(handle & HANDLE_VALUE_MASK);
		}

		static inline void Free(Handle &handle)
		{
			if(handle != HANDLE_NONE) {
				managers[handle>>MANAGER_ID_SHIFT]->resPool->Free(handle & HANDLE_VALUE_MASK);
				handle = HANDLE_NONE;
			}
		}

		static inline void Free(uint32 count, Handle *handles)
		{
			for(uint32 i = 0; i < count; i++) {
				Free(handles[i]);
			}
		}

	public:
		static const int32 DEFAULT_SIZE = 64;
		static const uint32 BITS_PER_MANAGER_ID = 3;
		
		// Must subtract one here, because we can't have a manager id that is all ones.
		// If such a manager id was paired with a maximum resource index, then the
		// resulting (valid) handle would be equal to HANDLE_NONE.  Can't have that.
		static const uint32 MAX_MANAGERS_PER_RESOURCE_TYPE = (1<<BITS_PER_MANAGER_ID)-1;
		static const uint32 MANAGER_ID_SHIFT = 32-BITS_PER_MANAGER_ID;
		static const uint32 MANAGER_ID_MASK = MAX_MANAGERS_PER_RESOURCE_TYPE << MANAGER_ID_SHIFT;
		static const uint32 HANDLE_VALUE_MASK = ~MANAGER_ID_MASK;
		
	private:
		static SubClass *managers[MAX_MANAGERS_PER_RESOURCE_TYPE];
		
	public:
		Manager(uint32 size = DEFAULT_SIZE)
		{
			assert(size <= (1<<MANAGER_ID_SHIFT)-1 && "Cannot create a manager this large");

			// Find the first unused manager id and mark it as used
			managerId = (uint32)-1;
			for(uint32 i = 0; i < MAX_MANAGERS_PER_RESOURCE_TYPE; i++) {
				if(managers[i] == nullptr) {
					managers[i] = static_cast<SubClass *>(this);
					managerId = i << MANAGER_ID_SHIFT;
					break;
				}
			}
			assert(managerId != (uint32)-1 && "Too many managers for this resource type");

			resPool = new ResourcePool<T>(size);
		}

		virtual ~Manager()
		{
			// Mark our manager id as available again
			managers[managerId>>MANAGER_ID_SHIFT] = nullptr;
			SAFE_DELETE(resPool);
		}

		inline uint32 GetSize() { return resPool->GetSize(); }
		inline uint32 GetCapacity() { return resPool->GetCapacity(); }

		// Finds an item using resource's equality operator
		// Adds a reference
		inline Handle Find(const T &item) { return resPool->Find(item) | managerId; }

		// Adds an item to the resource pool using move semantics
		// Adds a reference
		inline Handle Add(const Movable<T> &item) { return resPool->Alloc(item) | managerId; }

		// Adds an item to the resource pool, relying on copy constructor
		// Adds a reference
		inline Handle Add(const T &item) { return resPool->Alloc(item) | managerId; }

		virtual void Reset()
		{
			uint32 size = resPool->GetCapacity();
			SAFE_DELETE(resPool);
			resPool = new ResourcePool<T>(size);
		}

		void DumpStats(const char *label)
		{
			uint32 size = resPool->GetSize();
			uint32 cap = resPool->GetCapacity();
			Console::Info("%s: %d/%d (%d b / %d b)", label, size, cap, sizeof(T)*size, sizeof(T)*cap);
		}

		void DumpItems()
		{
			Console::Info("Manager items:");
			const ResourcePool<T>::Iterator end = resPool->End();
			for(ResourcePool<T>::Iterator iter = resPool->Begin(); iter != end; ++iter) {
				Console::Info("Item handle=%d refcount=%d", iter.Index(), iter.RefCount());
			}
		}

	protected:
		// A number to differentiate managers which hold the same type of resources
		// Each handle value has the manager id in the topmost 5 bits
		uint32 managerId;

		ResourcePool<T> *resPool;
	};



	
	template<class T, class SubClass>
	SubClass *Manager<T, SubClass>::managers[MAX_MANAGERS_PER_RESOURCE_TYPE] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	

} // namespace Maki
