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
// NOTE THAT THIS FILE IS BASED ON MATERIAL FROM:

// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt



#include "core/MakiQuaternion.h"

namespace maki {
	namespace core {

		const float quaternion_t::epsilon = 1e-03f;
		const quaternion_t quaternion_t::zero(0.0f, 0.0f, 0.0f, 0.0f);
		const quaternion_t quaternion_t::identity(1.0f, 0.0f, 0.0f, 0.0f);


		void quaternion_t::to_matrix(matrix44_t &out) const {
			const float tx  = x+x;
			const float ty  = y+y;
			const float tz  = z+z;
			const float twx = tx*w;
			const float twy = ty*w;
			const float twz = tz*w;
			const float txx = tx*x;
			const float txy = ty*x;
			const float txz = tz*x;
			const float tyy = ty*y;
			const float tyz = tz*y;
			const float tzz = tz*z;

			out.cols_[0][0] = 1.0f-(tyy+tzz);
			out.cols_[1][0] = txy-twz;
			out.cols_[2][0] = txz+twy;
			out.cols_[0][1] = txy+twz;
			out.cols_[1][1] = 1.0f-(txx+tzz);
			out.cols_[2][1] = tyz-twx;
			out.cols_[0][2] = txz-twy;
			out.cols_[1][2] = tyz+twx;
			out.cols_[2][2] = 1.0f-(txx+tyy);

			out.cols_[3][0] = 0.0f;
			out.cols_[3][1] = 0.0f;
			out.cols_[3][2] = 0.0f;
			out.cols_[3][3] = 1.0f;
			out.cols_[0][3] = 0.0f;
			out.cols_[1][3] = 0.0f;
			out.cols_[2][3] = 0.0f;
		}

		void quaternion_t::from_matrix(const matrix44_t &m) {
			// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
			// article "quaternion_t Calculus and Fast Animation".
			float trace = m.cols_[0][0] + m.cols_[1][1] + m.cols_[2][2];
			float root;

			if(trace > 0.0) {
				// |w| > 1/2, may as well choose w > 1/2
				root = sqrt(trace + 1.0f);  // 2w
				w = 0.5f * root;
				root = 0.5f / root;  // 1/(4w)
				x = (m.cols_[1][2] - m.cols_[2][1])*root;
				y = (m.cols_[2][0] - m.cols_[0][2])*root;
				z = (m.cols_[0][1] - m.cols_[1][0])*root;
			} else {
				// |w| <= 1/2
				static size_t next[3] = { 1, 2, 0 };
				size_t i = 0;
				
				if(m.cols_[1][1] > m.cols_[0][0])
					i = 1;
				
				if(m.cols_[2][2] > m.cols_[i][i])
					i = 2;

				size_t j = next[i];
				size_t k = next[j];

				root = sqrt(m.cols_[i][i] - m.cols_[j][j] - m.cols_[k][k] + 1.0f);
				float *apk_quat[3] = {&x, &y, &z};
				*apk_quat[i] = 0.5f * root;
				root = 0.5f / root;
				w = (m.cols_[j][k] - m.cols_[k][j]) * root;
				*apk_quat[j] = (m.cols_[i][j] + m.cols_[j][i]) * root;
				*apk_quat[k] = (m.cols_[i][k] + m.cols_[k][i]) * root;
			}
		}
	
		void quaternion_t::from_euler_angles(float theta_x, float theta_y, float theta_z) {
			float hx = theta_x / 2.0f;
			float hy = theta_y / 2.0f;
			float hz = theta_z / 2.0f;

			float cx = cos(hx);
			float cy = cos(hy);
			float cz = cos(hz);

			float sx = sin(hx);
			float sy = sin(hy);
			float sz = sin(hz);

			w = cx*cy*cz + sx*sy*sz;
			x = sx*cy*cz - cx*sy*sz;
			y = cx*sy*cz + sx*cy*sz;
			z = cx*cy*sz - sx*sy*cz;
		}


