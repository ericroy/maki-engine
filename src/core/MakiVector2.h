#pragma once
#include "core/core_stdafx.h"
#include "core/MakiVector4.h"

namespace Maki
{
	namespace Core
	{
		class Matrix44;

		class Vector2
		{
		public:
			inline Vector2() {}
			inline Vector2(float xy) : x(xy), y(xy) {}
			inline Vector2(float x, float y) : x(x), y(y) {}

			// Member vector operations

			inline float Length() const { return Length(*this); }
			inline float LengthSquared() const { return LengthSquared(*this); }
			inline float Dot(const Vector2 &v) const { return Dot(*this, v); }
			inline float Cross(const Vector2 &rhs) const { return Cross(*this, rhs); }
			inline void Normalize() { Normalize(*this); }

			// Operator overloads
			inline float operator*(const Vector2 &v) const { return x*v.x + y*v.y; }

			inline Vector2 operator*(float s) const { return Vector2(x*s, y*s); }
			inline void operator*=(float s) { x*=s; y*=s; }

			inline Vector2 operator/(float s) const { assert(s != 0); return Vector2(x/s, y/s); }
			inline void operator/=(float s) { assert(s != 0); x/=s; y/=s; }

			inline Vector2 operator-(const Vector2 &v) const { return Vector2(x-v.x, y-v.y); }
			inline void operator-=(const Vector2 &v) { x-=v.x; y-=v.y; }

			inline Vector2 operator+(const Vector2 &v) const { return Vector2(x+v.x, y+v.y); }
			inline void operator+=(const Vector2 &v) { x+=v.x; y+=v.y; }

			inline Vector2 operator-() const { return Vector2(-x, -y); }

			inline bool operator==(const Vector2 &v) { return x == v.x && y == v.y; }
			inline bool operator!=(const Vector2 &v) { return x != v.x || y != v.y; }

			// Static methods
			static inline float Dot(const Vector2 &v1, const Vector2 &v2);
			static inline void Normalize(Vector2 &v);
			static inline float Cross(const Vector2 &a, const Vector2 &b);
			static inline float Length(const Vector2 &v);
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

		float Vector2::Dot(const Vector2 &v1, const Vector2 &v2)
		{
			return v1.x * v2.x + v1.y * v2.y;
		}

		void Vector2::Normalize(Vector2 &v)
		{
			float invLen = 1.f / sqrt(v.x*v.x + v.y*v.y);
			v *= invLen;
		}

		float Vector2::Cross(const Vector2 &a, const Vector2 &b)
		{
			return a.x*b.y-a.y*b.x;
		}
	
		float Vector2::Length(const Vector2 &v)
		{
			return sqrt(v.x*v.x + v.y*v.y);
		}

		float Vector2::LengthSquared(const Vector2 &v)
		{
			return v.x*v.x + v.y*v.y;
		}

	} // namespace Core

} // namespace Maki
