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

			uint64 GetTimeMicro();

		private:
			double frequency;
			uint64 start;
		};


	} // namespace core

} // namespace maki
