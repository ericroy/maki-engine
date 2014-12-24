#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{
	
		class bounding_box_t
		{
		private:
			static const vector4_t corner_coeffs_[8];

		public:
			bounding_box_t()
				: empty_(true), pos_(0.0f), radii_(0.0f)
			{
			}
			bounding_box_t(const vector4_t &pos, const vector3_t &radii)
				: empty_(false), pos_(pos), radii_(radii.x_, radii.y_, radii.z_, 1.0f)
			{
			}
			bounding_box_t(const vector4_t &pos, const vector4_t &radii)
				: empty_(false), pos_(pos), radii_(radii)
			{
			}

			inline void merge(const bounding_box_t &other)
			{
				if(empty_) {
					pos_ = other.pos_;
					radii_ = other.radii_;
					empty_ = other.empty_;
					return;
				}

				vector4_t max_corner(
					std::max<float>(pos_.x_+radii_.x_, other.pos_.x_+other.radii_.x_),
					std::max<float>(pos_.y_+radii_.y_, other.pos_.y_+other.radii_.y_),
					std::max<float>(pos_.z_+radii_.z_, other.pos_.z_+other.radii_.z_),
					1.0f);
				vector4_t min_corner(
					std::min<float>(pos_.x_-radii_.x_, other.pos_.x_-other.radii_.x_),
					std::min<float>(pos_.y_-radii_.y_, other.pos_.y_-other.radii_.y_),
					std::min<float>(pos_.z_-radii_.z_, other.pos_.z_-other.radii_.z_),
					1.0f);
				pos_ = (max_corner + min_corner) / 2.0f;
				radii_ = max_corner - pos_;
			}

			inline void merge(float x, float y, float z)
			{
				if(empty_) {
					pos_ = vector4_t(x, y, z, 1.0f);
					empty_ = false;
					return;
				}

				vector4_t max_corner(
					std::max<float>(pos_.x_+radii_.x_, x),
					std::max<float>(pos_.y_+radii_.y_, y),
					std::max<float>(pos_.z_+radii_.z_, z),
					1.0f);
				vector4_t min_corner(
					std::min<float>(pos_.x_-radii_.x_, x),
					std::min<float>(pos_.y_-radii_.y_, y),
					std::min<float>(pos_.z_-radii_.z_, z),
					1.0f);
				pos_ = (max_corner + min_corner) / 2.0f;
				radii_ = max_corner - pos_;
			}
			inline void merge(const vector4_t &point) { merge(point.x_, point.y_, point.z_); }
			inline void merge(const vector3_t &point) { merge(point.x_, point.y_, point.z_); }
		
			inline void merge(const vector4_t &point, float radius)
			{
				merge(point.x_+radius, point.y_+radius, point.z_+radius);
				merge(point.x_-radius, point.y_-radius, point.z_-radius);
			}

			inline void reset()
			{
				empty_ = true;
				pos_ = vector4_t(0.0f);
				radii_ = vector4_t(0.0f);
			}

			inline vector4_t get_max_corner() const { return pos_ + radii_; }
			inline vector4_t get_min_corner() const { return pos_ - radii_; }
			inline vector4_t get_corner(uint32 i) const
			{
				assert(i < 8);
				return pos_ + corner_coeffs_[i] * radii_;
			}

			inline float get_radius() const { return radii_.length(); }
			inline float get_radius_sq() const { return radii_.length_squared(); }

		public:
			vector4_t pos_;
			vector4_t radii_;
			bool empty_;
		};


		inline bounding_box_t operator*(float scale, const bounding_box_t &bounds)
		{
			return bounding_box_t(bounds.pos_*scale, bounds.radii_*scale);
		}


	} // namespace core

} // namespace maki
