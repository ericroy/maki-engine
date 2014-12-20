#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		class time_source_t
		{
		public:
			time_source_t();
			virtual ~time_source_t();

			uint64 get_time_micro();

		private:
			double frequency_;
			uint64 start_;
		};


	} // namespace core

} // namespace maki
