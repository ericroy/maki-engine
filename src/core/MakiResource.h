#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{

		class MAKI_CORE_API Resource
		{
		public:
			template<typename T>
			struct MAKI_CORE_API FindPredicate : public std::unary_function<const T *, bool>
			{
				FindPredicate(Rid rid) : rid(rid) {}
				inline bool operator()(const T *res) const
				{
					return res->rid == rid;
				}
				Rid rid;
			};

		public:
			Resource() : rid(RID_NONE) {}
			Resource(const MoveToken<Resource> &other);
			explicit Resource(const Resource &other) { rid = other.rid; }
			virtual ~Resource() {}

			inline bool operator==(const Resource &other) const { return rid == other.rid; }

		public:
			// Resource id
			// This is an index into the data in the programmatically generated ResourceLibrary class
			Rid rid;
		};


	} // namespace Core

} // namespace Maki