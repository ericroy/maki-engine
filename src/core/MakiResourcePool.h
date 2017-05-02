#pragma once
#include <functional>
#include "core/MakiTypes.h"

namespace maki {
	namespace core {

		template<class T> class resource_pool_t;

		template<class T> class ref_t {
			friend class resource_pool_t<T>;
		
		private:
			ref_t(resource_pool_t<T> *pool, handle_t handle) : pool_(pool), handle_(handle) {}
		
		public:
			ref_t(decltype(nullptr)) {}
			ref_t() {}
			ref_t(ref_t<T> &&other) : pool_(other.pool_), handle_(other.handle_) {
				other.handle_ = HANDLE_NONE;
			}
			ref_t(const ref_t<T> &other) : pool_(other.pool_), handle_(other.handle_) {
				if (handle_ != HANDLE_NONE) {
					MAKI_ASSERT(pool_->reference_counts_[handle_] < REF_COUNT_MAX);
					pool_->reference_counts_[handle_]++;
				}
			}
			~ref_t() {
				release();
			}
			inline void operator=(const ref_t<T> &other) {
				if (&other != this) {
					release();
					pool_ = other.pool_;
					handle_ = other.handle_;
					if (handle_ != HANDLE_NONE) {
						MAKI_ASSERT(pool_->reference_counts_[handle_] < REF_COUNT_MAX);
						pool_->reference_counts_[handle_]++;
					}
				}
			}
			inline void operator=(ref_t<T> &&other) {
				if (&other != this) {
					release();
					pool_ = other.pool_;
					handle_ = other.handle_;
					other.handle_ = HANDLE_NONE;
				}
			}
			inline T &operator*() {
				MAKI_ASSERT(handle_ != HANDLE_NONE);
				return pool_->data_[handle_];
			}
			inline const T &operator*() const {
				MAKI_ASSERT(handle_ != HANDLE_NONE);
				return pool_->data_[handle_];
			}
			inline T *operator->() {
				MAKI_ASSERT(handle_ != HANDLE_NONE);
				return &pool_->data_[handle_];
			}
			inline const T *operator->() const {
				MAKI_ASSERT(handle_ != HANDLE_NONE);
				return &pool_->data_[handle_];
			}
			inline T *ptr() const {
				MAKI_ASSERT(handle_ != HANDLE_NONE);
				return &pool_->data_[handle_];
			}
			inline void release() {
				if (handle_ != HANDLE_NONE) {
					pool_->free(handle_);
					handle_ = HANDLE_NONE;
				}
			}
			inline ref_count_t ref_count() const {
				return (pool_ != nullptr && handle_ != HANDLE_NONE) ? pool_->reference_counts_[handle_] : 0;
			}
			inline handle_t handle() const {
				return handle_;
			}
			inline operator bool() const {
				return handle_ != HANDLE_NONE;
			}
			inline bool operator==(const ref_t<T> &other) const {
				return pool_ == other.pool_ && handle_ == other.handle_;
			}
			inline bool operator!=(const ref_t<T> &other) const {
				return pool_ != other.pool_ || handle_ != other.handle_;
			}
		private:
			resource_pool_t<T> *pool_ = nullptr;
			handle_t handle_ = HANDLE_NONE;
		};


