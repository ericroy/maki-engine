#pragma once
/*
Parts of this file based on OGRE quaterion class, the OGRE license appears below.
*/


/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (object_t-oriented Graphics Rendering engine_t)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
// This file is based on material originally from:
// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt


#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		class quaternion_t
		{
			friend quaternion_t operator*(float s, const quaternion_t &q);
    
		public:
			// Create the quaternion necessary to billboard a quad
			// to_camera: The direction from the sprite's world position to the camera's world position (normalized)
			// sprite_face_axis: The axis along which the untransformed sprite faces (normalized)
			// sprite_pivot_axis: The axis around which the sprite is allowed to pivot (normalized)
			static inline quaternion_t billboard_face(const vector4_t &to_camera, const vector4_t &sprite_face_axis, const vector4_t &sprite_up_axis);
			static inline quaternion_t billboard_pivot(const vector4_t &to_camera, const vector4_t &sprite_face_axis, const vector4_t &sprite_pivot_axis);

			// Performs Spherical linear interpolation between two quaternions, and returns the result.
			static inline quaternion_t slerp(float t, const quaternion_t &v0, const quaternion_t &v1);
			static inline quaternion_t nlerp(float t, const quaternion_t &v0, const quaternion_t &v1);

			// Setup for spherical quadratic interpolation
			static void intermediate(const quaternion_t &q0, const quaternion_t &q1, const quaternion_t &q2, quaternion_t &a, quaternion_t &b);

			// Spherical quadratic interpolation
			static quaternion_t squad(float t, const quaternion_t& p, const quaternion_t& a, const quaternion_t& b, const quaternion_t& q);

			// Cutoff for sine near zero
			static const float epsilon_;
			static const quaternion_t zero_;
			static const quaternion_t identity_;


			// Default constructor, initializes to identity rotation (aka 0 rads)
			inline quaternion_t() : w_(1), x_(0), y_(0), z_(0) {}
			inline quaternion_t(float theta_x, float theta_y, float theta_z) { from_euler_angles(theta_x, theta_y, theta_z); }
			inline quaternion_t(const vector3_t &euler_angles) { from_euler_angles(euler_angles); }
			inline quaternion_t(float w_, float x_, float y_, float z_) : w_(w_), x_(x_), y_(y_), z_(z_) {}

			void to_matrix(matrix44_t &rot) const;
			void from_matrix(const matrix44_t &m);

			void from_euler_angles(float theta_x, float theta_y, float theta_z);
			inline void from_euler_angles(const vector3_t &angles) { from_euler_angles(angles.x_, angles.y_, angles.z_); }
			void to_euler_angles(vector3_t &angles) const;

			void from_angle_axis(float theta, const vector3_t &axis);
			void from_angle_axis(float theta, const vector4_t &axis);
			void to_angle_axis(float &theta, vector3_t &axis) const;
			void to_angle_axis(float &theta, vector4_t &axis) const;

			// start and end must be normalized!
			void from_rotation_arc(const vector3_t &start, const vector3_t &end);

			inline quaternion_t operator+(const quaternion_t &q) const;
			inline quaternion_t operator-(const quaternion_t &q) const;
			inline quaternion_t operator*(const quaternion_t &q) const;
			inline quaternion_t operator*(float s) const;
			inline quaternion_t operator-() const;

			// functions of a quaternion
			// Returns the dot product of the quaternion
			inline float dot(const quaternion_t &q) const;

			// Returns the normal length of this quaternion.
			inline float norm() const;

			// Normalizes this quaternion, and returns the previous length
			inline void normalize(); 
			quaternion_t inverse() const;  // apply to non-zero quaternion
			quaternion_t unit_inverse() const;  // apply to unit-length quaternion
			quaternion_t exp() const;
			quaternion_t log() const;

			void compute_w();

			// Rotation of a vector by a quaternion
			vector3_t operator*(const vector3_t &v) const;
			vector4_t operator*(const vector4_t &v) const;

		public:
			float w_, x_, y_, z_;
		};


		quaternion_t quaternion_t::operator+(const quaternion_t &q) const
		{
			return quaternion_t(w_+q.w_, x_+q.x_, y_+q.y_, z_+q.z_);
		}

		quaternion_t quaternion_t::operator-(const quaternion_t &q) const
		{
			return quaternion_t(w_-q.w_, x_-q.x_, y_-q.y_, z_-q.z_);
		}

		quaternion_t quaternion_t::operator*(float s) const
		{
			return quaternion_t(s*w_,s*x_,s*y_,s*z_);
		}

		quaternion_t quaternion_t::operator*(const quaternion_t &q) const
		{
			// NOTE:  Multiplication is not generally commutative, so in most
			// cases p*q != q*p.
			return quaternion_t(
				w_ * q.w_ - x_ * q.x_ - y_ * q.y_ - z_ * q.z_,
				w_ * q.x_ + x_ * q.w_ + y_ * q.z_ - z_ * q.y_,
				w_ * q.y_ + y_ * q.w_ + z_ * q.x_ - x_ * q.z_,
				w_ * q.z_ + z_ * q.w_ + x_ * q.y_ - y_ * q.x_
			);
		}

		quaternion_t quaternion_t::operator-() const
		{
			return quaternion_t(-w_, -x_, -y_, -z_);
		}

		float quaternion_t::dot(const quaternion_t& q) const
		{
			return w_*q.w_ + x_*q.x_ + y_*q.y_ + z_*q.z_;
		}

		float quaternion_t::norm() const
		{
			return w_*w_ + x_*x_ + y_*y_ + z_*z_;
		}

		void quaternion_t::normalize(void)
		{
			float factor = 1.0f / sqrt(norm());
			w_ *= factor;
			x_ *= factor;
			y_ *= factor;
			z_ *= factor;
		}


		inline quaternion_t operator*(float s, const quaternion_t &q)
		{
			return quaternion_t(s*q.w_, s*q.x_, s*q.y_, s*q.z_);
		}


		// Based on Jonathan Blow's code presented here:
		// http://number-none.com/product/Understanding%20Slerp,%20Then%20Not%20Using%20It/
		//
		quaternion_t quaternion_t::slerp(float t, const quaternion_t &v0, const quaternion_t &v1)
		{
			// assert: v0, v1 are unit length
			float dot = v0.dot(v1);

			// Do we need to invert rotation?
			quaternion_t dst(v1);
			if(dot < 0.0f) {
				dot = -dot;
				dst = -v1;
			}

			if(dot > 0.9995f) {
				// Inputs are very close - linearly interpolate and normalize
				quaternion_t res = v0 * (1.0f - t) + dst * t;
				res.normalize();
				return res;
			}

			// clamp dot product (in case inputs were not *quite* normalized)
			if(dot < -1.0f)
			{
				dot = -1.0f;
			}
			else if(dot > 1.0f)
			{
				dot = 1.0f;
			}

			// theta = angle between v0 and the result
			float theta = std::acos(dot) * t;

			// This makes {v0, v2} an orthonormal basis
			quaternion_t v2 = dst - v0 * dot;
			v2.normalize();
				
			return v0 * std::cos(theta) + v2 * std::sin(theta);
		}

		quaternion_t quaternion_t::nlerp(float t, const quaternion_t &v0, const quaternion_t &v1)
		{
			// assert: v0, v1 are unit length
			float dot = v0.dot(v1);

			// Do we need to invert rotation?
			quaternion_t dst(v1);
			if(dot < 0.0f) {
				dot = -dot;
				dst = -v1;
			}

			quaternion_t res = v0 * (1.0f - t) + dst * t;
			res.normalize();
			return res;
		}


		quaternion_t quaternion_t::billboard_face(const vector4_t &to_camera, const vector4_t &sprite_face_axis, const vector4_t &sprite_up_axis)
		{
			// to_camera, sprite_face_axis, and sprite_pivot_axis must be normalized
			quaternion_t ret;
			ret.from_rotation_arc(sprite_face_axis, to_camera);
			return ret;
		}

		quaternion_t quaternion_t::billboard_pivot(const vector4_t &to_camera, const vector4_t &sprite_face_axis, const vector4_t &sprite_pivot_axis)
		{
			// to_camera, sprite_face_axis, and sprite_pivot_axis must be normalized
		
			// find the destination facing direction, as for normal billboarding.
			// Remove any component of that vector that is along the axis direction.
			// Renormalize.
			float axis_component = to_camera.dot(sprite_pivot_axis);
			if(fabs(axis_component) > 0.99999f) {
				// Degenerate case, desired facing direction is along the pivot axis
				return quaternion_t();
			}
			vector4_t dst = to_camera - sprite_pivot_axis * axis_component;
			dst.normalize();
		
			quaternion_t ret;
			ret.from_rotation_arc(sprite_face_axis, dst);
			return ret;
		}

	} // namespace core

} // namespace maki
