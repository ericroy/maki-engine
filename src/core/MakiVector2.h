#pragma once
#include "core/core_stdafx.h"
#include "core/MakiVector4.h"

namespace maki
{
	namespace core
	{
		class matrix44_t;

		class vector2_t
		{
		public:
			inline vector2_t() {}
			inline vector2_t(float xy) : x_(xy), y_(xy) {}
			inline vector2_t(float x, float y) : x_(x), y_(y) {}

			// Member vector operations

			inline float length() const { return length(*this); }
			inline float length_squared() const { return length_squared(*this); }
			inline float dot(const vector2_t &v) const { return dot(*this, v); }
			inline float cross(const vector2_t &rhs) const { return cross(*this, rhs); }
			inline void normalize() { normalize(*this); }

			// Operator overloads
			inline float operator*(const vector2_t &v) const { return x_*v.x_ + y_*v.y_; }

			inline vector2_t operator*(float s) const { return vector2_t(x_*s, y_*s); }
			inline void operator*=(float s) { x_*=s; y_*=s; }

			inline vector2_t operator/(float s) const { assert(s != 0); return vector2_t(x_/s, y_/s); }
			inline void operator/=(float s) { assert(s != 0); x_/=s; y_/=s; }

			inline vector2_t operator-(const vector2_t &v) const { return vector2_t(x_-v.x_, y_-v.y_); }
			inline void operator-=(const vector2_t &v) { x_-=v.x_; y_-=v.y_; }

			inline vector2_t operator+(const vector2_t &v) const { return vector2_t(x_+v.x_, y_+v.y_); }
			inline void operator+=(const vector2_t &v) { x_+=v.x_; y_+=v.y_; }

			inline vector2_t operator-() const { return vector2_t(-x_, -y_); }

			inline bool operator==(const vector2_t &v) { return x_ == v.x_ && y_ == v.y_; }
			inline bool operator!=(const vector2_t &v) { return x_ != v.x_ || y_ != v.y_; }

			// Static methods
			static inline float dot(const vector2_t &v1, const vector2_t &v2);
			static inline void normalize(vector2_t &v);
			static inline float cross(const vector2_t &a, const vector2_t &b);
			static inline float length(const vector2_t &v);
			static inline float length_squared(const vector2_t &v);

		public:
			union
			{
				struct
				{
					float x_, y_;
				};
				float vals_[2];
			};
		};

		float vector2_t::dot(const vector2_t &v1, const vector2_t &v2)
		{
			return v1.x_ * v2.x_ + v1.y_ * v2.y_;
		}

		void vector2_t::normalize(vector2_t &v)
		{
			float inv_len = 1.f / sqrt(v.x_*v.x_ + v.y_*v.y_);
			v *= inv_len;
		}

		float vector2_t::cross(const vector2_t &a, const vector2_t &b)
		{
			return a.x_*b.y_-a.y_*b.x_;
		}
	
		float vector2_t::length(const vector2_t &v)
		{
			return sqrt(v.x_*v.x_ + v.y_*v.y_);
		}

		float vector2_t::length_squared(const vector2_t &v)
		{
			return v.x_*v.x_ + v.y_*v.y_;
		}

	} // namespace core

} // namespace maki
