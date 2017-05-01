#pragma once
#include "core/MakiMacros.h"
#include "core/MakiTypes.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		template<class T, class SUBCLASS>
		class manager_t {
		public:
			static inline SUBCLASS *get_owner(handle_t handle) {
				return managers_[(handle & manager_id_mask) >> manager_id_shift];
			}

			static inline T *get(handle_t handle) {
				if(handle == HANDLE_NONE)
					return nullptr;
				return managers_[handle >> manager_id_shift]->res_pool_->get(handle & handle_value_mask);
			}

			static inline void add_ref(handle_t handle) {
				if(handle == HANDLE_NONE)
					return;
				managers_[handle >> manager_id_shift]->res_pool_->add_ref(handle & handle_value_mask);
			}

			static inline void free(handle_t &handle) {
				if(handle != HANDLE_NONE) {
					managers_[handle >> manager_id_shift]->res_pool_->free(handle & handle_value_mask);
					handle = HANDLE_NONE;
				}
			}

			static inline void free(uint64_t count, handle_t *handles) {
				for(uint64_t i = 0; i < count; i++)
					free(handles[i]);
			}

		public:
			static const uint64_t bits_per_manager_id = 3;
		
			// Must subtract one here, because we can't have a manager id that is all ones.
			// If such a manager id was paired with a maximum resource index, then the
			// resulting (valid) handle would be equal to HANDLE_NONE.  Can't have that.
			static const uint64_t max_managers_per_resource_type = (1 << bits_per_manager_id) - 1;
			static const uint64_t manager_id_shift = (sizeof(handle_t) * 8) - bits_per_manager_id;
			static const uint64_t manager_id_mask = max_managers_per_resource_type << manager_id_shift;
			static const uint64_t handle_value_mask = ~manager_id_mask;
		
		private:
			static SUBCLASS *managers_[max_managers_per_resource_type];
		
		public:
			manager_t(uint64_t size, const char *debug_name) {
				MAKI_ASSERT(size <= (1 << manager_id_shift) - 1 && "Cannot create a manager this large");

				// find the first unused manager id and mark it as used
				manager_id_ = (uint64_t)-1;
				for(uint64_t i = 0; i < max_managers_per_resource_type_; i++) {
					if(managers_[i] == nullptr) {
						managers_[i] = static_cast<SUBCLASS *>(this);
						manager_id_ = i << manager_id_shift;
						break;
					}
				}
				MAKI_ASSERT(manager_id_ != (uint64_t)-1 && "Too many managers for this resource type");

				res_pool_ = new resource_pool_t<T>(size, debug_name);
			}

			virtual ~manager_t() {
				// Mark our manager id as available again
				managers_[manager_id_ >> manager_id_shift] = nullptr;
				MAKI_SAFE_DELETE(res_pool_);
			}

			inline uint64_t length() { return res_pool_->length(); }
			inline uint64_t capacity() { return res_pool_->capacity(); }

			// Finds an item using resource's equality operator
			// Adds a reference
			inline handle_t find(const T &item) {
				return res_pool_->find(item) | manager_id_;
			}

			// Adds an item to the resource pool using move semantics
			// Adds a reference
			inline handle_t add(T &&item) {
				return res_pool_->alloc(item) | manager_id_;
			}

			// Adds an item to the resource pool, relying on copy constructor
			// Adds a reference
			inline handle_t add(const T &item) {
				return res_pool_->alloc(item) | manager_id_;
			}

			virtual void reset() {
				uint64_t size = res_pool_->get_capacity();
#if _DEBUG
				::std::string debug_name = res_pool_->debug_name;
				MAKI_SAFE_DELETE(res_pool_);
				res_pool_ = new resource_pool_t<T>(size, debug_name.c_str());
#else
				MAKI_SAFE_DELETE(res_pool_);
				res_pool_ = new resource_pool_t<T>(size, nullptr);
#endif
			}

			void dump_stats(const char *label) {
				uint64_t size = res_pool_->size();
				uint64_t cap = res_pool_->capacity();
				console_t::info("%s: %d/%d (%d b / %d b)", label, size, cap, sizeof(T)*size, sizeof(T)*cap);
			}

			void dump_items() {
				console_t::info("manager_t items:");
				for(auto iter = ::std::begin(res_pool_); iter != ::std::end(res_pool_); ++iter)
					console_t::info("Item handle=%d refcount=%d", iter.index(), iter.ref_count());
			}

		protected:
			// A number to differentiate managers_ which hold the same type of resources
			// Each handle value has the manager id in the topmost 5 bits
			uint64_t manager_id_;

			resource_pool_t<T> *res_pool_;
		};

		template<class T, class SUBCLASS>
		SUBCLASS *manager_t<T, SUBCLASS>::managers_[max_managers_per_resource_type] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};


	} // namespace core
} // namespace maki
