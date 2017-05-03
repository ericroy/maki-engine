#include "core/MakiMatrix44.h"
#include <cmath>

namespace maki {
	namespace core {

		const matrix44_t matrix44_t::identity_(true);
	
		void matrix44_t::rotation_x(float rads, matrix44_t &m) {
			float c = cos(rads);
			float s = sin(rads);
			m.cols[1][1] = c;
			m.cols[2][2] = c;
			m.cols[2][1] = -s;
			m.cols[1][2] = s;
		}

		void matrix44_t::rotation_y(float rads, matrix44_t &m) {
			float c = cos(rads);
			float s = sin(rads);
			m.cols[0][0] = c;
			m.cols[2][2] = c;
			m.cols[0][2] = -s;
			m.cols[2][0] = s;
		}

		void matrix44_t::rotation_z(float rads, matrix44_t &m) {
			float c = cos(rads);
			float s = sin(rads);
			m.cols[0][0] = c;
			m.cols[1][1] = c;
			m.cols[1][0] = -s;
			m.cols[0][1] = s;
		}

		void matrix44_t::translation(const vector4_t &trans, matrix44_t &m) {
			m.cols[3][0] = trans.x;
			m.cols[3][1] = trans.y;
			m.cols[3][2] = trans.z;
			m.cols[3][3] = 1.0f;
		}
		void matrix44_t::translation(const vector3_t &trans, matrix44_t &m) {
			m.cols[3][0] = trans.x;
			m.cols[3][1] = trans.y;
			m.cols[3][2] = trans.z;
			m.cols[3][3] = 1.0f;
		}
		void matrix44_t::translation(float x, float y, float z, matrix44_t &m) {
			m.cols[3][0] = x;
			m.cols[3][1] = y;
			m.cols[3][2] = z;
			m.cols[3][3] = 1.0f;
		}

		void matrix44_t::scale(const vector4_t &scale, matrix44_t &m) {
			m.cols[0][0] = scale.x;
			m.cols[1][1] = scale.y;
			m.cols[2][2] = scale.z;
		}
		void matrix44_t::scale(const vector3_t &scale, matrix44_t &m) {
			m.cols[0][0] = scale.x;
			m.cols[1][1] = scale.y;
			m.cols[2][2] = scale.z;
		}
		void matrix44_t::scale(float sx, float sy, float sz, matrix44_t &m) {
			m.cols[0][0] = sx;
			m.cols[1][1] = sy;
			m.cols[2][2] = sz;
		}