		/*
		http://www.blitzbasic.com/Community/posts.php?topic=24769
		; -------------------------------------------------------------------------------------------------------------------
		; This function converts a quaternion to a set of Euler angles.
		;
		;
		; Parameters:
		;
		;	Qx#, Qy#, Qz#, Qw#:
		;		quaternion_t to convert to Euler angles.
		;
		; -------------------------------------------------------------------------------------------------------------------
		Function quat_to_euler(Qx#, Qy#, Qz#, Qw#)

			Qx2# = Qx# * 2.0
			Qy2# = Qy# * 2.0
			Qz2# = Qz# * 2.0
	
			Sin_T# = (Qy2# * Qw#) - (Qx2# * Qz#)
			Cos_T# = 1.0 - (Sin_T# * Sin_T#)


			If Abs(Cos_T#) > QuatToEuler_Epsilon#
				Cos_T# = Sqr(Cos_T#)	
			Else
				Cos_T# = 0
			EndIf
		
		
			If Abs(Cos_T#) > QuatToEuler_Epsilon#

				Sin_V# = (      (Qy2# * Qz#) + (Qx2# * Qw#)) / Cos_T#
				Cos_V# = (1.0 - (Qx2# * Qx#) - (Qy2# * Qy#)) / Cos_T#
				Sin_F# = (      (Qx2# * Qy#) + (Qz2# * Qw#)) / Cos_T#
				Cos_F# = (1.0 - (Qy2# * Qy#) - (Qz2# * Qz#)) / Cos_T#

			Else

				Sin_V# =       (Qx2# * Qw#) - (Qy2# * Qz#)
				Cos_V# = 1.0 - (Qx2# * Qx#) - (Qz2# * Qz#)
				Sin_F# = 0
				Cos_F# = 1.0

			EndIf

			QuatLib_Pitch# =  ATan2(Sin_T#, Cos_T#)
			QuatLib_Yaw#   =  ATan2(Sin_F#, Cos_F#)
			QuatLib_Roll#  = -ATan2(Sin_V#, Cos_V#)

		end Function

		*/

		
		void quaternion_t::to_euler_angles(vector3_t &angles) const {
			// Adapted from code found here:
			// http://grezeszak.com/basics-of-quaternions-and-euler-angles/2011/
			// Only implementation I could find that was consistent with the other stuff in this file

			float ww = w * w;
			float xx = x * x;
			float yy = y * y;
			float zz = z * z;

			float test = w*y - z*x;
		
			// 0.4999999f will correspond to 89.96376 degrees
			if(test > 0.4999999f) {
				angles.x = -2.0f * atan2(w, x);
				angles.y = MAKI_PI/2.0f;
				angles.z = MAKI_PI;
			} else if(test < -0.4999999f) {
				angles.x = -2.0f * atan2(w, x);
				angles.y = -MAKI_PI/2.0f;
				angles.z = MAKI_PI;
			} else {
				angles.x = atan2(2.0f * (w * x + y * z), 1.0f - 2.0f * (xx + yy) );
				angles.y = asin (2.0f * test);
				angles.z = atan2(2.0f * (w * z + x * y), 1.0f - 2.0f * (yy + zz) );
			}
		}
		

		void quaternion_t::from_angle_axis(float theta, const vector3_t &axis) {
			// assert:  axis is unit length
			//
			// The quaternion representing the rotation is
			//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

			float half_angle(0.5f*theta);
			float s = sin(half_angle);
			w = cos(half_angle);
			x = s*axis.x;
			y = s*axis.y;
			z = s*axis.z;
		}

		void quaternion_t::from_angle_axis(float theta, const vector4_t &axis) {
			// assert:  axis is unit length
			//
			// The quaternion representing the rotation is
			//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

			float half_angle(0.5f*theta);
			float s = sin(half_angle);
			w = cos(half_angle);
			x = s*axis.x;
			y = s*axis.y;
			z = s*axis.z;
		}

		void quaternion_t::to_angle_axis(float &theta, vector3_t &axis) const {
			// The quaternion representing the rotation is
			//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

			float len_sq = x*x+y*y+z*z;
			if(len_sq > 0.0f) {
				theta = 2.0f*acos(w);
				float inv_len = 1.0f/sqrt(len_sq);
				axis.x = x*inv_len;
				axis.y = y*inv_len;
				axis.z = z*inv_len;
			} else {
				// angle is 0 (mod 2*pi), so any axis will do
				theta = 0.0f;
				axis.x = 1.0f;
				axis.y = 0.0f;
				axis.z = 0.0f;
			}
		}

		void quaternion_t::to_angle_axis(float &theta, vector4_t &axis) const {
			// The quaternion representing the rotation is
			//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

			float len_sq = x*x+y*y+z*z;
			if(len_sq > 0.0f) {
				theta = 2.0f*acos(w);
				float inv_len = 1.0f/sqrt(len_sq);
				axis.x = x*inv_len;
				axis.y = y*inv_len;
				axis.z = z*inv_len;
			} else {
				// angle is 0 (mod 2*pi), so any axis will do
				theta = 0.0f;
				axis.x = 1.0f;
				axis.y = 0.0f;
				axis.z = 0.0f;
			}
		}

