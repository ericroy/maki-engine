#pragma once
#include "core/core_stdafx.h"
#include "core/MakiVector2.h"

namespace maki
{
	namespace core
	{
	
		class frustum_t
		{
		public:
			frustum_t() {}
		
			frustum_t(float left, float right, float bottom, float top, float near_plane, float far_plane, float fov = 0.0f)
				: left_(left), right_(right), bottom_(bottom), top_(top), near_plane_(near_plane), far_plane_(far_plane), fov_(fov)
			{
			}

			frustum_t(float width, float height, float near_plane, float far_plane, float fov = 0.0f)
				: left_(-width/2), right_(width/2), bottom_(-height/2), top_(height/2), near_plane_(near_plane), far_plane_(far_plane), fov_(fov)
			{
			}

			frustum_t(const vector2_t &width_height, float near_plane, float far_plane, float fov = 0.0f)
				: left_(-width_height.x_/2), right_(width_height.x_/2), bottom_(-width_height.y_/2), top_(width_height.y_/2), near_plane_(near_plane), far_plane_(far_plane), fov_(fov)
			{
			}

			inline void set(float width, float height, float near_plane, float far_plane, float fov = 0.0f)
			{
				float wo2 = width / 2.0f;
				float ho2 = height / 2.0f;
				left_ = -wo2;
				right_ = wo2;
				bottom_ = -ho2;
				top_ = ho2;
				near_plane_ = near_plane;
				far_plane_ = far_plane;
				fov_ = fov;
			}
			inline void set(const vector2_t &width_height, float near_plane, float far_plane, float fov = 0.0f) { set(width_height.x_, width_height.y_, near_plane, far_plane, fov); }
			inline void set(float width, float height) { set(width, height, near_plane_, far_plane_, fov_); }		
			inline void set(const vector2_t &width_height) { set(width_height.x_, width_height.y_, near_plane_, far_plane_, fov_); }

			inline void set(float left, float right, float bottom, float top, float near_plane, float far_plane, float fov = 0.0f)
			{
				left_ = left;
				right_ = right;
				bottom_ = bottom;
				top_ = top;
				near_plane_ = near_plane;
				far_plane_ = far_plane;
				fov_ = fov;
			}
			inline void set(float left, float right, float bottom, float top) { set(left, right, bottom, top, near_plane_, far_plane_, fov_); }


			inline float get_width() const { return right_ - left_; }
			inline float get_height() const { return top_ - bottom_; }
			inline float get_aspect() const
			{
				float h = top_ - bottom_;
				return h != 0.0f ? (right_ - left_) / h : 1.0f;
			}

			void extract_planes(vector4_t planes[6]) const
			{
				// Near, Far
				planes[0] = vector4_t(0.0f, 0.0f, 1.0f, -near_plane_); 
				planes[1] = vector4_t(0.0f, 0.0f, -1.0f, far_plane_);

				if(fov_ == 0.0f) {
					// Left, right
					planes[2] = vector4_t(-1.0f, 0.0f, 0.0f, -left_); 
					planes[3] = vector4_t(1.0f, 0.0f, 0.0f, right_);

					// Top, bottom
					planes[4] = vector4_t(0.0f, 1.0f, 0.0f, top_);
					planes[5] = vector4_t(0.0f, -1.0f, 0.0f, -bottom_);
				} else {
					float hh = far_plane_ * std::tan(fov_ * MAKI_DEG_TO_RAD / 2.0f);
					float hw = hh * get_aspect();

					vector4_t far_top_right(hw, hh, far_plane_, 1.0f);
					vector4_t far_top_left(-hw, hh, far_plane_, 1.0f);
					vector4_t far_bottom_left(-hw, -hh, far_plane_, 1.0f);
					vector4_t far_bottom_right(hw, -hh, far_plane_, 1.0f);

					// Left, right
					planes[2] = far_top_left.cross(far_bottom_left);
					planes[2].normalize();
					planes[2].w_ = 0.0f;
					planes[3] = far_bottom_right.cross(far_top_right);
					planes[3].normalize();
					planes[3].w_ = 0.0f;

					// Top, bottom
					planes[4] = far_top_right.cross(far_top_left);
					planes[4].normalize();
					planes[4].w_ = 0.0f;
					planes[5] = far_bottom_left.cross(far_bottom_right);
					planes[5].normalize();
					planes[5].w_ = 0.0f;
				}
			}

		public:
			float left_;
			float right_;
			float bottom_;
			float top_;
			float near_plane_;
			float far_plane_;
			// zero implies orthographic
			float fov_;
		};

	} // namespace core

} // namespace maki
