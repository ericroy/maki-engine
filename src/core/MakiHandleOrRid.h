#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		class handle_or_rid_t
		{
		public:
			inline handle_or_rid_t() : is_handle_(true), handle_(HANDLE_NONE) {}
			inline handle_or_rid_t(handle_t h) : is_handle_(true), handle_(h) {}
			inline handle_or_rid_t(rid_t r) : is_handle_(false), rid_(r) {}

		public:
			bool is_handle_;
			union
			{
				handle_t handle_;
				rid_t rid_;
			};
		};

	} // namespace core

} // namespace maki
