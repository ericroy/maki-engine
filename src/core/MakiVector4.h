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
			static const vector4_t unit_x_;
			static const vector4_t unit_y_;
			static const vector4_t unit_z_;

		public:
			inline vector4_t() {}
			inline vector4_t(float xyz) : x_(xyz), y_(xyz), z_(xyz), w_(1.0f) {}
			inline vector4_t(float x, float y, float z, float w) : x_(x), y_(y), z_(z), w_(w) {}
			inline vector4_t(const float *vals) : x_(vals[0]), y_(vals[1]), z_(vals[2]), w_(vals[3]) {}

			// Member vector operations
			inline float length() const { return length(*this); }
			inline float length_squared() const { return length_squared(*this); }
			inline float dot(const vector4_t &v) const { return dot(*this, v); }
			inline vector4_t cross(const vector4_t &rhs) const { return cross(*this, rhs); }
			inline void normalize() { normalize(*this); }

			// Operator overloads
			inline float operator*(const vector4_t &v) const { return x_*v.x_ + y_*v.y_ + z_*v.z_ + w_*v.w_; }

			inline vector4_t operator*(float s) const { return vector4_t(x_*s, y_*s, z_*s, w_*s); }
			inline void operator*=(float s) { x_*=s; y_*=s; z_*=s; w_*=s; }

			inline vector4_t operator/(float s) const { assert(s != 0); return vector4_t(x_/s, y_/s, z_/s, w_/s); }
			inline void operator/=(float s) { assert(s != 0); x_/=s; y_/=s; z_/=s; w_/=s; }

			inline vector4_t operator-(const vector4_t &v) const { return vector4_t(x_-v.x_, y_-v.y_, z_-v.z_, w_-v.w_); }
			inline void operator-=(const vector4_t &v) { x_-=v.x_; y_-=v.y_; z_-=v.z_; w_-=v.w_; }

			inline vector4_t operator+(const vector4_t &v) const { return vector4_t(x_+v.x_, y_+v.y_, z_+v.z_, w_+v.w_); }
			inline void operator+=(const vector4_t &v) { x_+=v.x_; y_+=v.y_; z_+=v.z_; w_+=v.w_; }

			inline vector4_t operator-() const { return vector4_t(-x_, -y_, -z_, -w_); }

			inline bool operator==(const vector4_t &v) { return (x_ == v.x_ && y_ == v.y_ && z_ == v.z_ && w_ == v.w_); }
			inline bool operator!=(const vector4_t &v) { return (x_ != v.x_ || y_ != v.y_ || z_ != v.z_ || w_ != v.w_); }

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
			union
			{
				struct
				{
					float x_, y_, z_, w_;
				};
				float vals_[4];
			};
		};


		float vector4_t::dot4(const vector4_t &v1, const vector4_t &v2)
		{
			return v1.x_ * v2.x_ + v1.y_ * v2.y_ + v1.z_ * v2.z_ + v1.w_ * v2.w_;
		}

		float vector4_t::dot3(const vector4_t &v1, const vector4_t &v2)
		{
			return v1.x_ * v2.x_ + v1.y_ * v2.y_ + v1.z_ * v2.z_;
		}

		void vector4_t::normalize3(vector4_t &v)
		{
			float invLen = 1.f / sqrt(v.x_*v.x_ + v.y_*v.y_ + v.z_*v.z_);
			v *= invLen;
		}

		void vector4_t::normalize4(vector4_t &v)
		{
			float invLen = 1.f / sqrt(v.x_*v.x_ + v.y_*v.y_ + v.z_*v.z_ + v.w_*v.w_);
			v *= invLen;
		}

		vector4_t vector4_t::cross3(const vector4_t &a, const vector4_t &b)
		{
			return vector4_t(a.y_*b.z_-a.z_*b.y_, a.z_*b.x_-a.x_*b.z_, a.x_*b.y_-a.y_*b.x_, 0.0f);
		}
	
		float vector4_t::length3(const vector4_t &v)
		{
			return sqrt(v.x_*v.x_ + v.y_*v.y_ + v.z_*v.z_);
		}

		float vector4_t::length_squared3(const vector4_t &v)
		{
			return v.x_*v.x_ + v.y_*v.y_ + v.z_*v.z_;
		}

	} // namespace core

} // namespace maki
