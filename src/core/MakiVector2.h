#pragma once
#include "core/MakiTypes.h"
#include "core/MakiVector4.h"

namespace maki {
	namespace core {
		class matrix44_t;

		class vector2_t {
		public:
			inline vector2_t() : x(0.0f), y(0.0f) {}
			inline vector2_t(float xy) : x(xy), y(xy) {}
			inline vector2_t(float x, float y) : x(x), y(y) {}

			// Member vector operations

			inline float length() const { return length(*this); }
			inline float length_squared() const { return length_squared(*this); }
			inline float dot(const vector2_t &v) const { return dot(*this, v); }
			inline float cross(const vector2_t &rhs) const { return cross(*this, rhs); }
			inline void normalize() { normalize(*this); }

			// Operator overloads
			inline float operator*(const vector2_t &v) const { return x * v.x + y * v.y; }

			inline vector2_t operator*(float s) const { return vector2_t(x * s, y * s); }
			inline void operator*=(float s) { x *= s; y *= s; }

			inline vector2_t operator/(float s) const { MAKI_ASSERT(s != 0); return vector2_t(x / s, y / s); }
			inline void operator/=(float s) { MAKI_ASSERT(s != 0); x /= s; y /= s; }

			inline vector2_t operator-(const vector2_t &v) const { return vector2_t(x - v.x, y - v.y); }
			inline void operator-=(const vector2_t &v) { x -= v.x; y -= v.y; }

			inline vector2_t operator+(const vector2_t &v) const { return vector2_t(x + v.x, y + v.y); }
			inline void operator+=(const vector2_t &v) { x += v.x; y += v.y; }

			inline vector2_t operator-() const { return vector2_t(-x, -y); }

			inline bool operator==(const vector2_t &v) { return x == v.x && y == v.y; }
			inline bool operator!=(const vector2_t &v) { return x != v.x || y != v.y; }

			// Static methods
			static inline float dot(const vector2_t &v1, const vector2_t &v2);
			static inline void normalize(vector2_t &v);
			static inline float cross(const vector2_t &a, const vector2_t &b);
			static inline float length(const vector2_t &v);
			static inline float length_squared(const vector2_t &v);

		public:
			union {
				struct {
					float x, y;
				};
				float vals[2];
			};
		};

		float vector2_t::dot(const vector2_t &v1, const vector2_t &v2) {
			return v1.x * v2.x + v1.y * v2.y;
		}

		void vector2_t::normalize(vector2_t &v) {
			float inv_len = 1.f / sqrt(v.x*v.x + v.y*v.y);
			v *= inv_len;
		}

		float vector2_t::cross(const vector2_t &a, const vector2_t &b) {
			return a.x*b.y-a.y*b.x;
		}
	
		float vector2_t::length(const vector2_t &v) {
			return sqrt(v.x*v.x + v.y*v.y);
		}

		float vector2_t::length_squared(const vector2_t &v) {
			return v.x*v.x + v.y*v.y;
		}

	} // namespace core
} // namespace maki
