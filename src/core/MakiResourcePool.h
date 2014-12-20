#pragma once
#include "core/core_stdafx.h"
#include <functional>

namespace maki
{
	namespace core
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
		class resource_pool_t
		{
		private:
			struct node_t
			{
				uint32 next_, prev_;
			};

		public:
			class iterator_t
			{
				friend class resource_pool_t<T>;
			public:
				inline void operator++() { current_ = nodes_[current_].next_; }
				inline bool operator==(const iterator_t &iter) const { return iter.current_ == current_; }
				inline bool operator!=(const iterator_t &iter) const { return iter.current_ != current_; }
				inline const T &operator*() const { return data_[current_]; }
				inline T *ptr() const { return &data_[current_]; }
				inline uint32 index() const { return current_; }
				inline uint16 ref_count() const { return ref_counts_[current_]; }
		
			private:
				iterator_t(node_t *nodes, T *data, uint16 *ref_counts, uint32 current)
				:	nodes_(nodes), data_(data), ref_counts_(ref_counts), current_(current) {}

			private:
				node_t *nodes_;
				T *data_;
				uint16 *ref_counts_;
				uint32 current_;
			};

			inline iterator_t begin() const { return iterator_t(nodes_, data_, reference_counts_, head_); }
			inline const iterator_t end_() const { return iterator_t(nodes_, data_, reference_counts_, end_); }

		public:
			resource_pool_t(uint32 max_size, const char *debug_name)
				: data_(0), count_(0)
			{

				capacity_ = max_size;

				data_ = (T*)allocator_t::malloc(sizeof(T) * capacity_, std::alignment_of<T>::value_);
				reference_counts_ = new uint16[capacity_];
		
				// Allocate one extra node as the "end_" element
				nodes_ = new node_t[capacity_+1];

				// Link up the nodes_
				for(uint32 i = 0; i < capacity_; i++)
				{
					nodes_[i].next_ = i+1;
					nodes_[i].prev_ = i-1;
				}

				// First element's prev ponts to end_
				nodes_[0].prev_ = capacity_;
		
				// end_ element's next and prev point to itself
				end_ = capacity_;
				nodes_[end_].next_ = nodes_[end_].prev_ = capacity_;
		
				head_ = end_;
				free_head_ = 0;


				assert(data_ && nodes_ && reference_counts_);	
				memset(static_cast<void *>(data_), 0, capacity_*sizeof(T));
				memset(reference_counts_, 0, capacity_*sizeof(uint16));

#if _DEBUG
				debug_name_ = debug_name;
#endif
			}

			~resource_pool_t()
			{
	#ifdef _DEBUG
				for(uint32 i = 0; i < count_; i++) {
					if(reference_counts_[i] > 0) {
						console_t::warning("~resource_pool_t() \"%s\": warning, item at index=%u still allocated. (refcount=%d)", debug_name_.c_str(), i, reference_counts_[i]);
					}
				}
	#endif
				if(data_ != nullptr) {
					allocator_t::free(data_);
				}
				MAKI_SAFE_DELETE_ARRAY(nodes_);
				MAKI_SAFE_DELETE_ARRAY(reference_counts_);
				end_ = head_ = free_head_ = -1;
			}

			inline handle_t alloc(const move_token_t<T> &value)
			{
				handle_t handle = alloc();
				assert(handle != HANDLE_NONE);
				T *mem = get(handle);
				assert(mem);
				// Use placement new to call move constructor
				new(mem) T(value);
				return handle;
			}

			inline handle_t alloc(const T &value)
			{
				handle_t handle = alloc();
				assert(handle != HANDLE_NONE);
				T *mem = get(handle);
				assert(mem);
				// Use placement new to call copy constructor
				new(mem) T(value);
				return handle;
			}

