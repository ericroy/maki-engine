#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{
		class matrix44_t;

		class vector4_t
		{
		public:
			static const vector4_t UnitX;
			static const vector4_t UnitY;
			static const vector4_t UnitZ;

		public:
			inline vector4_t() {}
			inline vector4_t(float xyz) : x(xyz), y(xyz), z(xyz), w(1.0f) {}
			inline vector4_t(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
			inline vector4_t(const float *vals) : x(vals[0]), y(vals[1]), z(vals[2]), w(vals[3]) {}

			// Member vector operations
			inline float length() const { return length(*this); }
			inline float LengthSquared() const { return LengthSquared(*this); }
			inline float dot(const vector4_t &v) const { return dot(*this, v); }
			inline vector4_t cross(const vector4_t &rhs) const { return cross(*this, rhs); }
			inline void normalize() { normalize(*this); }

			// Operator overloads
			inline float operator*(const vector4_t &v) const { return x*v.x_ + y*v.y_ + z*v.z_ + w*v.w_; }

			inline vector4_t operator*(float s) const { return vector4_t(x*s, y*s, z*s, w*s); }
			inline void operator*=(float s) { x*=s; y*=s; z*=s; w*=s; }

			inline vector4_t operator/(float s) const { assert(s != 0); return vector4_t(x/s, y/s, z/s, w/s); }
			inline void operator/=(float s) { assert(s != 0); x/=s; y/=s; z/=s; w/=s; }

			inline vector4_t operator-(const vector4_t &v) const { return vector4_t(x-v.x_, y-v.y_, z-v.z_, w-v.w_); }
			inline void operator-=(const vector4_t &v) { x-=v.x_; y-=v.y_; z-=v.z_; w-=v.w_; }

			inline vector4_t operator+(const vector4_t &v) const { return vector4_t(x+v.x_, y+v.y_, z+v.z_, w+v.w_); }
			inline void operator+=(const vector4_t &v) { x+=v.x_; y+=v.y_; z+=v.z_; w+=v.w_; }

			inline vector4_t operator-() const { return vector4_t(-x, -y, -z, -w); }

			inline bool operator==(const vector4_t &v) { return (x == v.x_ && y == v.y_ && z == v.z_ && w == v.w_); }
			inline bool operator!=(const vector4_t &v) { return (x != v.x_ || y != v.y_ || z != v.z_ || w != v.w_); }

			// Static methods

			static inline float dot(const vector4_t &v1, const vector4_t &v2) { return Dot3(v1, v2); }
			static inline float Dot4(const vector4_t &v1, const vector4_t &v2);
			static inline float Dot3(const vector4_t &v1, const vector4_t &v2);

			static inline void normalize(vector4_t &v) { return Normalize3(v); }
			static inline void Normalize3(vector4_t &v);
			static inline void Normalize4(vector4_t &v);

			static inline vector4_t cross(const vector4_t &a, const vector4_t &b) { return Cross3(a, b); }
			static inline vector4_t Cross3(const vector4_t &a, const vector4_t &b);

			static inline float length(const vector4_t &v) { return Length3(v); }
			static inline float Length3(const vector4_t &v);

			static inline float LengthSquared(const vector4_t &v) { return LengthSquared3(v); }
			static inline float LengthSquared3(const vector4_t &v);

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


		float vector4_t::Dot4(const vector4_t &v1, const vector4_t &v2)
		{
			return v1.x_ * v2.x_ + v1.y_ * v2.y_ + v1.z_ * v2.z_ + v1.w_ * v2.w_;
		}

		float vector4_t::Dot3(const vector4_t &v1, const vector4_t &v2)
		{
			return v1.x_ * v2.x_ + v1.y_ * v2.y_ + v1.z_ * v2.z_;
		}

		void vector4_t::Normalize3(vector4_t &v)
		{
			float invLen = 1.f / sqrt(v.x_*v.x_ + v.y_*v.y_ + v.z_*v.z_);
			v *= invLen;
		}

		void vector4_t::Normalize4(vector4_t &v)
		{
			float invLen = 1.f / sqrt(v.x_*v.x_ + v.y_*v.y_ + v.z_*v.z_ + v.w_*v.w_);
			v *= invLen;
		}

		vector4_t vector4_t::Cross3(const vector4_t &a, const vector4_t &b)
		{
			return vector4_t(a.y_*b.z_-a.z_*b.y_, a.z_*b.x_-a.x_*b.z_, a.x_*b.y_-a.y_*b.x_, 0.0f);
		}
	
		float vector4_t::Length3(const vector4_t &v)
		{
			return sqrt(v.x_*v.x_ + v.y_*v.y_ + v.z_*v.z_);
		}

		float vector4_t::LengthSquared3(const vector4_t &v)
		{
			return v.x_*v.x_ + v.y_*v.y_ + v.z_*v.z_;
		}

	} // namespace core

} // namespace maki
