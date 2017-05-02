#pragma once
#include "core/MakiMacros.h"

namespace maki {
	namespace core {
		class matrix44_t;

		class vector4_t {
		public:
			static const vector4_t unit_x;
			static const vector4_t unit_y;
			static const vector4_t unit_z;

		public:
			inline vector4_t() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
			inline vector4_t(float xyz) : x(xyz), y(xyz), z(xyz), w(1.0f) {}
			inline vector4_t(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
			inline vector4_t(const float *vals) : x(vals[0]), y(vals[1]), z(vals[2]), w(vals[3]) {}

			// Member vector operations
			inline float length() const { return length(*this); }
			inline float length_squared() const { return length_squared(*this); }
			inline float dot(const vector4_t &v) const { return dot(*this, v); }
			inline vector4_t cross(const vector4_t &rhs) const { return cross(*this, rhs); }
			inline void normalize() { normalize(*this); }

			// Operator overloads
			inline float operator*(const vector4_t &v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }

			inline vector4_t operator*(float s) const { return vector4_t(x * s, y * s, z * s, w * s); }
			inline void operator*=(float s) { x *= s; y *= s; z *= s; w *= s; }

			inline vector4_t operator/(float s) const { MAKI_ASSERT(s != 0); return vector4_t(x / s, y / s, z / s, w / s); }
			inline void operator/=(float s) { MAKI_ASSERT(s != 0); x /= s; y /= s; z /= s; w /= s; }

			inline vector4_t operator-(const vector4_t &v) const { return vector4_t(x - v.x, y - v.y, z - v.z, w - v.w); }
			inline void operator-=(const vector4_t &v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; }

			inline vector4_t operator+(const vector4_t &v) const { return vector4_t(x + v.x, y + v.y, z + v.z, w + v.w); }
			inline void operator+=(const vector4_t &v) { x += v.x; y += v.y; z += v.z; w += v.w; }

			inline vector4_t operator-() const { return vector4_t(-x, -y, -z, -w); }

			inline bool operator==(const vector4_t &v) { return (x == v.x && y == v.y && z == v.z && w == v.w); }
			inline bool operator!=(const vector4_t &v) { return (x != v.x || y != v.y || z != v.z || w != v.w); }

			// Static methods

			static inline float dot(const vector4_t &v1, const vector4_t &v2) { return dot3(v1, v2); }
			static inline float dot4(const vector4_t &v1, const vector4_t &v2);
			static inline float dot3(const vector4_t &v1, const vector4_t &v2);

			static inline void normalize(vector4_t &v) { return normalize3(v); }
			static inline void normalize3(vector4_t &v);
			static inline void normalize4(vector4_t &v);

			static inline vector4_t cross(const vector4_t &a, const vector4_t &b) { return cross3(a, b); }
			static inline vector4_t cross3(const vector4_t &a, const vector4_t &b);

			static inline float length(const vector4_t &v) { return length3(v); }
			static inline float length3(const vector4_t &v);

			static inline float length_squared(const vector4_t &v) { return length_squared3(v); }
			static inline float length_squared3(const vector4_t &v);

		public:
			union {
				struct {
					float x, y, z, w;
				};
				float vals[4];
			};
		};


		float vector4_t::dot4(const vector4_t &v1, const vector4_t &v2) {
			return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
		}

		float vector4_t::dot3(const vector4_t &v1, const vector4_t &v2) {
			return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
		}

		void vector4_t::normalize3(vector4_t &v) {
			float inv_len = 1.f / sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
			v *= inv_len;
		}

		void vector4_t::normalize4(vector4_t &v) {
			float inv_len = 1.f / sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
			v *= inv_len;
		}

		vector4_t vector4_t::cross3(const vector4_t &a, const vector4_t &b) {
			return vector4_t(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x, 0.0f);
		}
	
		float vector4_t::length3(const vector4_t &v) {
			return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		}

		float vector4_t::length_squared3(const vector4_t &v) {
			return v.x * v.x + v.y * v.y + v.z * v.z;
		}

	} // namespace core
} // namespace maki
