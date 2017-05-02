#pragma once
#include "core/MakiTypes.h"

namespace maki {
	namespace core {

		class time_source_t {
		public:
			time_source_t();
			uint64_t get_time_micro();

		private:
			double frequency_ = 0.0;
			uint64_t start_ = 0;
		};

	} // namespace core
} // namespace maki
