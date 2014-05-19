#pragma once
#include <xmmintrin.h>
#include <cstring>
#include "core/MakiAllocator.h"
#include "core/MakiVector4.h"
#include "core/MakiVector3.h"
#include "core/MakiFrustum.h"

namespace Maki
{
	namespace Core
	{

		class __declspec(align(SIMD_ALIGN)) Matrix44 : public Aligned<SIMD_ALIGN>
		{
		public:
			static void RotationX(float rads, Matrix44 &m);
			static void RotationY(float rads, Matrix44 &m);
			static void RotationZ(float rads, Matrix44 &m);

			static void Translation(const Vector4 &trans, Matrix44 &m);
			static void Translation(const Vector3 &trans, Matrix44 &m);
			static void Translation(float x, float y, float z, Matrix44 &m);
		
			static void Scale(const Vector4 &scale, Matrix44 &m);
			static void Scale(const Vector3 &scale, Matrix44 &m);
			static void Scale(float sx, float sy, float sz, Matrix44 &m);

			static void Transpose(const Matrix44 &m, Matrix44 &out);
			static void Inverse(const Matrix44 &m, Matrix44 &out);
			static void AffineInverse(const Matrix44 &m, Matrix44 &out);
			static void LookAt(const Vector4 &eye, const Vector4 &target, const Vector4 &up, Matrix44 &out);

			static void Frustum(float l, float r, float b, float t, float n, float f, Matrix44 &m);
			inline static void Frustum(const Core::Frustum &f, Matrix44 &m) { Matrix44::Frustum(f.left, f.right, f.bottom, f.top, f.nearPlane, f.farPlane, m); }
		
			static void Perspective(float fovY, float aspect, float nearPlane, float farPlane, Matrix44 &m);
			inline static void Perspective(const Core::Frustum &f, Matrix44 &m) { Perspective(f.fov, f.GetAspect(), f.nearPlane, f.farPlane, m); }

			static void Ortho(float l, float r, float b, float t, float n, float f, Matrix44 &m);
			inline static void Ortho(const Core::Frustum &f, Matrix44 &m) { Ortho(f.left, f.right, f.bottom, f.top, f.nearPlane, f.farPlane, m); }

			static const Matrix44 Identity;

		public:
			inline Matrix44() {}
			inline Matrix44(bool init) { if(init) SetIdentity(); }
			inline Matrix44(const Matrix44 &m) { memcpy(vals, m.vals, sizeof(vals)); }

			inline Matrix44(const Vector4 &col0, const Vector4 &col1, const Vector4 &col2, const Vector4 &col3)
			{
				this->cols[0][0] = col0.vals[0];
				this->cols[0][1] = col0.vals[1];
				this->cols[0][2] = col0.vals[2];
				this->cols[0][3] = col0.vals[3];
		
				this->cols[1][0] = col1.vals[0];
				this->cols[1][1] = col1.vals[1];
				this->cols[1][2] = col1.vals[2];
				this->cols[1][3] = col1.vals[3];

				this->cols[2][0] = col2.vals[0];
				this->cols[2][1] = col2.vals[1];
				this->cols[2][2] = col2.vals[2];
				this->cols[2][3] = col2.vals[3];

				this->cols[3][0] = col3.vals[0];
				this->cols[3][1] = col3.vals[1];
				this->cols[3][2] = col3.vals[2];
				this->cols[3][3] = col3.vals[3];
			}

			inline void SetIdentity()
			{
				cols[0][0] = 1.0f;
				cols[0][1] = 0.0f;
				cols[0][2] = 0.0f;
				cols[0][3] = 0.0f;

				cols[1][0] = 0.0f;
				cols[1][1] = 1.0f;
				cols[1][2] = 0.0f;
				cols[1][3] = 0.0f;

				cols[2][0] = 0.0f;
				cols[2][1] = 0.0f;
				cols[2][2] = 1.0f;
				cols[2][3] = 0.0f;

				cols[3][0] = 0.0f;
				cols[3][1] = 0.0f;
				cols[3][2] = 0.0f;
				cols[3][3] = 1.0f;
			}

			inline Vector4 GetRow(uint32 row) const
			{
				return Vector4(cols[0][row], cols[1][row], cols[2][row], cols[3][row]);
			}
		
