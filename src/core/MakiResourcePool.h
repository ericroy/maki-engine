#pragma once
#include "core/core_stdafx.h"
#include <functional>

namespace Maki
{
	namespace Core
	{

		/*
		This class manages an array of objects.  Objects in the array
		are preallocated but constructors are called as they are requested,
		and destructors are called as they are freed. All the managed objects
		are reference counted, so they will only be destroyed if their reference
		count falls to zero.  References are increased whenever a handle is given
		out, and decreased whenever a handle is freed.
		*/
		template<class T>
		class ResourcePool
		{
		private:
			struct Node
			{
				uint32 next, prev;
			};

		public:
			class Iterator
			{
				friend class ResourcePool<T>;
			public:
				inline void operator++() { current = nodes[current].next; }
				inline bool operator==(const Iterator &iter) const { return iter.current == current; }
				inline bool operator!=(const Iterator &iter) const { return iter.current != current; }
				inline const T &operator*() const { return data[current]; }
				inline T *Ptr() const { return &data[current]; }
				inline uint32 Index() const { return current; }
				inline uint16 RefCount() const { return refCounts[current]; }
		
			private:
				Iterator(Node *nodes, T *data, uint16 *refCounts, uint32 current)
				:	nodes(nodes), data(data), refCounts(refCounts), current(current) {}

			private:
				Node *nodes;
				T *data;
				uint16 *refCounts;
				uint32 current;
			};

			inline Iterator Begin() const { return Iterator(nodes, data, referenceCounts, head); }
			inline const Iterator End() const { return Iterator(nodes, data, referenceCounts, end); }

		public:
			ResourcePool(uint32 maxSize, const char *debugName)
				: data(0), count(0)
			{

				capacity = maxSize;

				data = (T*)_aligned_malloc(sizeof(T) * capacity, std::alignment_of<T>::value);
				referenceCounts = new uint16[capacity];
		
				// Allocate one extra node as the "end" element
				nodes = new Node[capacity+1];

				// Link up the nodes
				for(uint32 i = 0; i < capacity; i++)
				{
					nodes[i].next = i+1;
					nodes[i].prev = i-1;
				}

				// First element's prev ponts to end
				nodes[0].prev = capacity;
		
				// End element's next and prev point to itself
				end = capacity;
				nodes[end].next = nodes[end].prev = capacity;
		
				head = end;
				freeHead = 0;


				assert(data && nodes && referenceCounts);	
				memset(data, 0, capacity*sizeof(T));
				memset(referenceCounts, 0, capacity*sizeof(uint16));

#if _DEBUG
				this->debugName = debugName;
#endif
			}

			~ResourcePool()
			{
	#ifdef _DEBUG
				for(uint32 i = 0; i < count; i++) {
					if(referenceCounts[i] > 0) {
						Console::Warning("~ResourcePool() \"%s\": Warning, item at index=%u still allocated. (refcount=%d)", debugName.c_str(), i, referenceCounts[i]);
					}
				}
	#endif
				if(data != nullptr) {
					_aligned_free(data);
				}
				SAFE_DELETE_ARRAY(nodes);
				SAFE_DELETE_ARRAY(referenceCounts);
				end = head = freeHead = -1;
			}

			inline Handle Alloc(const MoveToken<T> &value)
			{
				Handle handle = Alloc();
				assert(handle != HANDLE_NONE);
				T *mem = Get(handle);
				assert(mem);
				// Use placement new to call move constructor
				new(mem) T(value);
				return handle;
			}

			inline Handle Alloc(const T &value)
			{
				Handle handle = Alloc();
				assert(handle != HANDLE_NONE);
				T *mem = Get(handle);
				assert(mem);
				// Use placement new to call copy constructor
				new(mem) T(value);
				return handle;
			}

			Handle Alloc()
			{
				Handle handle = HANDLE_NONE;
				if(freeHead != end) {
					handle = freeHead;

					// Move the free head to the next free element
					freeHead = nodes[freeHead].next;

					// insert the new element at the head
					uint32 oldHead = head;
					head = handle;
					if(oldHead != end) {
						nodes[oldHead].prev = head;
					}
					nodes[head].next = oldHead;
					nodes[head].prev = end;

					// Adjust ref counts
					assert(referenceCounts[handle] == 0);
					referenceCounts[handle]++;

					count++;
				} else {
					assert(false && "Resource pool depleted");
				}
				return handle;
			}

			void Free(Handle handle)
			{
				assert(handle < capacity);
				assert(referenceCounts[handle] > 0);
		
				// Remove a reference from this item
				referenceCounts[handle]--;

				// Actually mark this object as deallocated if nobody else has a ref to it
				if(referenceCounts[handle] == 0) {
					count--;
			
					// Remove this element from allocated list
					if(handle == head) {
						head = nodes[handle].next;
					}
					if(nodes[handle].next != end) {
						nodes[nodes[handle].next].prev = nodes[handle].prev;
					}
					if(nodes[handle].prev != end) {
						nodes[nodes[handle].prev].next = nodes[handle].next;
					}


					// Insert this element at the free head
					if(freeHead != end) {
						nodes[freeHead].prev = handle;
					}
					nodes[handle].next = freeHead;
					nodes[handle].prev = end;
					freeHead = handle;
			

					// Deconstruct the object
					data[handle].~T();
				}
			}
	
			inline T *Get(Handle handle) const
			{
	#if _DEBUG
				if(handle < capacity) {
					assert(referenceCounts[handle] > 0);
					return &data[handle];
				} else {
					return nullptr;
				}
	#endif
				return &data[handle];			
			}

			// Fnds out if item is n this list, and returns its index (or -1 if not found).
			// Note: this method adds a reference to the item if it is found.
			Handle Find(const T &item)
			{
				const Iterator iterEnd = End();
				for(Iterator iter = Begin(); iter != iterEnd; ++iter) {
					if(item == *iter) {
						assert(referenceCounts[iter.current] > 0);
						// increase the ref count for this object
						referenceCounts[iter.current]++;
						return iter.current;
					}
				}
				return HANDLE_NONE;
			}

			// Returns a handle to the first item for which predicate is true.
			// Note: this method adds a reference to the item if it is found.
			Handle Match(std::function<bool(const T *)> predicate)
			{
				const Iterator iterEnd = End();
				for(Iterator iter = Begin(); iter != iterEnd; ++iter) {
					if(predicate(iter.Ptr())) {
						assert(referenceCounts[iter.current] > 0);
						// increase the ref count for this object
						referenceCounts[iter.current]++;
						return iter.current;
					}
				}
				return HANDLE_NONE;
			}

			inline void AddRef(Handle handle)
			{
				if(handle < capacity) {
					assert(referenceCounts[handle] > 0);
					referenceCounts[handle]++;
				}
			}

			inline uint16 GetRefCount(Handle handle)
			{
				if(handle < capacity) {
					assert(referenceCounts[handle] > 0);
					return referenceCounts[handle];
				}
				return 0;
			}

			inline uint32 GetCapacity() const { return capacity; }
			inline uint32 GetSize() const { return count; }
			inline const T *GetBaseAddr() const { return data; }

		private:
			// The array of data
			T *data;

			// Array of lnked nodes
			Node *nodes;
			uint32 head;
			uint32 freeHead;
			uint32 end;

			// The reference count for each object
			uint16 *referenceCounts;

			uint32 count;
			uint32 capacity;

#if _DEBUG
		public:
			std::string debugName;
#endif
		};

	} // namespace Core

} // namespace Maki
