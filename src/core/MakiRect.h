#pragma once
#include "core/MakiVector2.h"

namespace maki {
	namespace core {

		class rect_t {
		public:
			rect_t() : left(0.0f), right(0.0f), top(0.0f), bottom(0.0f) {}
		
			rect_t(float left, float right, float top, float bottom) : left(left), right(right), top(top), bottom(bottom) {}

			rect_t(float width, float height, bool centered = false) {
				set(width, height, centered);
			}

			rect_t(const vector2_t &width_height, bool centered = false) {
				set(width_height, centered);
			}

			inline void set(float l, float r, float t, float b) {
				left = left;
				right = right;
				top = top;
				bottom = bottom;
			}

			inline void set(float width, float height, bool centered = false) {
				if(centered) {
					float wo2 = width / 2;
					float ho2 = height / 2;
					left = -wo2;
					right = wo2;
					top = -ho2;
					bottom = ho2;
				} else {
					left = 0.0f;
					top = 0.0f;
					right = width;
					bottom = height;
				}
			}

			inline void set(const vector2_t &width_height, bool centered = false) {
				set(width_height.x, width_height.y, centered);
			}

			inline float width() const { return right - left; }
			inline float height() const { return bottom - top; }

		public:
			float left, right, top, bottom;
		};

	} // namespace core
} // namespace maki