		void matrix44_t::transpose(const matrix44_t &m, matrix44_t &out) {
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

		void matrix44_t::look_at(const vector4_t &eye, const vector4_t &target, const vector4_t &up, matrix44_t &out) {
			vector4_t nz = eye-target;
			nz.normalize();
			vector4_t nx = vector4_t::cross(up, nz);
			nx.normalize();
			vector4_t ny = vector4_t::cross(nz, nx);
		
			out.cols[0][0] = nx.vals[0];
			out.cols[0][1] = ny.vals[0];
			out.cols[0][2] = nz.vals[0];
			out.cols[0][3] = 0.0f;

			out.cols[1][0] = nx.vals[1];
			out.cols[1][1] = ny.vals[1];
			out.cols[1][2] = nz.vals[1];
			out.cols[1][3] = 0.0f;

			out.cols[2][0] = nx.vals[2];
			out.cols[2][1] = ny.vals[2];
			out.cols[2][2] = nz.vals[2];
			out.cols[2][3] = 0.0f;

			out.cols[3][0] = -nx.dot(eye);
			out.cols[3][1] = -ny.dot(eye);
			out.cols[3][2] = -nz.dot(eye);
			out.cols[3][3] = 1.0f;
		}

		void matrix44_t::frustum(float l, float r, float b, float t, float n, float f, matrix44_t &out) {
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

		void matrix44_t::perspective(float fov_y, float aspect, float near_plane, float far_plane, matrix44_t &out) {
			float t = std::tan(fov_y / 2.0f * MAKI_DEG_TO_RAD);
		
			// Half height of near plane
			float hh = near_plane * t;
		
			// Half width of near plane
			float hw = hh * aspect;
		
			frustum(-hw, hw, -hh, hh, near_plane, far_plane, out);
		}

		void matrix44_t::ortho(float l, float r, float b, float t, float n, float f, matrix44_t &out) {
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
		void matrix44_t::inverse(const matrix44_t &m, matrix44_t &out)
		{
			if(m.is_affine()) {
				affine_inverse(m, out);
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

			const float inv_det = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

			const float d00 = t00 * inv_det;
			const float d10 = t10 * inv_det;
			const float d20 = t20 * inv_det;
			const float d30 = t30 * inv_det;

			const float d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * inv_det;
			const float d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * inv_det;
			const float d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * inv_det;
			const float d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * inv_det;

			v0 = m10 * m31 - m11 * m30;
			v1 = m10 * m32 - m12 * m30;
			v2 = m10 * m33 - m13 * m30;
			v3 = m11 * m32 - m12 * m31;
			v4 = m11 * m33 - m13 * m31;
			v5 = m12 * m33 - m13 * m32;

			const float d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * inv_det;
			const float d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * inv_det;
			const float d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * inv_det;
			const float d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * inv_det;

			v0 = m21 * m10 - m20 * m11;
			v1 = m22 * m10 - m20 * m12;
			v2 = m23 * m10 - m20 * m13;
			v3 = m22 * m11 - m21 * m12;
			v4 = m23 * m11 - m21 * m13;
			v5 = m23 * m12 - m22 * m13;

			const float d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * inv_det;
			const float d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * inv_det;
			const float d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * inv_det;
			const float d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * inv_det;

			out.cols[0][0] = d00;	out.cols[1][0] = d01;	out.cols[2][0] = d02;	out.cols[3][0] = d03;
			out.cols[0][1] = d10;	out.cols[1][1] = d11;	out.cols[2][1] = d12;	out.cols[3][1] = d13;
			out.cols[0][2] = d20;	out.cols[1][2] = d21;	out.cols[2][2] = d22;	out.cols[3][2] = d23;
			out.cols[0][3] = d30;	out.cols[1][3] = d31;	out.cols[2][3] = d32;	out.cols[3][3] = d33;
		}

		//void matrix44_t::affine_inverse(const matrix44_t &m, matrix44_t &out)
		//{
		//	const vector4_t translation(m.cols[3][0], m.cols[3][1], m.cols[3][2], m.cols[3][3]);

		//	// Calculate the inverse of the rotation part
		//	// Also empty out the translation portions
		//	transpose(m, out);
		//	out.cols[0][3] = 0.0f;
		//	out.cols[1][3] = 0.0f;
		//	out.cols[2][3] = 0.0f;
		//	out.cols[3][3] = 1.0f;

		//	out.cols[3][0] = 0.0f;
		//	out.cols[3][1] = 0.0f;
		//	out.cols[3][2] = 0.0f;
		//	out.cols[3][3] = 1.0f;
		//	
		//	// Use the inverse rotation to calculate the inverse translation
		//	vector4_t temp = out * -translation;
		//	out.cols[3][0] = temp.x;
		//	out.cols[3][1] = temp.y;
		//	out.cols[3][2] = temp.z;
		//	out.cols[3][3] = temp.w_;

		//	out.cols[0][3] = 0.0f;
		//	out.cols[1][3] = 0.0f;
		//	out.cols[2][3] = 0.0f;
		//	out.cols[3][3] = 1.0f;
		//}

		// Adapted from Ogre3d's implementation:
		void matrix44_t::affine_inverse(const matrix44_t &m, matrix44_t &out)
		{
			float m10 = m.cols[0][1], m11 = m.cols[1][1], m12 = m.cols[2][1];
			float m20 = m.cols[0][2], m21 = m.cols[1][2], m22 = m.cols[2][2];

			float t00 = m22 * m11 - m21 * m12;
			float t10 = m20 * m12 - m22 * m10;
			float t20 = m21 * m10 - m20 * m11;

			float m00 = m.cols[0][0], m01 = m.cols[1][0], m02 = m.cols[2][0];

			float inv_det = 1.0f / (m00 * t00 + m01 * t10 + m02 * t20);

			t00 *= inv_det; t10 *= inv_det; t20 *= inv_det;

			m00 *= inv_det; m01 *= inv_det; m02 *= inv_det;

			float r00 = t00;
			float r01 = m02 * m21 - m01 * m22;
			float r02 = m01 * m12 - m02 * m11;

			float r10 = t10;
			float r11 = m00 * m22 - m02 * m20;
			float r12 = m02 * m10 - m00 * m12;

			float r20 = t20;
			float r21 = m01 * m20 - m00 * m21;
			float r22 = m00 * m11 - m01 * m10;

			float m03 = m.cols[3][0], m13 = m.cols[3][1], m23 = m.cols[3][2];

			float r03 = - (r00 * m03 + r01 * m13 + r02 * m23);
			float r13 = - (r10 * m03 + r11 * m13 + r12 * m23);
			float r23 = - (r20 * m03 + r21 * m13 + r22 * m23);

			out.cols[0][0] = r00;
			out.cols[0][1] = r10;
			out.cols[0][2] = r20;
			out.cols[0][3] = 0.0f;

			out.cols[1][0] = r01;
			out.cols[1][1] = r11;
			out.cols[1][2] = r21;
			out.cols[1][3] = 0.0f;

			out.cols[2][0] = r02;
			out.cols[2][1] = r12;
			out.cols[2][2] = r22;
			out.cols[2][3] = 0.0f;

			out.cols[3][0] = r03;
			out.cols[3][1] = r13;
			out.cols[3][2] = r23;
			out.cols[3][3] = 1.0f;
		}



	} // namespace core
} // namespace maki