			inline void SetRow(uint32 row, const Vector4 &v)
			{
				cols[0][row] = v.x;
				cols[1][row] = v.y;
				cols[2][row] = v.z;
				cols[3][row] = v.w;
			}

			inline bool IsAffine() const
			{
				return cols[0][3] == 0.0f && cols[1][3] == 0.0f && cols[2][3] == 0.0f && cols[3][3] == 1.0f;
			}

			inline bool IsIdentity() const
			{
				return memcmp(vals, Identity.vals, sizeof(vals)) == 0;
			}


			// Operator overloads
			inline Matrix44 operator*(float s) const
			{
				Matrix44 ret;
				for(uint32 i = 0; i < 16; i++) {
					ret.vals[i] = vals[i] * s;
				}
				return ret;
			}

			inline void operator*=(float s)
			{
				for(uint32 i = 0; i < 16; i++) {
					vals[i] *= s;
				}
			}

			inline Vector4 operator*(const Vector4 &v) const
			{
				return Vector4(cols[0][0]*v.x + cols[1][0]*v.y + cols[2][0]*v.z + cols[3][0]*v.w,
					cols[0][1]*v.x + cols[1][1]*v.y + cols[2][1]*v.z + cols[3][1]*v.w,
					cols[0][2]*v.x + cols[1][2]*v.y + cols[2][2]*v.z + cols[3][2]*v.w,
					cols[0][3]*v.x + cols[1][3]*v.y + cols[2][3]*v.z + cols[3][3]*v.w);
			}

			inline Vector3 operator*(const Vector3 &v) const
			{
				// Cheating:
				// This operation doesn't really exist in a mathematical sense.  Pretend that the vector
				// has a 1 in the fourth component, so it picks up the translation part of the matrix.
				return Vector3(cols[0][0]*v.x + cols[1][0]*v.y + cols[2][0]*v.z + cols[3][0],
					cols[0][1]*v.x + cols[1][1]*v.y + cols[2][1]*v.z + cols[3][1],
					cols[0][2]*v.x + cols[1][2]*v.y + cols[2][2]*v.z + cols[3][2]);
			}

