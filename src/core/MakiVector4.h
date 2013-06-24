#pragma once

namespace Maki
{
	namespace Core
	{
		class Matrix44;

		class Vector4
		{
		public:
			static const Vector4 UnitX;
			static const Vector4 UnitY;
			static const Vector4 UnitZ;

		public:
			inline Vector4() {}
			inline Vector4(float xyz) : x(xyz), y(xyz), z(xyz), w(1.0f) {}
			inline Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
			inline Vector4(const float *vals) : x(vals[0]), y(vals[1]), z(vals[2]), w(vals[3]) {}

			// Member vector operations
			inline float Length() const { return Length(*this); }
			inline float LengthSquared() const { return LengthSquared(*this); }
			inline float Dot(const Vector4 &v) const { return Dot(*this, v); }
			inline Vector4 Cross(const Vector4 &rhs) const { return Cross(*this, rhs); }
			inline void Normalize() { Normalize(*this); }

			// Operator overloads
			inline float operator*(const Vector4 &v) const { return x*v.x + y*v.y + z*v.z + w*v.w; }

			inline Vector4 operator*(float s) const { return Vector4(x*s, y*s, z*s, w*s); }
			inline void operator*=(float s) { x*=s; y*=s; z*=s; w*=s; }

			inline Vector4 operator/(float s) const { assert(s != 0); return Vector4(x/s, y/s, z/s, w/s); }
			inline void operator/=(float s) { assert(s != 0); x/=s; y/=s; z/=s; w/=s; }

			inline Vector4 operator-(const Vector4 &v) const { return Vector4(x-v.x, y-v.y, z-v.z, w-v.w); }
			inline void operator-=(const Vector4 &v) { x-=v.x; y-=v.y; z-=v.z; w-=v.w; }

			inline Vector4 operator+(const Vector4 &v) const { return Vector4(x+v.x, y+v.y, z+v.z, w+v.w); }
			inline void operator+=(const Vector4 &v) { x+=v.x; y+=v.y; z+=v.z; w+=v.w; }

			inline Vector4 operator-() const { return Vector4(-x, -y, -z, -w); }

			inline bool operator==(const Vector4 &v) { return (x == v.x && y == v.y && z == v.z && w == v.w); }
			inline bool operator!=(const Vector4 &v) { return (x != v.x || y != v.y || z != v.z || w != v.w); }

			// Static methods

			static inline float Dot(const Vector4 &v1, const Vector4 &v2) { return Dot3(v1, v2); }
			static inline float Dot4(const Vector4 &v1, const Vector4 &v2);
			static inline float Dot3(const Vector4 &v1, const Vector4 &v2);

			static inline void Normalize(Vector4 &v) { return Normalize3(v); }
			static inline void Normalize3(Vector4 &v);
			static inline void Normalize4(Vector4 &v);

			static inline Vector4 Cross(const Vector4 &a, const Vector4 &b) { return Cross3(a, b); }
			static inline Vector4 Cross3(const Vector4 &a, const Vector4 &b);

			static inline float Length(const Vector4 &v) { return Length3(v); }
			static inline float Length3(const Vector4 &v);

			static inline float LengthSquared(const Vector4 &v) { return LengthSquared3(v); }
			static inline float LengthSquared3(const Vector4 &v);

		public:
			union
			{
				struct
				{
					float x, y, z, w;
				};
				float vals[4];
			};
		};


		float Vector4::Dot4(const Vector4 &v1, const Vector4 &v2)
		{
			return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
		}

		float Vector4::Dot3(const Vector4 &v1, const Vector4 &v2)
		{
			return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
		}

		void Vector4::Normalize3(Vector4 &v)
		{
			float invLen = 1.f / sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
			v *= invLen;
		}

		void Vector4::Normalize4(Vector4 &v)
		{
			float invLen = 1.f / sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
			v *= invLen;
		}

		Vector4 Vector4::Cross3(const Vector4 &a, const Vector4 &b)
		{
			return Vector4(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x, 0.0f);
		}
	
		float Vector4::Length3(const Vector4 &v)
		{
			return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
		}

		float Vector4::LengthSquared3(const Vector4 &v)
		{
			return v.x*v.x + v.y*v.y + v.z*v.z;
		}

	} // namespace Core

} // namespace Maki
