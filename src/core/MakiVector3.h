#pragma once
#include "core/core_stdafx.h"
#include "core/MakiVector4.h"

namespace maki
{
	namespace core
	{
		class matrix44_t;

		class vector3_t
		{
		public:
			static const vector3_t unit_x_;
			static const vector3_t unit_y_;
			static const vector3_t unit_z_;

		public:
			inline vector3_t() {}
			inline vector3_t(float xyz) : x_(xyz), y_(xyz), z_(xyz) {}
			inline vector3_t(float x, float y, float z) : x_(x), y_(y), z_(z) {}
			inline vector3_t(const vector4_t &v) : x_(v.x_), y_(v.y_), z_(v.z_) {}

			// Member vector operations

			inline float length() const { return length(*this); }
			inline float length_squared() const { return length_squared(*this); }
			inline float dot(const vector3_t &v) const { return dot(*this, v); }
			inline vector3_t cross(const vector3_t &rhs) const { return cross(*this, rhs); }
			inline void normalize() { normalize(*this); }

			// Operator overloads
			inline float operator*(const vector3_t &v) const { return x_*v.x_ + y_*v.y_ + z_*v.z_; }

			inline vector3_t operator*(float s) const { return vector3_t(x_*s, y_*s, z_*s); }
			inline void operator*=(float s) { x_*=s; y_*=s; z_*=s; }

			inline vector3_t operator/(float s) const { assert(s != 0); return vector3_t(x_/s, y_/s, z_/s); }
			inline void operator/=(float s) { assert(s != 0); x_/=s; y_/=s; z_/=s; }

			inline vector3_t operator-(const vector3_t &v) const { return vector3_t(x_-v.x_, y_-v.y_, z_-v.z_); }
			inline void operator-=(const vector3_t &v) { x_-=v.x_; y_-=v.y_; z_-=v.z_; }

			inline vector3_t operator+(const vector3_t &v) const { return vector3_t(x_+v.x_, y_+v.y_, z_+v.z_); }
			inline void operator+=(const vector3_t &v) { x_+=v.x_; y_+=v.y_; z_+=v.z_; }

			inline vector3_t operator-() const { return vector3_t(-x_, -y_, -z_); }

			inline bool operator==(const vector3_t &v) { return x_ == v.x_ && y_ == v.y_ && z_ == v.z_; }
			inline bool operator!=(const vector3_t &v) { return x_ != v.x_ || y_ != v.y_ || z_ != v.z_; }

			// Static methods
			static inline float dot(const vector3_t &v1, const vector3_t &v2);
			static inline void normalize(vector3_t &v);
			static inline vector3_t cross(const vector3_t &a, const vector3_t &b);
			static inline float length(const vector3_t &v);
			static inline float length_squared(const vector3_t &v);

		public:
			union
			{
				struct
				{
					float x_, y_, z_;
				};
				float vals_[3];
			};
		};

		float vector3_t::dot(const vector3_t &v1, const vector3_t &v2)
		{
			return v1.x_ * v2.x_ + v1.y_ * v2.y_ + v1.z_ * v2.z_;
		}

		void vector3_t::normalize(vector3_t &v)
		{
			float inv_len = 1.f / sqrt(v.x_*v.x_ + v.y_*v.y_ + v.z_*v.z_);
			v *= inv_len;
		}

		vector3_t vector3_t::cross(const vector3_t &a, const vector3_t &b)
		{
			return vector3_t(a.y_*b.z_-a.z_*b.y_, a.z_*b.x_-a.x_*b.z_, a.x_*b.y_-a.y_*b.x_);
		}
	
		float vector3_t::length(const vector3_t &v)
		{
			return sqrt(v.x_*v.x_ + v.y_*v.y_ + v.z_*v.z_);
		}

		float vector3_t::length_squared(const vector3_t &v)
		{
			return v.x_*v.x_ + v.y_*v.y_ + v.z_*v.z_;
		}


	} // namespace core

} // namespace maki