			inline Matrix44 operator*(const Matrix44 &v) const
			{
				Matrix44 ret;
			
#if MAKI_USE_SIMD
				const __m128 a = *(__m128 *)cols[0];
				const __m128 b = *(__m128 *)cols[1];
				const __m128 c = *(__m128 *)cols[2];
				const __m128 d = *(__m128 *)cols[3];
			
				__m128 t, t2;

				t = _mm_set1_ps(v.cols[0][0]);
				t2 = _mm_mul_ps(a,t);
				t = _mm_set1_ps(v.cols[0][1]);
				t2 = _mm_add_ps(_mm_mul_ps(b,t),t2);
				t = _mm_set1_ps(v.cols[0][2]);
				t2 = _mm_add_ps(_mm_mul_ps(c,t),t2);
				t = _mm_set1_ps(v.cols[0][3]);
				t2 = _mm_add_ps(_mm_mul_ps(d,t),t2);
				_mm_store_ps(ret.cols[0],t2);

				t = _mm_set1_ps(v.cols[1][0]);
				t2 = _mm_mul_ps(a,t);
				t = _mm_set1_ps(v.cols[1][1]);
				t2 = _mm_add_ps(_mm_mul_ps(b,t),t2);
				t = _mm_set1_ps(v.cols[1][2]);
				t2 = _mm_add_ps(_mm_mul_ps(c,t),t2);
				t = _mm_set1_ps(v.cols[1][3]);
				t2 = _mm_add_ps(_mm_mul_ps(d,t),t2);
				_mm_store_ps(ret.cols[1],t2);

				t = _mm_set1_ps(v.cols[2][0]);
				t2 = _mm_mul_ps(a,t);
				t = _mm_set1_ps(v.cols[2][1]);
				t2 = _mm_add_ps(_mm_mul_ps(b,t),t2);
				t = _mm_set1_ps(v.cols[2][2]);
				t2 = _mm_add_ps(_mm_mul_ps(c,t),t2);
				t = _mm_set1_ps(v.cols[2][3]);
				t2 = _mm_add_ps(_mm_mul_ps(d,t),t2);
				_mm_store_ps(ret.cols[2],t2);

				t = _mm_set1_ps(v.cols[3][0]);
				t2 = _mm_mul_ps(a,t);
				t = _mm_set1_ps(v.cols[3][1]);
				t2 = _mm_add_ps(_mm_mul_ps(b,t),t2);
				t = _mm_set1_ps(v.cols[3][2]);
				t2 = _mm_add_ps(_mm_mul_ps(c,t),t2);
				t = _mm_set1_ps(v.cols[3][3]);
				t2 = _mm_add_ps(_mm_mul_ps(d,t),t2);
				_mm_store_ps(ret.cols[3],t2);

#else

				ret.cols[0][0] = cols[0][0] * v.cols[0][0] + cols[1][0] * v.cols[0][1] + cols[2][0] * v.cols[0][2] + cols[3][0] * v.cols[0][3];
				ret.cols[0][1] = cols[0][1] * v.cols[0][0] + cols[1][1] * v.cols[0][1] + cols[2][1] * v.cols[0][2] + cols[3][1] * v.cols[0][3];
				ret.cols[0][2] = cols[0][2] * v.cols[0][0] + cols[1][2] * v.cols[0][1] + cols[2][2] * v.cols[0][2] + cols[3][2] * v.cols[0][3];
				ret.cols[0][3] = cols[0][3] * v.cols[0][0] + cols[1][3] * v.cols[0][1] + cols[2][3] * v.cols[0][2] + cols[3][3] * v.cols[0][3];

				ret.cols[1][0] = cols[0][0] * v.cols[1][0] + cols[1][0] * v.cols[1][1] + cols[2][0] * v.cols[1][2] + cols[3][0] * v.cols[1][3];
				ret.cols[1][1] = cols[0][1] * v.cols[1][0] + cols[1][1] * v.cols[1][1] + cols[2][1] * v.cols[1][2] + cols[3][1] * v.cols[1][3];
				ret.cols[1][2] = cols[0][2] * v.cols[1][0] + cols[1][2] * v.cols[1][1] + cols[2][2] * v.cols[1][2] + cols[3][2] * v.cols[1][3];
				ret.cols[1][3] = cols[0][3] * v.cols[1][0] + cols[1][3] * v.cols[1][1] + cols[2][3] * v.cols[1][2] + cols[3][3] * v.cols[1][3];

				ret.cols[2][0] = cols[0][0] * v.cols[2][0] + cols[1][0] * v.cols[2][1] + cols[2][0] * v.cols[2][2] + cols[3][0] * v.cols[2][3];
				ret.cols[2][1] = cols[0][1] * v.cols[2][0] + cols[1][1] * v.cols[2][1] + cols[2][1] * v.cols[2][2] + cols[3][1] * v.cols[2][3];
				ret.cols[2][2] = cols[0][2] * v.cols[2][0] + cols[1][2] * v.cols[2][1] + cols[2][2] * v.cols[2][2] + cols[3][2] * v.cols[2][3];
				ret.cols[2][3] = cols[0][3] * v.cols[2][0] + cols[1][3] * v.cols[2][1] + cols[2][3] * v.cols[2][2] + cols[3][3] * v.cols[2][3];

				ret.cols[3][0] = cols[0][0] * v.cols[3][0] + cols[1][0] * v.cols[3][1] + cols[2][0] * v.cols[3][2] + cols[3][0] * v.cols[3][3];
				ret.cols[3][1] = cols[0][1] * v.cols[3][0] + cols[1][1] * v.cols[3][1] + cols[2][1] * v.cols[3][2] + cols[3][1] * v.cols[3][3];
				ret.cols[3][2] = cols[0][2] * v.cols[3][0] + cols[1][2] * v.cols[3][1] + cols[2][2] * v.cols[3][2] + cols[3][2] * v.cols[3][3];
				ret.cols[3][3] = cols[0][3] * v.cols[3][0] + cols[1][3] * v.cols[3][1] + cols[2][3] * v.cols[3][2] + cols[3][3] * v.cols[3][3];

#endif
				return ret;
			}

			inline void Transpose()
			{
				Transpose(*this, *this);
			}
		
		public:
			union {
				float cols[4][4];
				float vals[16];
			};
		};

		Matrix44 operator*(float s, const Matrix44 &m);

	} // namespace Core

} // namespace Maki