		void quaternion_t::from_rotation_arc(const vector3_t &start, const vector3_t &end) {
			// Calculate the quaternion that will rotate vector "start" to vector "end"
			vector3_t perp = start.cross(end);
			x = perp.x; y = perp.y; z = perp.z;
			w = sqrt(start.length_squared() * end.length_squared()) + start.dot(end);
			normalize();
		}

		quaternion_t quaternion_t::inverse() const {
			float norm = w*w + x*x + y*y + z*z;
			if(norm > 0.0f) {
				float inv_norm = 1.0f/norm;
				return quaternion_t(w*inv_norm, -x*inv_norm, -y*inv_norm, -z*inv_norm);
			} else {
				// return an invalid result to flag the error
				return zero_;
			}
		}

		quaternion_t quaternion_t::unit_inverse() const {
			// assert:  'this' is unit length
			return quaternion_t(w, -x, -y, -z);
		}

		quaternion_t quaternion_t::exp() const {
			// If q = A*(x*i+y*j+z*k) where (x,y,z) is unit length, then
			// exp(q) = cos(A)+sin(A)*(x*i+y*j+z*k).  If sin(A) is near zero,
			// use exp(q) = cos(A)+A*(x*i+y*j+z*k) since A/sin(A) has limit 1.

			float angle = sqrt(x*x + y*y + z*z);
			float s = sin(angle);

			quaternion_t result;
			result.w = cos(angle);

			if(fabs(s) >= epsilon_)
			{
				float coeff = s/angle;
				result.x = coeff*x;
				result.y = coeff*y;
				result.z = coeff*z;
			}
			else
			{
				result.x = x;
				result.y = y;
				result.z = z;
			}

			return result;
		}

		quaternion_t quaternion_t::log() const {
			// If q = cos(A)+sin(A)*(x*i+y*j+z*k) where (x,y,z) is unit length, then
			// log(q) = A*(x*i+y*j+z*k).  If sin(A) is near zero, use log(q) =
			// sin(A)*(x*i+y*j+z*k) since sin(A)/A has limit 1.

			quaternion_t result;
			result.w = 0.0f;

			if(fabs(w) < 1.0f) {
				float angle = acos(w);
				float s = sin(angle);
				if(fabs(s) >= epsilon_) {
					float coeff = angle/s;
					result.x = coeff*x;
					result.y = coeff*y;
					result.z = coeff*z;
					return result;
				}
			}

			result.x = x;
			result.y = y;
			result.z = z;
			return result;
		}

		vector3_t quaternion_t::operator*(const vector3_t &v) const {
			// nVidia SDK implementation
			vector3_t uv, uuv;
			vector3_t qvec(x, y, z);
			uv = qvec.cross(v);
			uuv = qvec.cross(uv);
			uv *= (2.0f * w);
			uuv *= 2.0f;
			return v + uv + uuv;
		}

		vector4_t quaternion_t::operator*(const vector4_t &v) const {
			// nVidia SDK implementation
			vector4_t uv, uuv;
			vector4_t qvec(x, y, z, 1.0f);
			uv = qvec.cross(v);
			uuv = qvec.cross(uv);
			uv *= (2.0f * w);
			uuv *= 2.0f;
			return v + uv + uuv;
		}

	
		void quaternion_t::intermediate(const quaternion_t &q0, const quaternion_t &q1, const quaternion_t &q2, quaternion_t &a, quaternion_t &b) {
			// assert:  q0, q1, q2 are unit quaternions
			quaternion_t q0inv = q0.unit_inverse();
			quaternion_t q1inv = q1.unit_inverse();
			quaternion_t p0 = q0inv*q1;
			quaternion_t p1 = q1inv*q2;
			quaternion_t arg = 0.25f*(p0.log()-p1.log());

			a = q1 * arg.exp();
			b = q1 * (-arg).exp();
		}


		quaternion_t quaternion_t::squad(float t, const quaternion_t &p, const quaternion_t &a, const quaternion_t &b, const quaternion_t &q) {
			float t_slerp = 2.0f*t*(1.0f-t);
			quaternion_t p_slerp = slerp(t, p, q);
			quaternion_t q_slerp = slerp(t, a, b);
			return slerp(t_slerp, p_slerp, q_slerp);
		}

		void quaternion_t::compute_w() {
			float t = 1.0f - (x*x) - (y*y) - (z*z);
			if(t < 0.0f)
				w = 0.0f;
			else
				w = -sqrt(t);
		}

	} // namespace core
} // namespace maki
