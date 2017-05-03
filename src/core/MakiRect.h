#pragma once
#include "core/MakiVector2.h"

namespace maki {
	namespace core {

		class rect_t {
		public:
			rect_t() = default;
			rect_t(float left, float right, float top, float bottom) : left_(left), right_(right), top_(top), bottom_(bottom) {}

			rect_t(float width, float height, bool centered = false) {
				set(width, height, centered);
			}

			rect_t(const vector2_t &width_height, bool centered = false) {
				set(width_height, centered);
			}

			inline void set(float left, float right, float top, float bottom) {
				left_ = left;
				right_ = right;
				top_ = top;
				bottom_ = bottom;
			}

			inline void set(float width, float height, bool centered = false) {
				if(centered) {
					float wo2 = width / 2;
					float ho2 = height / 2;
					left_ = -wo2;
					right_ = wo2;
					top_ = -ho2;
					bottom_ = ho2;
				} else {
					left_ = 0.0f;
					top_ = 0.0f;
					right_ = width;
					bottom_ = height;
				}
			}

			inline void set(const vector2_t &width_height, bool centered = false) {
				set(width_height.x, width_height.y, centered);
			}

			inline float width() const { return right_ - left_; }
			inline float height() const { return bottom_ - top_; }
			inline float left() const { return left_; }
			inline float right() const { return right_; }
			inline float top() const { return top_; }
			inline float bottom() const { return bottom_; }

		private:
			float left_ = 0;
			float right_ = 0;
			float top_ = 0;
			float bottom_ = 0;
		};

	} // namespace core
} // namespace maki
