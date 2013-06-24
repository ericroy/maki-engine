/*
Parts of this file based on OGRE quaterion class, the OGRE license appears below.
*/

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
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



#include "core/core_stdafx.h"
#include "core/MakiQuaternion.h"

namespace Maki
{
	namespace Core
	{

		const float Quaternion::Epsilon = 1e-03f;
		const Quaternion Quaternion::Zero(0.0f, 0.0f, 0.0f, 0.0f);
		const Quaternion Quaternion::Identity(1.0f, 0.0f, 0.0f, 0.0f);


		void Quaternion::ToMatrix(Matrix44 &out) const
		{
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

			out.cols[0][0] = 1.0f-(tyy+tzz);
			out.cols[1][0] = txy-twz;
			out.cols[2][0] = txz+twy;
			out.cols[0][1] = txy+twz;
			out.cols[1][1] = 1.0f-(txx+tzz);
			out.cols[2][1] = tyz-twx;
			out.cols[0][2] = txz-twy;
			out.cols[1][2] = tyz+twx;
			out.cols[2][2] = 1.0f-(txx+tyy);

			out.cols[3][0] = 0.0f;
			out.cols[3][1] = 0.0f;
			out.cols[3][2] = 0.0f;
			out.cols[3][3] = 1.0f;
			out.cols[0][3] = 0.0f;
			out.cols[1][3] = 0.0f;
			out.cols[2][3] = 0.0f;
		}

		void Quaternion::FromMatrix(const Matrix44 &m)
		{
			// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
			// article "Quaternion Calculus and Fast Animation".
			float trace = m.cols[0][0] + m.cols[1][1] + m.cols[2][2];
			float root;

			if(trace > 0.0) {
				// |w| > 1/2, may as well choose w > 1/2
				root = sqrt(trace + 1.0f);  // 2w
				w = 0.5f * root;
				root = 0.5f / root;  // 1/(4w)
				x = (m.cols[1][2] - m.cols[2][1])*root;
				y = (m.cols[2][0] - m.cols[0][2])*root;
				z = (m.cols[0][1] - m.cols[1][0])*root;
			} else {
				// |w| <= 1/2
				static size_t next[3] = { 1, 2, 0 };
				size_t i = 0;
				if(m.cols[1][1] > m.cols[0][0]) {
					i = 1;
				}
				if(m.cols[2][2] > m.cols[i][i]) {
					i = 2;
				}
				size_t j = next[i];
				size_t k = next[j];

				root = sqrt(m.cols[i][i] - m.cols[j][j] - m.cols[k][k] + 1.0f);
				float *apkQuat[3] = {&x, &y, &z};
				*apkQuat[i] = 0.5f * root;
				root = 0.5f / root;
				w = (m.cols[j][k] - m.cols[k][j]) * root;
				*apkQuat[j] = (m.cols[i][j] + m.cols[j][i]) * root;
				*apkQuat[k] = (m.cols[i][k] + m.cols[k][i]) * root;
			}
		}
	
