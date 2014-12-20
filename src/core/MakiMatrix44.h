#pragma once
#include <xmmintrin.h>
#include <cstring>
#include "core/MakiAllocator.h"
#include "core/MakiVector4.h"
#include "core/MakiVector3.h"
#include "core/MakiFrustum.h"

namespace maki
{
	namespace core
	{

		class __declspec(align(MAKI_SIMD_ALIGN)) matrix44_t : public aligned_t<MAKI_SIMD_ALIGN>
		{
		public:
			static void rotation_x(float rads, matrix44_t &m);
			static void rotation_y(float rads, matrix44_t &m);
			static void rotation_z(float rads, matrix44_t &m);

			static void translation(const vector4_t &trans, matrix44_t &m);
			static void translation(const vector3_t &trans, matrix44_t &m);
			static void translation(float x, float y, float z, matrix44_t &m);
		
			static void scale(const vector4_t &scale, matrix44_t &m);
			static void scale(const vector3_t &scale, matrix44_t &m);
			static void scale(float sx, float sy, float sz, matrix44_t &m);

			static void transpose(const matrix44_t &m, matrix44_t &out);
			static void inverse(const matrix44_t &m, matrix44_t &out);
			static void affine_inverse(const matrix44_t &m, matrix44_t &out);
			static void look_at(const vector4_t &eye, const vector4_t &target, const vector4_t &up, matrix44_t &out);

			static void frustum(float l, float r, float b, float t, float n, float f, matrix44_t &m);
			inline static void frustum(const core::frustum_t &f, matrix44_t &m) { matrix44_t::frustum(f.left_, f.right_, f.bottom_, f.top_, f.near_plane_, f.far_plane_, m); }
		
			static void perspective(float fovY, float aspect, float near_plane_, float far_plane_, matrix44_t &m);
			inline static void perspective(const core::frustum_t &f, matrix44_t &m) { perspective(f.fov_, f.get_aspect(), f.near_plane_, f.far_plane_, m); }

			static void ortho(float l, float r, float b, float t, float n, float f, matrix44_t &m);
			inline static void ortho(const core::frustum_t &f, matrix44_t &m) { ortho(f.left_, f.right_, f.bottom_, f.top_, f.near_plane_, f.far_plane_, m); }

			static const matrix44_t identity_;

		public:
			inline matrix44_t() {}
			inline matrix44_t(bool init) { if(init) set_identity(); }
			inline matrix44_t(const matrix44_t &m) { memcpy(vals_, m.vals_, sizeof(vals_)); }

			inline matrix44_t(const vector4_t &col0, const vector4_t &col1, const vector4_t &col2, const vector4_t &col3)
			{
				this->cols_[0][0] = col0.vals_[0];
				this->cols_[0][1] = col0.vals_[1];
				this->cols_[0][2] = col0.vals_[2];
				this->cols_[0][3] = col0.vals_[3];
		
				this->cols_[1][0] = col1.vals_[0];
				this->cols_[1][1] = col1.vals_[1];
				this->cols_[1][2] = col1.vals_[2];
				this->cols_[1][3] = col1.vals_[3];

				this->cols_[2][0] = col2.vals_[0];
				this->cols_[2][1] = col2.vals_[1];
				this->cols_[2][2] = col2.vals_[2];
				this->cols_[2][3] = col2.vals_[3];

				this->cols_[3][0] = col3.vals_[0];
				this->cols_[3][1] = col3.vals_[1];
				this->cols_[3][2] = col3.vals_[2];
				this->cols_[3][3] = col3.vals_[3];
			}

			inline void set_identity()
			{
				cols_[0][0] = 1.0f;
				cols_[0][1] = 0.0f;
				cols_[0][2] = 0.0f;
				cols_[0][3] = 0.0f;

				cols_[1][0] = 0.0f;
				cols_[1][1] = 1.0f;
				cols_[1][2] = 0.0f;
				cols_[1][3] = 0.0f;

				cols_[2][0] = 0.0f;
				cols_[2][1] = 0.0f;
				cols_[2][2] = 1.0f;
				cols_[2][3] = 0.0f;

				cols_[3][0] = 0.0f;
				cols_[3][1] = 0.0f;
				cols_[3][2] = 0.0f;
				cols_[3][3] = 1.0f;
			}

			inline vector4_t get_row(uint32 row) const
			{
				return vector4_t(cols_[0][row], cols_[1][row], cols_[2][row], cols_[3][row]);
			}
		