			handle_t alloc()
			{
				handle_t handle = HANDLE_NONE;
				if(free_head_ != end_) {
					handle = free_head_;

					// maki_move the free head_ to the next free element
					free_head_ = nodes_[free_head_].next_;

					// insert the new element at the head_
					uint32 oldHead = head_;
					head_ = handle;
					if(oldHead != end_) {
						nodes_[oldHead].prev_ = head_;
					}
					nodes_[head_].next_ = oldHead;
					nodes_[head_].prev_ = end_;

					// Adjust ref counts
					assert(reference_counts_[handle] == 0);
					reference_counts_[handle]++;

					count_++;
				} else {
					assert(false && "resource_t pool depleted");
				}
				return handle;
			}

			void free(handle_t handle)
			{
				assert(handle < capacity_);
				assert(reference_counts_[handle] > 0);
		
				// Remove a reference from this item
				reference_counts_[handle]--;

				// Actually mark this object as deallocated if nobody else has a ref to it
				if(reference_counts_[handle] == 0) {
					count_--;
			
					// Remove this element from allocated list
					if(handle == head_) {
						head_ = nodes_[handle].next_;
					}
					if(nodes_[handle].next_ != end_) {
						nodes_[nodes_[handle].next_].prev_ = nodes_[handle].prev_;
					}
					if(nodes_[handle].prev_ != end_) {
						nodes_[nodes_[handle].prev_].next_ = nodes_[handle].next_;
					}


					// Insert this element at the free head_
					if(free_head_ != end_) {
						nodes_[free_head_].prev_ = handle;
					}
					nodes_[handle].next_ = free_head_;
					nodes_[handle].prev_ = end_;
					free_head_ = handle;
			

					// Deconstruct the object
					data_[handle].~T();
				}
			}
	
			inline T *get(handle_t handle) const
			{
	#if _DEBUG
				if(handle < capacity_) {
					assert(reference_counts_[handle] > 0);
					return &data_[handle];
				} else {
					return nullptr;
				}
	#endif
				return &data_[handle];			
			}

			// Fnds out if item is n this list, and returns its index (or -1 if not found).
			// Note: this method adds a reference to the item if it is found.
			handle_t find(const T &item)
			{
				const iterator_t iter_end = end_();
				for(iterator_t iter = begin(); iter != iter_end; ++iter) {
					if(item == *iter) {
						assert(reference_counts_[iter.current_] > 0);
						// increase the ref count for this object
						reference_counts_[iter.current_]++;
						return iter.current_;
					}
				}
				return HANDLE_NONE;
			}

			// Returns a handle to the first item for which predicate is true.
			// Note: this method adds a reference to the item if it is found.
			handle_t match(std::function<bool(const T *)> predicate)
			{
				const iterator_t iter_end = end_();
				for(iterator_t iter = begin(); iter != iter_end; ++iter) {
					if(predicate(iter.ptr())) {
						assert(reference_counts_[iter.current_] > 0);
						// increase the ref count for this object
						reference_counts_[iter.current_]++;
						return iter.current_;
					}
				}
				return HANDLE_NONE;
			}

			inline void add_ref(handle_t handle)
			{
				if(handle < capacity_) {
					assert(reference_counts_[handle] > 0);
					reference_counts_[handle]++;
				}
			}

			inline uint16 get_ref_count(handle_t handle)
			{
				if(handle < capacity_) {
					assert(reference_counts_[handle] > 0);
					return reference_counts_[handle];
				}
				return 0;
			}

			inline uint32 get_capacity() const { return capacity_; }
			inline uint32 get_size() const { return count_; }
			inline const T *get_base_addr() const { return data_; }

		private:
			// The array of data_
			T *data_;

			// array_t of lnked nodes_
			node_t *nodes_;
			uint32 head_;
			uint32 free_head_;
			uint32 end_;

			// The reference count for each object
			uint16 *reference_counts_;

			uint32 count_;
			uint32 capacity_;

#if _DEBUG
		public:
			std::string debug_name_;
#endif
		};

	} // namespace core

} // namespace maki
