#pragma once
#include "core/MakiTypes.h"
#include "core/MakiVector4.h"
#include "core/MakiVector3.h"

namespace maki {
	namespace core {

		class bounding_sphere_t {
		public:
			bounding_sphere_t() = default;
			
			bounding_sphere_t(const vector4_t &pos, float radius)
				: empty(false), pos(pos), radius(radius) {
			}
			
			bounding_sphere_t(const vector3_t &pos, float radius)
				: empty(false), pos(pos.x, pos.y, pos.z, 1.0f), radius(radius) {
			}

			inline void merge(const bounding_sphere_t &other) {
				if(other.empty)
					return;

				if(empty) {
					pos = other.pos;
					radius = other.radius;
					empty = false;
					return;
				}

				vector4_t dir = other.pos - pos;
				float dist = dir.length();
				if(dist < 0.000001f) {
					if(other.radius > radius)
						radius = other.radius;
				} else {
					dir /= dist;
					vector4_t min_point = dir * -radius;
					vector4_t max_point = dir * (dist + other.radius);
					pos = (min_point + max_point) / 2.0f;
					radius = (radius + dist + other.radius) / 2.0f;
				}
			}

			inline void merge(const vector4_t &p) {
				if(empty) {
					pos = p;
					empty = false;
					return;
				}
			
				vector4_t dir = p - pos;
				float dist = dir.length();
				if(dist < 0.000001f) {
					if(dist > radius)
						radius = dist;
				} else {
					dir /= dist;
					vector4_t min_point = dir * -radius;
					vector4_t max_point = dir * dist;
					pos = (min_point + max_point) / 2.0f;
					radius = (radius + dist) / 2.0f;
				}
			}

			inline void reset() {
				pos = vector4_t(0.0f);
				radius = 0.0f;
				empty = true;
			}

		public:
			vector4_t pos;
			float radius = 0.0;
			bool empty = true;
		};

	} // namespace core
} // namespace maki
