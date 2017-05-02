#pragma once
#include "core/MakiTypes.h"

namespace maki {
	namespace core {

		class resource_t {
		public:
			resource_t() = default;
			resource_t(resource_t &&other);
			explicit resource_t(const resource_t &other);
			virtual ~resource_t() = default;
			
			inline rid_t rid() const {
				return rid_;
			}

			inline void set_rid(rid_t rid) {
				rid_ = rid;
			}

		protected:
			// Resource id
			// This is an index into the data in the programmatically generated resource_library_t class
			rid_t rid_ = RID_NONE;
		};

	} // namespace core
} // namespace maki
