#pragma once
#include "core/core_stdafx.h"
#include "core/MakiVector4.h"

namespace Maki
{
	class Matrix44;

	class Vector3
	{
	public:
		static const Vector3 UnitX;
		static const Vector3 UnitY;
		static const Vector3 UnitZ;

	public:
		inline Vector3() {}
		inline Vector3(float xyz) : x(xyz), y(xyz), z(xyz) {}
		inline Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
		inline Vector3(const Vector4 &v) : x(v.x), y(v.y), z(v.z) {}

		// Member vector operations

		inline float Length() const { return Length(*this); }
		inline float LengthSquared() const { return LengthSquared(*this); }
		inline float Dot(const Vector3 &v) const { return Dot(*this, v); }
		inline Vector3 Cross(const Vector3 &rhs) const { return Cross(*this, rhs); }
		inline void Normalize() { Normalize(*this); }

		// Operator overloads
		inline float operator*(const Vector3 &v) const { return x*v.x + y*v.y + z*v.z; }

		inline Vector3 operator*(float s) const { return Vector3(x*s, y*s, z*s); }
		inline void operator*=(float s) { x*=s; y*=s; z*=s; }

		inline Vector3 operator/(float s) const { assert(s != 0); return Vector3(x/s, y/s, z/s); }
		inline void operator/=(float s) { assert(s != 0); x/=s; y/=s; z/=s; }

		inline Vector3 operator-(const Vector3 &v) const { return Vector3(x-v.x, y-v.y, z-v.z); }
		inline void operator-=(const Vector3 &v) { x-=v.x; y-=v.y; z-=v.z; }

		inline Vector3 operator+(const Vector3 &v) const { return Vector3(x+v.x, y+v.y, z+v.z); }
		inline void operator+=(const Vector3 &v) { x+=v.x; y+=v.y; z+=v.z; }

		inline Vector3 operator-() const { return Vector3(-x, -y, -z); }

		inline bool operator==(const Vector3 &v) { return x == v.x && y == v.y && z == v.z; }
		inline bool operator!=(const Vector3 &v) { return x != v.x || y != v.y || z != v.z; }

		// Static methods
		static inline float Dot(const Vector3 &v1, const Vector3 &v2);
		static inline void Normalize(Vector3 &v);
		static inline Vector3 Cross(const Vector3 &a, const Vector3 &b);
		static inline float Length(const Vector3 &v);
		static inline float LengthSquared(const Vector3 &v);

	public:
		union
		{
			struct
			{
				float x, y, z;
			};
			float vals[3];
		};
	};

	float Vector3::Dot(const Vector3 &v1, const Vector3 &v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	void Vector3::Normalize(Vector3 &v)
	{
		float invLen = 1.f / sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
		v *= invLen;
	}

	Vector3 Vector3::Cross(const Vector3 &a, const Vector3 &b)
	{
		return Vector3(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
	}
	
	float Vector3::Length(const Vector3 &v)
	{
		return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	}

	float Vector3::LengthSquared(const Vector3 &v)
	{
		return v.x*v.x + v.y*v.y + v.z*v.z;
	}


} // namespace Maki