			inline void set_row(uint32 row, const vector4_t &v)
			{
				cols_[0][row] = v.x;
				cols_[1][row] = v.y;
				cols_[2][row] = v.z;
				cols_[3][row] = v.w;
			}

			inline bool is_affine() const
			{
				return cols_[0][3] == 0.0f && cols_[1][3] == 0.0f && cols_[2][3] == 0.0f && cols_[3][3] == 1.0f;
			}

			inline bool is_identity() const
			{
				return memcmp(vals_, identity_.vals_, sizeof(vals_)) == 0;
			}


			// Operator overloads
			inline matrix44_t operator*(float s) const
			{
				matrix44_t ret;
				for(uint32 i = 0; i < 16; i++) {
					ret.vals_[i] = vals_[i] * s;
				}
				return ret;
			}

			inline void operator*=(float s)
			{
				for(uint32 i = 0; i < 16; i++) {
					vals_[i] *= s;
				}
			}

			inline vector4_t operator*(const vector4_t &v) const
			{
				return vector4_t(cols_[0][0]*v.x + cols_[1][0]*v.y + cols_[2][0]*v.z + cols_[3][0]*v.w,
					cols_[0][1]*v.x + cols_[1][1]*v.y + cols_[2][1]*v.z + cols_[3][1]*v.w,
					cols_[0][2]*v.x + cols_[1][2]*v.y + cols_[2][2]*v.z + cols_[3][2]*v.w,
					cols_[0][3]*v.x + cols_[1][3]*v.y + cols_[2][3]*v.z + cols_[3][3]*v.w);
			}

			inline vector3_t operator*(const vector3_t &v) const
			{
				// Cheating:
				// This operation doesn't really exist in a mathematical sense.  Pretend that the vector
				// has a 1 in the fourth component, so it picks up the translation part of the matrix.
				return vector3_t(cols_[0][0]*v.x + cols_[1][0]*v.y + cols_[2][0]*v.z + cols_[3][0],
					cols_[0][1]*v.x + cols_[1][1]*v.y + cols_[2][1]*v.z + cols_[3][1],
					cols_[0][2]*v.x + cols_[1][2]*v.y + cols_[2][2]*v.z + cols_[3][2]);
			}

