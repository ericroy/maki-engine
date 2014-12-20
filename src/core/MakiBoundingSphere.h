#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		class bounding_sphere_t
		{
		public:
			bounding_sphere_t()
				: empty_(true), pos_(0.0f), radius_(0.0f)
			{
			}
			bounding_sphere_t(const vector4_t &pos, float radius)
				: empty_(false), pos_(pos), radius_(radius)
			{
			}
			bounding_sphere_t(const Vector3 &pos, float radius)
				: empty_(false), pos_(pos.x, pos.y, pos.z, 1.0f), radius_(radius)
			{
			}

			inline void merge(const bounding_sphere_t &other)
			{
				if(other.empty_) {
					return;
				}
				if(empty_) {
					pos_ = other.pos_;
					radius_ = other.radius_;
					empty_ = false;
					return;
				}

				vector4_t dir = other.pos_ - pos_;
				float dist = dir.length();
				if(dist < 0.000001f) {
					if(other.radius_ > radius_) {
						radius_ = other.radius_;
					}
				} else {
					dir /= dist;
					vector4_t min_point = dir * -radius_;
					vector4_t max_point = dir * (dist + other.radius_);
					pos_ = (min_point + max_point) / 2.0f;
					radius_ = (radius_ + dist + other.radius_) / 2.0f;
				}
			}

			inline void merge(const vector4_t &p)
			{
				if(empty_) {
					pos_ = p;
					empty_ = false;
					return;
				}
			
				vector4_t dir = p - pos_;
				float dist = dir.length();
				if(dist < 0.000001f) {
					if(dist > radius_) {
						radius_ = dist;
					}
				} else {
					dir /= dist;
					vector4_t min_point = dir * -radius_;
					vector4_t max_point = dir * dist;
					pos_ = (min_point + max_point) / 2.0f;
					radius_ = (radius_ + dist) / 2.0f;
				}
			}

			inline void reset()
			{
				pos_ = vector4_t(0.0f);
				radius_ = 0.0f;
				empty_ = true;
			}

		public:
			vector4_t pos_;
			float radius_;
			bool empty_;
		};

	} // namespace core

} // namespace maki
