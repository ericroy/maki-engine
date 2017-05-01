#pragma once
#include <algorithm>
#include "core/MakiTypes.h"
#include "core/MakiVector4.h"
#include "core/MakiVector3.h"

namespace maki {
	namespace core {
	
		class bounding_box_t {
		private:
			static const vector4_t corner_coeffs_[8];

		public:
			bounding_box_t() = default;
			bounding_box_t(const vector4_t &pos, const vector3_t &radii) : empty(false), pos(pos), radii(radii.x, radii.y, radii.z, 1.0f) {}
			bounding_box_t(const vector4_t &pos, const vector4_t &radii) : empty(false), pos(pos), radii(radii) {}

			inline void merge(const bounding_box_t &other) {
				if(empty) {
					pos = other.pos;
					radii = other.radii;
					empty = other.empty;
					return;
				}

				vector4_t max_corner(
					::std::max<float>(pos.x + radii.x, other.pos.x + other.radii.x),
					::std::max<float>(pos.y + radii.y, other.pos.y + other.radii.y),
					::std::max<float>(pos.z + radii.z, other.pos.z + other.radii.z),
					1.0f);
				vector4_t min_corner(
					::std::min<float>(pos.x - radii.x, other.pos.x - other.radii.x),
					::std::min<float>(pos.y - radii.y, other.pos.y - other.radii.y),
					::std::min<float>(pos.z - radii.z, other.pos.z - other.radii.z),
					1.0f);
				pos = (max_corner + min_corner) / 2.0f;
				radii = max_corner - pos;
			}

			inline void merge(float x, float y, float z) {
				if(empty) {
					pos = vector4_t(x, y, z, 1.0f);
					empty = false;
					return;
				}

				vector4_t max_corner(
					::std::max<float>(pos.x + radii.x, x),
					::std::max<float>(pos.y + radii.y, y),
					::std::max<float>(pos.z + radii.z, z),
					1.0f);
				vector4_t min_corner(
					::std::min<float>(pos.x - radii.x, x),
					::std::min<float>(pos.y - radii.y, y),
					::std::min<float>(pos.z - radii.z, z),
					1.0f);
				pos = (max_corner + min_corner) / 2.0f;
				radii = max_corner - pos;
			}
			inline void merge(const vector4_t &point) { merge(point.x, point.y, point.z); }
			inline void merge(const vector3_t &point) { merge(point.x, point.y, point.z); }
		
			inline void merge(const vector4_t &point, float radius) {
				merge(point.x + radius, point.y + radius, point.z + radius);
				merge(point.x - radius, point.y - radius, point.z - radius);
			}

			inline void reset() {
				empty = true;
				pos = vector4_t(0.0f);
				radii = vector4_t(0.0f);
			}

			inline vector4_t get_max_corner() const { return pos + radii; }
			inline vector4_t get_min_corner() const { return pos - radii; }
			inline vector4_t get_corner(uint32_t i) const {
				assert(i < 8);
				return pos + corner_coeffs_[i] * radii;
			}

			inline float get_radius() const { return radii.length(); }
			inline float get_radius_sq() const { return radii.length_squared(); }

		public:
			vector4_t pos;
			vector4_t radii;
			bool empty = true;
		};


		inline bounding_box_t operator*(float scale, const bounding_box_t &bounds) {
			return bounding_box_t(bounds.pos*scale, bounds.radii*scale);
		}


	} // namespace core
} // namespace maki
