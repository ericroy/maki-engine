#pragma once
#include "core/core_stdafx.h"
#include "core/MakiVector4.h"

namespace maki
{
	namespace core
	{
		class matrix44_t;

		class Vector2
		{
		public:
			inline Vector2() {}
			inline Vector2(float xy) : x(xy), y(xy) {}
			inline Vector2(float x, float y) : x(x), y(y) {}

			// Member vector operations

			inline float length() const { return length(*this); }
			inline float LengthSquared() const { return LengthSquared(*this); }
			inline float dot(const Vector2 &v) const { return dot(*this, v); }
			inline float cross(const Vector2 &rhs) const { return cross(*this, rhs); }
			inline void normalize() { normalize(*this); }

			// Operator overloads
			inline float operator*(const Vector2 &v) const { return x*v.x_ + y*v.y_; }

			inline Vector2 operator*(float s) const { return Vector2(x*s, y*s); }
			inline void operator*=(float s) { x*=s; y*=s; }

			inline Vector2 operator/(float s) const { assert(s != 0); return Vector2(x/s, y/s); }
			inline void operator/=(float s) { assert(s != 0); x/=s; y/=s; }

			inline Vector2 operator-(const Vector2 &v) const { return Vector2(x-v.x_, y-v.y_); }
			inline void operator-=(const Vector2 &v) { x-=v.x_; y-=v.y_; }

			inline Vector2 operator+(const Vector2 &v) const { return Vector2(x+v.x_, y+v.y_); }
			inline void operator+=(const Vector2 &v) { x+=v.x_; y+=v.y_; }

			inline Vector2 operator-() const { return Vector2(-x, -y); }

			inline bool operator==(const Vector2 &v) { return x == v.x_ && y == v.y_; }
			inline bool operator!=(const Vector2 &v) { return x != v.x_ || y != v.y_; }

			// Static methods
			static inline float dot(const Vector2 &v1, const Vector2 &v2);
			static inline void normalize(Vector2 &v);
			static inline float cross(const Vector2 &a, const Vector2 &b);
			static inline float length(const Vector2 &v);
			static inline float LengthSquared(const Vector2 &v);

		public:
			union
			{
				struct
				{
					float x, y;
				};
				float vals[2];
			};
		};

		float Vector2::dot(const Vector2 &v1, const Vector2 &v2)
		{
			return v1.x_ * v2.x_ + v1.y_ * v2.y_;
		}

		void Vector2::normalize(Vector2 &v)
		{
			float invLen = 1.f / sqrt(v.x_*v.x_ + v.y_*v.y_);
			v *= invLen;
		}

		float Vector2::cross(const Vector2 &a, const Vector2 &b)
		{
			return a.x_*b.y_-a.y_*b.x_;
		}
	
		float Vector2::length(const Vector2 &v)
		{
			return sqrt(v.x_*v.x_ + v.y_*v.y_);
		}

		float Vector2::LengthSquared(const Vector2 &v)
		{
			return v.x_*v.x_ + v.y_*v.y_;
		}

	} // namespace core

} // namespace maki
