#include "core/core_stdafx.h"
#include "core/MakiMatrix44.h"
#include <cmath>

namespace Maki
{
	const Matrix44 Matrix44::Identity(true);
	
	void Matrix44::RotationX(float rads, Matrix44 &m) {
		float c = cos(rads);
		float s = sin(rads);
		m.cols[1][1] = c;
		m.cols[2][2] = c;
		m.cols[2][1] = -s;
		m.cols[1][2] = s;
	}

	void Matrix44::RotationY(float rads, Matrix44 &m) {
		float c = cos(rads);
		float s = sin(rads);
		m.cols[0][0] = c;
		m.cols[2][2] = c;
		m.cols[0][2] = -s;
		m.cols[2][0] = s;
	}

	void Matrix44::RotationZ(float rads, Matrix44 &m) {
		float c = cos(rads);
		float s = sin(rads);
		m.cols[0][0] = c;
		m.cols[1][1] = c;
		m.cols[1][0] = -s;
		m.cols[0][1] = s;
	}

	void Matrix44::Translation(const Vector4 &trans, Matrix44 &m) {
		m.cols[3][0] = trans.x;
		m.cols[3][1] = trans.y;
		m.cols[3][2] = trans.z;
		m.cols[3][3] = 1.0f;
	}
	void Matrix44::Translation(const Vector3 &trans, Matrix44 &m) {
		m.cols[3][0] = trans.x;
		m.cols[3][1] = trans.y;
		m.cols[3][2] = trans.z;
		m.cols[3][3] = 1.0f;
	}
	void Matrix44::Translation(float x, float y, float z, Matrix44 &m) {
		m.cols[3][0] = x;
		m.cols[3][1] = y;
		m.cols[3][2] = z;
		m.cols[3][3] = 1.0f;
	}

	void Matrix44::Scale(const Vector4 &scale, Matrix44 &m) {
		m.cols[0][0] = scale.x;
		m.cols[1][1] = scale.y;
		m.cols[2][2] = scale.z;
	}
	void Matrix44::Scale(const Vector3 &scale, Matrix44 &m) {
		m.cols[0][0] = scale.x;
		m.cols[1][1] = scale.y;
		m.cols[2][2] = scale.z;
	}
	void Matrix44::Scale(float sx, float sy, float sz, Matrix44 &m) {
		m.cols[0][0] = sx;
		m.cols[1][1] = sy;
		m.cols[2][2] = sz;
	}

	void Matrix44::Transpose(const Matrix44 &m, Matrix44 &out) {
		float temp[4][4];
		memcpy(temp, m.vals, sizeof(m.vals));

		out.cols[0][0] = temp[0][0];
		out.cols[0][1] = temp[1][0];
		out.cols[0][2] = temp[2][0];
		out.cols[0][3] = temp[3][0];
		
		out.cols[1][0] = temp[0][1];
		out.cols[1][1] = temp[1][1];
		out.cols[1][2] = temp[2][1];
		out.cols[1][3] = temp[3][1];

		out.cols[2][0] = temp[0][2];
		out.cols[2][1] = temp[1][2];
		out.cols[2][2] = temp[2][2];
		out.cols[2][3] = temp[3][2];

		out.cols[3][0] = temp[0][3];
		out.cols[3][1] = temp[1][3];
		out.cols[3][2] = temp[2][3];
		out.cols[3][3] = temp[3][3];
	}

	void Matrix44::LookAt(const Vector4 &eye, const Vector4 &target, const Vector4 &up, Matrix44 &out) {
		Vector4 nZ = eye-target;
		nZ.Normalize();
		Vector4 nX = Vector4::Cross(up, nZ);
		nX.Normalize();
		Vector4 nY = Vector4::Cross(nZ, nX);
		
		out.cols[0][0] = nX.vals[0];
		out.cols[0][1] = nY.vals[0];
		out.cols[0][2] = nZ.vals[0];
		out.cols[0][3] = 0.0f;

		out.cols[1][0] = nX.vals[1];
		out.cols[1][1] = nY.vals[1];
		out.cols[1][2] = nZ.vals[1];
		out.cols[1][3] = 0.0f;

		out.cols[2][0] = nX.vals[2];
		out.cols[2][1] = nY.vals[2];
		out.cols[2][2] = nZ.vals[2];
		out.cols[2][3] = 0.0f;

		out.cols[3][0] = -nX.Dot(eye);
		out.cols[3][1] = -nY.Dot(eye);
		out.cols[3][2] = -nZ.Dot(eye);
		out.cols[3][3] = 1.0f;
	}

	void Matrix44::Frustum(float l, float r, float b, float t, float n, float f, Matrix44 &out) {
		out.cols[0][0] = 2.0f * n / (r-l);
		out.cols[0][1] = 0.0f;
		out.cols[0][2] = 0.0f;
		out.cols[0][3] = 0.0f;

		out.cols[1][0] = 0.0f;
		out.cols[1][1] = 2.0f * n / (t-b);
		out.cols[1][2] = 0.0f;
		out.cols[1][3] = 0.0f;
	
		out.cols[2][0] = (r+l) / (r-l);
		out.cols[2][1] = (t+b) / (t-b);
		out.cols[2][2] = f / (n-f);
		out.cols[2][3] = -1.0f;

		out.cols[3][0] = 0.0f;
		out.cols[3][1] = 0.0f;
		out.cols[3][2] = f * n / (n-f);
		out.cols[3][3] = 0.0f;
	}