		void Quaternion::FromEulerAngles(float thetaX, float thetaY, float thetaZ) {
			float hx = thetaX / 2.0f;
			float hy = thetaY / 2.0f;
			float hz = thetaZ / 2.0f;

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

		void Quaternion::ToEulerAngles(Vector3 &angles) const
		{
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

		void Quaternion::FromAngleAxis(float theta, const Vector3 &axis)
		{
			// assert:  axis is unit length
			//
			// The quaternion representing the rotation is
			//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

			float halfAngle(0.5f*theta);
			float s = sin(halfAngle);
			w = cos(halfAngle);
			x = s*axis.x;
			y = s*axis.y;
			z = s*axis.z;
		}

		void Quaternion::FromAngleAxis(float theta, const Vector4 &axis)
		{
			// assert:  axis is unit length
			//
			// The quaternion representing the rotation is
			//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

			float halfAngle(0.5f*theta);
			float s = sin(halfAngle);
			w = cos(halfAngle);
			x = s*axis.x;
			y = s*axis.y;
			z = s*axis.z;
		}

		void Quaternion::ToAngleAxis(float &theta, Vector3 &axis) const
		{
			// The quaternion representing the rotation is
			//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

			float lenSq = x*x+y*y+z*z;
			if(lenSq > 0.0f) {
				theta = 2.0f*acos(w);
				float invLen = 1.0f/sqrt(lenSq);
				axis.x = x*invLen;
				axis.y = y*invLen;
				axis.z = z*invLen;
			} else {
				// angle is 0 (mod 2*pi), so any axis will do
				theta = 0.0f;
				axis.x = 1.0f;
				axis.y = 0.0f;
				axis.z = 0.0f;
			}
		}

		void Quaternion::ToAngleAxis(float &theta, Vector4 &axis) const
		{
			// The quaternion representing the rotation is
			//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

			float lenSq = x*x+y*y+z*z;
			if(lenSq > 0.0f) {
				theta = 2.0f*acos(w);
				float invLen = 1.0f/sqrt(lenSq);
				axis.x = x*invLen;
				axis.y = y*invLen;
				axis.z = z*invLen;
			} else {
				// angle is 0 (mod 2*pi), so any axis will do
				theta = 0.0f;
				axis.x = 1.0f;
				axis.y = 0.0f;
				axis.z = 0.0f;
			}
		}

		void Quaternion::FromRotationArc(const Vector3 &start, const Vector3 &end)
		{
			// Calculate the quaternion that will rotate vector "start" to vector "end"
			Vector3 perp = start.Cross(end);
			x = perp.x; y = perp.y; z = perp.z;
			w = sqrt(start.LengthSquared() * end.LengthSquared()) + start.Dot(end);
			Normalize();
		}

		Quaternion Quaternion::Inverse() const
		{
			float norm = w*w + x*x + y*y + z*z;
			if(norm > 0.0f) {
				float invNorm = 1.0f/norm;
				return Quaternion(w*invNorm, -x*invNorm, -y*invNorm, -z*invNorm);
			} else {
				// return an invalid result to flag the error
				return Zero;
			}
		}

		Quaternion Quaternion::UnitInverse() const
		{
			// assert:  'this' is unit length
			return Quaternion(w, -x, -y, -z);
		}

		Quaternion Quaternion::Exp() const
		{
			// If q = A*(x*i+y*j+z*k) where (x,y,z) is unit length, then
			// exp(q) = cos(A)+sin(A)*(x*i+y*j+z*k).  If sin(A) is near zero,
			// use exp(q) = cos(A)+A*(x*i+y*j+z*k) since A/sin(A) has limit 1.

			float angle = sqrt(x*x + y*y + z*z);
			float s = sin(angle);

			Quaternion result;
			result.w = cos(angle);

			if(fabs(s) >= Epsilon)
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

		Quaternion Quaternion::Log() const
		{
			// If q = cos(A)+sin(A)*(x*i+y*j+z*k) where (x,y,z) is unit length, then
			// log(q) = A*(x*i+y*j+z*k).  If sin(A) is near zero, use log(q) =
			// sin(A)*(x*i+y*j+z*k) since sin(A)/A has limit 1.

			Quaternion result;
			result.w = 0.0f;

			if(fabs(w) < 1.0f) {
				float angle = acos(w);
				float s = sin(angle);
				if(fabs(s) >= Epsilon) {
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

		Vector3 Quaternion::operator*(const Vector3 &v) const
		{
			// nVidia SDK implementation
			Vector3 uv, uuv;
			Vector3 qvec(x, y, z);
			uv = qvec.Cross(v);
			uuv = qvec.Cross(uv);
			uv *= (2.0f * w);
			uuv *= 2.0f;
			return v + uv + uuv;
		}

		Vector4 Quaternion::operator*(const Vector4 &v) const
		{
			// nVidia SDK implementation
			Vector4 uv, uuv;
			Vector4 qvec(x, y, z, 1.0f);
			uv = qvec.Cross(v);
			uuv = qvec.Cross(uv);
			uv *= (2.0f * w);
			uuv *= 2.0f;
			return v + uv + uuv;
		}

	
		void Quaternion::Intermediate(const Quaternion &q0, const Quaternion &q1, const Quaternion &q2, Quaternion &a, Quaternion &b)
		{
			// assert:  q0, q1, q2 are unit quaternions
			Quaternion q0inv = q0.UnitInverse();
			Quaternion q1inv = q1.UnitInverse();
			Quaternion p0 = q0inv*q1;
			Quaternion p1 = q1inv*q2;
			Quaternion arg = 0.25f*(p0.Log()-p1.Log());

			a = q1 * arg.Exp();
			b = q1 * (-arg).Exp();
		}


		Quaternion Quaternion::Squad(float t, const Quaternion &p, const Quaternion &a, const Quaternion &b, const Quaternion &q)
		{
			float slerpT = 2.0f*t*(1.0f-t);
			Quaternion slerpP = Slerp(t, p, q);
			Quaternion slerpQ = Slerp(t, a, b);
			return Slerp(slerpT, slerpP ,slerpQ);
		}

		void Quaternion::ComputeW()
		{
			float t = 1.0f - (x*x) - (y*y) - (z*z);
			if(t < 0.0f) {
				w = 0.0f;
			} else {
				w = -sqrt(t);
			}
		}

	} // namespace Core

} // namespace Maki