		/*
		This class manages an array of objects.  Objects in the array
		are preallocated but constructors are called as they are requested,
		and destructors are called as they are freed. All the managed objects
		are reference counted, so they will only be destroyed if their reference
		count falls to zero.  References are increased whenever a handle is given
		out, and decreased whenever a handle is freed.
		*/
		template<class T> class resource_pool_t {
			friend class ref_t<T>;

		private:
			struct node_t {
				uint32_t next, prev;
			};

		public:
			class iterator_t {
				friend class resource_pool_t<T>;
			public:
				inline void operator++() { handle_ = pool_->nodes_[handle_].next; }
				inline bool operator==(const iterator_t &iter) const { return iter.handle_ == handle_; }
				inline bool operator!=(const iterator_t &iter) const { return iter.handle_ != handle_; }
				inline ref_t operator*() const { return ref_t(pool_, handle_); }
			private:
				iterator_t(resource_pool_t<T> *pool, handle_t handle) : pool_(pool), handle_(handle) {}
			private:
				resource_pool_t<T> *pool_;
				handle_t handle_;
			};

		public:
			resource_pool_t(uint32_t max_size, const char *debug_name)
				: data_(0), count_(0) {

				capacity_ = max_size;

				data_ = (T*)allocator_t::malloc(sizeof(T) * capacity_, std::alignment_of<T>::value);
				reference_counts_ = (ref_count_t *)allocator_t::malloc(sizeof(ref_count_t) * capacity_, sizeof(ref_count_t));
		
				// Allocate one extra node as the "end" element
				nodes_ = (node_t *)allocator_t::malloc(sizeof(node_t) * (capacity_ + 1), sizeof(node_t));

				// Link up the nodes_
				for(uint32_t i = 0; i < capacity_; i++) {
					nodes_[i].next = i + 1;
					nodes_[i].prev = i - 1;
				}

				// First element's prev ponts to end_
				nodes_[0].prev = capacity_;
		
				// End element's next and prev point to itself
				end_ = capacity_;
				nodes_[end_].next = nodes_[end_].prev = capacity_;
		
				head_ = end_;
				free_head_ = 0;

				MAKI_ASSERT(data_ && nodes_ && reference_counts_);	
				memset(static_cast<void *>(data_), 0, capacity_ * sizeof(T));
				memset(reference_counts_, 0, capacity_ * sizeof(ref_count_t));
			}

			~resource_pool_t() {
#ifdef _DEBUG
				for(uint32_t i = 0; i < count_; i++) {
					if(reference_counts_[i] > 0)
						console_t::warning("~resource_pool_t() \"%s\": warning, item at index=%u still allocated. (refcount=%u)", name_.c_str(), i, reference_counts_[i]);
				}
#endif
				MAKI_SAFE_FREE(data_);
				MAKI_SAFE_DELETE_ARRAY(nodes_);
				MAKI_SAFE_DELETE_ARRAY(reference_counts_);
				end_ = head_ = free_head_ = -1;
			}

			inline ref_t<T> alloc(T &&value) {
				handle_t handle = alloc_inner();
				MAKI_ASSERT(handle != HANDLE_NONE);
				T *mem = &data_[handle];
				MAKI_ASSERT(mem);
				// Use placement new to call move constructor
				new(mem) T(value);
				return ref_t<T>(this, handle);
			}

			inline ref_t<T> alloc(const T &value) {
				handle_t handle = alloc_inner();
				MAKI_ASSERT(handle != HANDLE_NONE);
				T *mem = &data_[handle];
				MAKI_ASSERT(mem);
				// Use placement new to call copy constructor
				new(mem) T(value);
				return ref_t<T>(this, handle);
			}

			inline ref_t<T> alloc() {
				handle_t handle = alloc_inner();
				MAKI_ASSERT(handle != HANDLE_NONE);
				T *mem = &data_[handle];
				MAKI_ASSERT(mem);
				// Use placement new to call default constructor
				new(mem) T();
				return ref_t<T>(this, handle);
			}

			inline iterator_t begin() const { return iterator_t(this, head_); }
			inline const iterator_t end() const { return iterator_t(this, end_); }
			inline size_t size() const { return count_; }
			inline size_t capacity() const { return capacity_; }

			template<class Predicate>
			ref_t<T> find(Predicate pred) {
				for (auto handle = head_; handle != end_; handle = nodes_[handle].next) {
					MAKI_ASSERT(reference_counts_[handle] > 0);
					if (pred(data_[handle])) {
						MAKI_ASSERT(reference_counts_[handle] < REF_COUNT_MAX);
						reference_counts_[handle]++;
						return ref_t<T>(this, handle);
					}
				}
				return nullptr;
			}

			const ::std::string &name() const { return name_; }

		private:
			handle_t alloc_inner() {
				handle_t handle = HANDLE_NONE;
				if(free_head_ != end_) {
					handle = free_head_;

					// maki_move the free head_ to the next free element
					free_head_ = nodes_[free_head_].next;

					// insert the new element at the head_
					uint32_t old_head = head_;
					head_ = handle;
					if(old_head != end_)
						nodes_[old_head].prev = head_;
					nodes_[head_].next = old_head;
					nodes_[head_].prev = end_;

					// Adjust ref counts
					MAKI_ASSERT(reference_counts_[handle] == 0);
					reference_counts_[handle]++;

					count_++;
				} else {
					MAKI_ASSERT(false && "resource_t pool depleted");
				}
				return handle;
			}

			void free(handle_t handle) {
				MAKI_ASSERT(handle < capacity_);
				MAKI_ASSERT(reference_counts_[handle] > 0);
		
				// Remove a reference from this item
				reference_counts_[handle]--;

				// Actually mark this object as deallocated if nobody else has a ref to it
				if(reference_counts_[handle] == 0) {
					count_--;
			
					// Remove this element from allocated list
					if(handle == head_)
						head_ = nodes_[handle].next;
					if(nodes_[handle].next != end_)
						nodes_[nodes_[handle].next].prev = nodes_[handle].prev;
					if(nodes_[handle].prev != end_)
						nodes_[nodes_[handle].prev].next = nodes_[handle].next;

					// Insert this element at the free head_
					if(free_head_ != end_)
						nodes_[free_head_].prev = handle;
					nodes_[handle].next = free_head_;
					nodes_[handle].prev = end_;
					free_head_ = handle;

					// Deconstruct the object
					data_[handle].~T();
				}
			}

		private:
			// The array of data_
			T *data_;

			// array_t of lnked nodes_
			node_t *nodes_;
			uint32_t head_;
			uint32_t free_head_;
			uint32_t end_;

			// The reference count for each object
			ref_count_t *reference_counts_;

			uint32_t count_;
			uint32_t capacity_;

			::std::string name_;
		};

	} // namespace core
} // namespace maki
