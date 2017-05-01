#pragma once
#include "core/MakiMacros.h"
#include "core/MakiVector4.h"

namespace maki {
	namespace core {
		class matrix44_t;

		class vector3_t {
		public:
			static const vector3_t unit_x;
			static const vector3_t unit_y;
			static const vector3_t unit_z;

		public:
			inline vector3_t() {}
			inline vector3_t(float xyz) : x(xyz), y(xyz), z(xyz) {}
			inline vector3_t(float x, float y, float z) : x(x), y(y), z(z) {}
			inline vector3_t(const vector4_t &v) : x(v.x), y(v.y), z(v.z) {}

			// Member vector operations

			inline float length() const { return length(*this); }
			inline float length_squared() const { return length_squared(*this); }
			inline float dot(const vector3_t &v) const { return dot(*this, v); }
			inline vector3_t cross(const vector3_t &rhs) const { return cross(*this, rhs); }
			inline void normalize() { normalize(*this); }

			// Operator overloads
			inline float operator*(const vector3_t &v) const { return x * v.x + y * v.y + z * v.z; }

			inline vector3_t operator*(float s) const { return vector3_t(x*s, y*s, z*s); }
			inline void operator*=(float s) { x *= s; y *= s; z *= s; }

			inline vector3_t operator/(float s) const { MAKI_ASSERT(s != 0); return vector3_t(x / s, y / s, z / s); }
			inline void operator/=(float s) { MAKI_ASSERT(s != 0); x /= s; y /= s; z /= s; }

			inline vector3_t operator-(const vector3_t &v) const { return vector3_t(x - v.x, y - v.y, z - v.z); }
			inline void operator-=(const vector3_t &v) { x -= v.x; y -= v.y; z -= v.z; }

			inline vector3_t operator+(const vector3_t &v) const { return vector3_t(x + v.x, y + v.y, z + v.z); }
			inline void operator+=(const vector3_t &v) { x += v.x; y += v.y; z += v.z; }

			inline vector3_t operator-() const { return vector3_t(-x, -y, -z); }

			inline bool operator==(const vector3_t &v) { return x == v.x && y == v.y && z == v.z; }
			inline bool operator!=(const vector3_t &v) { return x != v.x || y != v.y || z != v.z; }

			// Static methods
			static inline float dot(const vector3_t &v1, const vector3_t &v2);
			static inline void normalize(vector3_t &v);
			static inline vector3_t cross(const vector3_t &a, const vector3_t &b);
			static inline float length(const vector3_t &v);
			static inline float length_squared(const vector3_t &v);

		public:
			union {
				struct {
					float x, y, z;
				};
				float vals[3];
			};
		};

		float vector3_t::dot(const vector3_t &v1, const vector3_t &v2) {
			return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
		}

		void vector3_t::normalize(vector3_t &v) {
			float inv_len = 1.f / sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
			v *= inv_len;
		}

		vector3_t vector3_t::cross(const vector3_t &a, const vector3_t &b) {
			return vector3_t(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
		}
	
		float vector3_t::length(const vector3_t &v) {
			return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
		}

		float vector3_t::length_squared(const vector3_t &v) {
			return v.x * v.x + v.y * v.y + v.z * v.z;
		}


	} // namespace core
} // namespace maki