	void Matrix44::Perspective(float fovY, float aspect, float nearPlane, float farPlane, Matrix44 &out) {
		float t = std::tan(fovY / 2.0f * MAKI_DEG_TO_RAD);
		
		// Half height of near plane
		float hh = nearPlane * t;
		
		// Half width of near plane
		float hw = hh * aspect;
		
		Frustum(-hw, hw, -hh, hh, nearPlane, farPlane, out);
	}

	void Matrix44::Ortho(float l, float r, float b, float t, float n, float f, Matrix44 &out) {
		out.cols[0][0] = 2.0f/(r-l);
		out.cols[0][1] = 0.0f;
		out.cols[0][2] = 0.0f;
		out.cols[0][3] = 0.0f;

		out.cols[1][0] = 0.0f;
		out.cols[1][1] = 2.0f/(t-b);
		out.cols[1][2] = 0.0f;
		out.cols[1][3] = 0.0f;
	
		out.cols[2][0] = 0.0f;
		out.cols[2][1] = 0.0f;
		out.cols[2][2] = 1.0f/(n-f);
		out.cols[2][3] = 0.0f;

		out.cols[3][0] = (l+r)/(l-r);
		out.cols[3][1] = (b+t)/(b-t);
		out.cols[3][2] = n/(n-f);
		out.cols[3][3] = 1.0f;
	}


	// Adapted from Ogre3d's implementation:
	void Matrix44::Inverse(const Matrix44 &m, Matrix44 &out)
	{
		if(m.IsAffine()) {
			AffineInverse(m, out);
			return;
		}

		const float m00 = m.cols[0][0], m01 = m.cols[1][0], m02 = m.cols[2][0], m03 = m.cols[3][0];
        const float m10 = m.cols[0][1], m11 = m.cols[1][1], m12 = m.cols[2][1], m13 = m.cols[3][1];
        const float m20 = m.cols[0][2], m21 = m.cols[1][2], m22 = m.cols[2][2], m23 = m.cols[3][2];
        const float m30 = m.cols[0][3], m31 = m.cols[1][3], m32 = m.cols[2][3], m33 = m.cols[3][3];

        float v0 = m20 * m31 - m21 * m30;
        float v1 = m20 * m32 - m22 * m30;
        float v2 = m20 * m33 - m23 * m30;
        float v3 = m21 * m32 - m22 * m31;
        float v4 = m21 * m33 - m23 * m31;
        float v5 = m22 * m33 - m23 * m32;

        const float t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
        const float t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
        const float t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
        const float t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

        const float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

        const float d00 = t00 * invDet;
        const float d10 = t10 * invDet;
        const float d20 = t20 * invDet;
        const float d30 = t30 * invDet;

        const float d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        const float d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        const float d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        const float d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        v0 = m10 * m31 - m11 * m30;
        v1 = m10 * m32 - m12 * m30;
        v2 = m10 * m33 - m13 * m30;
        v3 = m11 * m32 - m12 * m31;
        v4 = m11 * m33 - m13 * m31;
        v5 = m12 * m33 - m13 * m32;

        const float d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        const float d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        const float d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        const float d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        v0 = m21 * m10 - m20 * m11;
        v1 = m22 * m10 - m20 * m12;
        v2 = m23 * m10 - m20 * m13;
        v3 = m22 * m11 - m21 * m12;
        v4 = m23 * m11 - m21 * m13;
        v5 = m23 * m12 - m22 * m13;

        const float d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        const float d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        const float d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        const float d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

		out.cols[0][0] = d00;	out.cols[1][0] = d01;	out.cols[2][0] = d02;	out.cols[3][0] = d03;
		out.cols[0][1] = d10;	out.cols[1][1] = d11;	out.cols[2][1] = d12;	out.cols[3][1] = d13;
		out.cols[0][2] = d20;	out.cols[1][2] = d21;	out.cols[2][2] = d22;	out.cols[3][2] = d23;
		out.cols[0][3] = d30;	out.cols[1][3] = d31;	out.cols[2][3] = d32;	out.cols[3][3] = d33;
	}

	void Matrix44::AffineInverse(const Matrix44 &m, Matrix44 &out)
	{
		static const Vector4 zzz1 = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		const Vector4 translation(m.cols[3][0], m.cols[3][1], m.cols[3][2], m.cols[3][3]);

		// Calculate the inverse of the rotation part
		// Also empty out the translation portions
		Transpose(m, out);
		out.cols[0][3] = 0.0f;
		out.cols[1][3] = 0.0f;
		out.cols[2][3] = 0.0f;
		out.cols[3][3] = 1.0f;

		out.cols[3][0] = 0.0f;
		out.cols[3][1] = 0.0f;
		out.cols[3][2] = 0.0f;
		out.cols[3][3] = 1.0f;
		
		// Use the inverse rotation to calculate the inverse translation
		Vector4 temp = out * -translation;
		out.cols[3][0] = temp.x;
		out.cols[3][1] = temp.y;
		out.cols[3][2] = temp.z;
		out.cols[3][3] = temp.w;

		out.cols[0][3] = 0.0f;
		out.cols[1][3] = 0.0f;
		out.cols[2][3] = 0.0f;
		out.cols[3][3] = 1.0f;
	}

} // namespace Maki