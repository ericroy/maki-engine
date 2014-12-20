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
			bounding_box_t(const vector4_t &pos, const Vector3 &radii)
				: empty_(false), pos_(pos), radii_(radii.x, radii.y, radii.z, 1.0f)
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
					std::max<float>(pos_.x+radii_.x, other.pos_.x+other.radii_.x),
					std::max<float>(pos_.y+radii_.y, other.pos_.y+other.radii_.y),
					std::max<float>(pos_.z+radii_.z, other.pos_.z+other.radii_.z),
					1.0f);
				vector4_t min_corner(
					std::min<float>(pos_.x-radii_.x, other.pos_.x-other.radii_.x),
					std::min<float>(pos_.y-radii_.y, other.pos_.y-other.radii_.y),
					std::min<float>(pos_.z-radii_.z, other.pos_.z-other.radii_.z),
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
					std::max<float>(pos_.x+radii_.x, x),
					std::max<float>(pos_.y+radii_.y, y),
					std::max<float>(pos_.z+radii_.z, z),
					1.0f);
				vector4_t min_corner(
					std::min<float>(pos_.x-radii_.x, x),
					std::min<float>(pos_.y-radii_.y, y),
					std::min<float>(pos_.z-radii_.z, z),
					1.0f);
				pos_ = (max_corner + min_corner) / 2.0f;
				radii_ = max_corner - pos_;
			}
			inline void merge(const vector4_t &point) { merge(point.x, point.y, point.z); }
			inline void merge(const Vector3 &point) { merge(point.x, point.y, point.z); }
		
			inline void merge(const vector4_t &point, float radius)
			{
				merge(point.x+radius, point.y+radius, point.z+radius);
				merge(point.x-radius, point.y-radius, point.z-radius);
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

			inline float GetRadius() const { return radii_.length(); }
			inline float GetRadiusSq() const { return radii_.LengthSquared(); }

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
