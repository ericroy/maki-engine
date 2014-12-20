#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		class resource_t
		{
		public:
			template<typename T>
			struct find_predicate_t : public std::unary_function<const T *, bool>
			{
				find_predicate_t(rid_t rid) : rid(rid) {}
				inline bool operator()(const T *res) const
				{
					return res->rid == rid;
				}
				rid_t rid;
			};

		public:
			resource_t() : rid_(RID_NONE) {}
			resource_t(const move_token_t<resource_t> &other);
			explicit resource_t(const resource_t &other) { rid_ = other.rid_; }
			virtual ~resource_t() {}

			inline bool operator==(const resource_t &other) const { return rid_ == other.rid_; }

		public:
			// resource_t id
			// This is an index into the data in the programmatically generated resource_library_t class
			rid_t rid_;
		};


	} // namespace core

} // namespace maki
