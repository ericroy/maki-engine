#pragma once
#include <functional>
#include "core/MakiTypes.h"

namespace maki {
	namespace core {

		class resource_t {
		public:
			template<typename T>
			struct find_predicate_t : public ::std::unary_function<const T *, bool> {
				find_predicate_t(rid_t rid) : rid(rid) {}
				inline bool operator()(const T *res) const { return res->rid == rid; }
				rid_t rid = RID_NONE;
			};

		public:
			resource_t() = default;
			resource_t(resource_t &&other);
			explicit resource_t(const resource_t &other) { rid_ = other.rid_; }
			virtual ~resource_t() = default;
			inline rid_t rid() const { return rid_; }
			inline bool operator==(const resource_t &other) const { return rid_ == other.rid_; }

		protected:
			// Resource id
			// This is an index into the data in the programmatically generated resource_library_t class
			rid_t rid_ = RID_NONE;
		};

	} // namespace core
} // namespace maki