			inline matrix44_t operator*(const matrix44_t &v) const
			{
				matrix44_t ret;
			
#if MAKI_USE_SIMD
				const __m128 a = *(__m128 *)cols_[0];
				const __m128 b = *(__m128 *)cols_[1];
				const __m128 c = *(__m128 *)cols_[2];
				const __m128 d = *(__m128 *)cols_[3];
			
				__m128 t, t2;

				t = _mm_set1_ps(v.cols_[0][0]);
				t2 = _mm_mul_ps(a,t);
				t = _mm_set1_ps(v.cols_[0][1]);
				t2 = _mm_add_ps(_mm_mul_ps(b,t),t2);
				t = _mm_set1_ps(v.cols_[0][2]);
				t2 = _mm_add_ps(_mm_mul_ps(c,t),t2);
				t = _mm_set1_ps(v.cols_[0][3]);
				t2 = _mm_add_ps(_mm_mul_ps(d,t),t2);
				_mm_store_ps(ret.cols_[0],t2);

				t = _mm_set1_ps(v.cols_[1][0]);
				t2 = _mm_mul_ps(a,t);
				t = _mm_set1_ps(v.cols_[1][1]);
				t2 = _mm_add_ps(_mm_mul_ps(b,t),t2);
				t = _mm_set1_ps(v.cols_[1][2]);
				t2 = _mm_add_ps(_mm_mul_ps(c,t),t2);
				t = _mm_set1_ps(v.cols_[1][3]);
				t2 = _mm_add_ps(_mm_mul_ps(d,t),t2);
				_mm_store_ps(ret.cols_[1],t2);

				t = _mm_set1_ps(v.cols_[2][0]);
				t2 = _mm_mul_ps(a,t);
				t = _mm_set1_ps(v.cols_[2][1]);
				t2 = _mm_add_ps(_mm_mul_ps(b,t),t2);
				t = _mm_set1_ps(v.cols_[2][2]);
				t2 = _mm_add_ps(_mm_mul_ps(c,t),t2);
				t = _mm_set1_ps(v.cols_[2][3]);
				t2 = _mm_add_ps(_mm_mul_ps(d,t),t2);
				_mm_store_ps(ret.cols_[2],t2);

				t = _mm_set1_ps(v.cols_[3][0]);
				t2 = _mm_mul_ps(a,t);
				t = _mm_set1_ps(v.cols_[3][1]);
				t2 = _mm_add_ps(_mm_mul_ps(b,t),t2);
				t = _mm_set1_ps(v.cols_[3][2]);
				t2 = _mm_add_ps(_mm_mul_ps(c,t),t2);
				t = _mm_set1_ps(v.cols_[3][3]);
				t2 = _mm_add_ps(_mm_mul_ps(d,t),t2);
				_mm_store_ps(ret.cols_[3],t2);

#else

				ret.cols_[0][0] = cols_[0][0] * v.cols_[0][0] + cols_[1][0] * v.cols_[0][1] + cols_[2][0] * v.cols_[0][2] + cols_[3][0] * v.cols_[0][3];
				ret.cols_[0][1] = cols_[0][1] * v.cols_[0][0] + cols_[1][1] * v.cols_[0][1] + cols_[2][1] * v.cols_[0][2] + cols_[3][1] * v.cols_[0][3];
				ret.cols_[0][2] = cols_[0][2] * v.cols_[0][0] + cols_[1][2] * v.cols_[0][1] + cols_[2][2] * v.cols_[0][2] + cols_[3][2] * v.cols_[0][3];
				ret.cols_[0][3] = cols_[0][3] * v.cols_[0][0] + cols_[1][3] * v.cols_[0][1] + cols_[2][3] * v.cols_[0][2] + cols_[3][3] * v.cols_[0][3];

				ret.cols_[1][0] = cols_[0][0] * v.cols_[1][0] + cols_[1][0] * v.cols_[1][1] + cols_[2][0] * v.cols_[1][2] + cols_[3][0] * v.cols_[1][3];
				ret.cols_[1][1] = cols_[0][1] * v.cols_[1][0] + cols_[1][1] * v.cols_[1][1] + cols_[2][1] * v.cols_[1][2] + cols_[3][1] * v.cols_[1][3];
				ret.cols_[1][2] = cols_[0][2] * v.cols_[1][0] + cols_[1][2] * v.cols_[1][1] + cols_[2][2] * v.cols_[1][2] + cols_[3][2] * v.cols_[1][3];
				ret.cols_[1][3] = cols_[0][3] * v.cols_[1][0] + cols_[1][3] * v.cols_[1][1] + cols_[2][3] * v.cols_[1][2] + cols_[3][3] * v.cols_[1][3];

				ret.cols_[2][0] = cols_[0][0] * v.cols_[2][0] + cols_[1][0] * v.cols_[2][1] + cols_[2][0] * v.cols_[2][2] + cols_[3][0] * v.cols_[2][3];
				ret.cols_[2][1] = cols_[0][1] * v.cols_[2][0] + cols_[1][1] * v.cols_[2][1] + cols_[2][1] * v.cols_[2][2] + cols_[3][1] * v.cols_[2][3];
				ret.cols_[2][2] = cols_[0][2] * v.cols_[2][0] + cols_[1][2] * v.cols_[2][1] + cols_[2][2] * v.cols_[2][2] + cols_[3][2] * v.cols_[2][3];
				ret.cols_[2][3] = cols_[0][3] * v.cols_[2][0] + cols_[1][3] * v.cols_[2][1] + cols_[2][3] * v.cols_[2][2] + cols_[3][3] * v.cols_[2][3];

				ret.cols_[3][0] = cols_[0][0] * v.cols_[3][0] + cols_[1][0] * v.cols_[3][1] + cols_[2][0] * v.cols_[3][2] + cols_[3][0] * v.cols_[3][3];
				ret.cols_[3][1] = cols_[0][1] * v.cols_[3][0] + cols_[1][1] * v.cols_[3][1] + cols_[2][1] * v.cols_[3][2] + cols_[3][1] * v.cols_[3][3];
				ret.cols_[3][2] = cols_[0][2] * v.cols_[3][0] + cols_[1][2] * v.cols_[3][1] + cols_[2][2] * v.cols_[3][2] + cols_[3][2] * v.cols_[3][3];
				ret.cols_[3][3] = cols_[0][3] * v.cols_[3][0] + cols_[1][3] * v.cols_[3][1] + cols_[2][3] * v.cols_[3][2] + cols_[3][3] * v.cols_[3][3];

#endif
				return ret;
			}

			inline void transpose()
			{
				transpose(*this, *this);
			}
		
		public:
			union {
				float cols_[4][4];
				float vals_[16];
			};
		};

		matrix44_t operator*(float s, const matrix44_t &m);

	} // namespace core

} // namespace maki
