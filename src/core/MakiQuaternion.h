#pragma once
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
// This file is based on material originally from:
// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt


#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{

		class Quaternion
		{
			friend Quaternion operator*(float s, const Quaternion &q);
    
		public:
			// Create the quaternion necessary to billboard a quad
			// toCamera: The direction from the sprite's world position to the camera's world position (normalized)
			// spriteFaceAxis: The axis along which the untransformed sprite faces (normalized)
			// spritePivotAxis: The axis around which the sprite is allowed to pivot (normalized)
			static inline Quaternion BillboardFace(const Vector4 &toCamera, const Vector4 &spriteFaceAxis, const Vector4 &spriteUpAxis);
			static inline Quaternion BillboardPivot(const Vector4 &toCamera, const Vector4 &spriteFaceAxis, const Vector4 &spritePivotAxis);

			// Performs Spherical linear interpolation between two quaternions, and returns the result.
			static inline Quaternion Slerp(float t, const Quaternion &v0, const Quaternion &v1);
			static inline Quaternion Nlerp(float t, const Quaternion &v0, const Quaternion &v1);

			// Setup for spherical quadratic interpolation
			static void Intermediate(const Quaternion &q0, const Quaternion &q1, const Quaternion &q2, Quaternion &a, Quaternion &b);

			// Spherical quadratic interpolation
			static Quaternion Squad(float t, const Quaternion& p, const Quaternion& a, const Quaternion& b, const Quaternion& q);

			// Cutoff for sine near zero
			static const float Epsilon;
			static const Quaternion Zero;
			static const Quaternion Identity;


			// Default constructor, initializes to identity rotation (aka 0 rads)
			inline Quaternion() : w(1), x(0), y(0), z(0) {}
			inline Quaternion(float thetaX, float thetaY, float thetaZ) { FromEulerAngles(thetaX, thetaY, thetaZ); }
			inline Quaternion(const Vector3 &eulerAngles) { FromEulerAngles(eulerAngles); }
			inline Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

			void ToMatrix(Matrix44 &rot) const;
			void FromMatrix(const Matrix44 &m);

			void FromEulerAngles(float thetaX, float thetaY, float thetaZ);
			inline void FromEulerAngles(const Vector3 &angles) { FromEulerAngles(angles.x, angles.y, angles.z); }
			void ToEulerAngles(Vector3 &angles) const;

			void FromAngleAxis(float theta, const Vector3 &axis);
			void FromAngleAxis(float theta, const Vector4 &axis);
			void ToAngleAxis(float &theta, Vector3 &axis) const;
			void ToAngleAxis(float &theta, Vector4 &axis) const;

			// Start and end must be normalized!
			void FromRotationArc(const Vector3 &start, const Vector3 &end);

			inline Quaternion operator+(const Quaternion &q) const;
			inline Quaternion operator-(const Quaternion &q) const;
			inline Quaternion operator*(const Quaternion &q) const;
			inline Quaternion operator*(float s) const;
			inline Quaternion operator-() const;

			// functions of a quaternion
			// Returns the dot product of the quaternion
			inline float Dot(const Quaternion &q) const;

			// Returns the normal length of this quaternion.
			inline float Norm() const;

			// Normalizes this quaternion, and returns the previous length
			inline void Normalize(); 
			Quaternion Inverse() const;  // apply to non-zero quaternion
			Quaternion UnitInverse() const;  // apply to unit-length quaternion
			Quaternion Exp() const;
			Quaternion Log() const;

			void ComputeW();

			// Rotation of a vector by a quaternion
			Vector3 operator*(const Vector3 &v) const;
			Vector4 operator*(const Vector4 &v) const;

		public:
			float w, x, y, z;
		};


		Quaternion Quaternion::operator+(const Quaternion &q) const
		{
			return Quaternion(w+q.w, x+q.x, y+q.y, z+q.z);
		}

		Quaternion Quaternion::operator-(const Quaternion &q) const
		{
			return Quaternion(w-q.w, x-q.x, y-q.y, z-q.z);
		}

		Quaternion Quaternion::operator*(float s) const
		{
			return Quaternion(s*w,s*x,s*y,s*z);
		}

		Quaternion Quaternion::operator*(const Quaternion &q) const
		{
			// NOTE:  Multiplication is not generally commutative, so in most
			// cases p*q != q*p.
			return Quaternion(
				w * q.w - x * q.x - y * q.y - z * q.z,
				w * q.x + x * q.w + y * q.z - z * q.y,
				w * q.y + y * q.w + z * q.x - x * q.z,
				w * q.z + z * q.w + x * q.y - y * q.x
			);
		}

		Quaternion Quaternion::operator-() const
		{
			return Quaternion(-w, -x, -y, -z);
		}

		float Quaternion::Dot(const Quaternion& q) const
		{
			return w*q.w + x*q.x + y*q.y + z*q.z;
		}

		float Quaternion::Norm() const
		{
			return w*w + x*x + y*y + z*z;
		}

		void Quaternion::Normalize(void)
		{
			float factor = 1.0f / sqrt(Norm());
			w *= factor;
			x *= factor;
			y *= factor;
			z *= factor;
		}


		inline Quaternion operator*(float s, const Quaternion &q)
		{
			return Quaternion(s*q.w, s*q.x, s*q.y, s*q.z);
		}


		// Based on Jonathan Blow's code presented here:
		// http://number-none.com/product/Understanding%20Slerp,%20Then%20Not%20Using%20It/
		//
		Quaternion Quaternion::Slerp(float t, const Quaternion &v0, const Quaternion &v1)
		{
			// assert: v0, v1 are unit length
			float dot = v0.Dot(v1);

			// Do we need to invert rotation?
			Quaternion dst(v1);
			if(dot < 0.0f) {
				dot = -dot;
				dst = -v1;
			}

			if(dot > 0.9995f) {
				// Inputs are very close - linearly interpolate and normalize
				Quaternion res = v0 * (1.0f - t) + dst * t;
				res.Normalize();
				return res;
			}

			// Clamp dot product (in case inputs were not *quite* normalized)
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
			Quaternion v2 = dst - v0 * dot;
			v2.Normalize();
				
			return v0 * std::cos(theta) + v2 * std::sin(theta);
		}

		Quaternion Quaternion::Nlerp(float t, const Quaternion &v0, const Quaternion &v1)
		{
			// assert: v0, v1 are unit length
			float dot = v0.Dot(v1);

			// Do we need to invert rotation?
			Quaternion dst(v1);
			if(dot < 0.0f) {
				dot = -dot;
				dst = -v1;
			}

			Quaternion res = v0 * (1.0f - t) + dst * t;
			res.Normalize();
			return res;
		}


		Quaternion Quaternion::BillboardFace(const Vector4 &toCamera, const Vector4 &spriteFaceAxis, const Vector4 &spriteUpAxis)
		{
			// toCamera, spriteFaceAxis, and spritePivotAxis must be normalized
			Quaternion ret;
			ret.FromRotationArc(spriteFaceAxis, toCamera);
			return ret;
		}

		Quaternion Quaternion::BillboardPivot(const Vector4 &toCamera, const Vector4 &spriteFaceAxis, const Vector4 &spritePivotAxis)
		{
			// toCamera, spriteFaceAxis, and spritePivotAxis must be normalized
		
			// Find the destination facing direction, as for normal billboarding.
			// Remove any component of that vector that is along the axis direction.
			// Renormalize.
			float axisComponent = toCamera.Dot(spritePivotAxis);
			if(fabs(axisComponent) > 0.99999f) {
				// Degenerate case, desired facing direction is along the pivot axis
				return Quaternion();
			}
			Vector4 dst = toCamera - spritePivotAxis * axisComponent;
			dst.Normalize();
		
			Quaternion ret;
			ret.FromRotationArc(spriteFaceAxis, dst);
			return ret;
		}

	} // namespace Core

} // namespace Maki