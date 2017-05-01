#include "core/MakiBoundingBox.h"

namespace maki {
	namespace core {

		const vector4_t bounding_box_t::corner_coeffs_[8] = {
			vector4_t(1.0f, 1.0f, 1.0f, 1.0f),
			vector4_t(-1.0f, 1.0f, 1.0f, 1.0f),
			vector4_t(-1.0f, -1.0f, 1.0f, 1.0f),
			vector4_t(1.0f, -1.0f, 1.0f, 1.0f),

			vector4_t(1.0f, 1.0f, -1.0f, 1.0f),
			vector4_t(-1.0f, 1.0f, -1.0f, 1.0f),
			vector4_t(-1.0f, -1.0f, -1.0f, 1.0f),
			vector4_t(1.0f, -1.0f, -1.0f, 1.0f),
		};

	} // namespace core
} // namespace maki
