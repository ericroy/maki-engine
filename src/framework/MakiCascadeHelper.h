#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{
	namespace Framework
	{
	
		template<int CASCADE_COUNT>
		class CascadeHelper
		{
		private:
			struct Cascade
			{
				Vector4 verts[8];
			};

		public:
			CascadeHelper()
			{
				float fractionalSplitDistances[CASCADE_COUNT-1];
				for(uint32 i = 0; i < CASCADE_COUNT-1; i++) {
					fractionalSplitDistances[i] = (i+1) / (float)CASCADE_COUNT;
				}
				SetFractionalSplitDistances(fractionalSplitDistances);
			}

			CascadeHelper(const float fractionalSplitDistances[CASCADE_COUNT-1])
			{
				SetFractionalSplitDistances(fractionalSplitDistances);
			}

			~CascadeHelper()
			{
			}

			inline void SetFractionalSplitDistances(const float fractionalSplitDistances[CASCADE_COUNT-1])
			{
				memcpy(this->fractionalSplitDistances, fractionalSplitDistances, (CASCADE_COUNT-1)*sizeof(float));
			}

			void CalculateCascades(const Matrix44 &cameraView, const Matrix44 &cameraProj, const Frustum &frustum)
			{
				Matrix44 viewProj = cameraProj * cameraView;
				Matrix44 invViewProj;
				Matrix44::Inverse(viewProj, invViewProj);

				Vector4 frustumCorners[8] = {
					Vector4(1.0f, 1.0f, 0.0f, 1.0f),		// Near, top right
					Vector4(-1.0f, 1.0f, 0.0f, 1.0f),		// Near, top left
					Vector4(-1.0f, -1.0f, 0.0f, 1.0f),		// Near, bottom left
					Vector4(1.0f, -1.0f, 0.0f, 1.0f),		// Near, bottom right

					Vector4(1.0f, 1.0f, 1.0f, 1.0f),		// Far, top right
					Vector4(-1.0f, 1.0f, 1.0f, 1.0f),		// Far, top left
					Vector4(-1.0f, -1.0f, 1.0f, 1.0f),		// Far, bottom left
					Vector4(1.0f, -1.0f, 1.0f, 1.0f)		// Far, bottom right
				};
				for(uint32 i = 0; i < 8; i++) {
					frustumCorners[i] = invViewProj * frustumCorners[i];
					frustumCorners[i] /= frustumCorners[i].w;
				}

				Vector4 nearToFar[4];
				for(uint32 i = 0; i < 4; i++) {
					nearToFar[i] = frustumCorners[4+i] - frustumCorners[i];
					nearToFar[i].w = 1.0f;
				}

				for(uint32 i = 0; i < CASCADE_COUNT-1; i++) {
					splitDistances[i] = fractionalSplitDistances[i] * (frustum.farPlane - frustum.nearPlane) + frustum.nearPlane;
				}

				// Calculate the 8 points describing each cascade world space
				for(uint32 i = 0; i < CASCADE_COUNT; i++) {
					float sliceNear, sliceFar;
					if(i == 0) {
						sliceNear = 0.0f;
						sliceFar = fractionalSplitDistances[0];
					} else if(i == CASCADE_COUNT-1) {
						sliceNear = fractionalSplitDistances[CASCADE_COUNT-2];
						sliceFar = 1.0f;
					} else {
						sliceNear = fractionalSplitDistances[i-1];
						sliceFar = fractionalSplitDistances[i];
					}

					Cascade &cascade = cascades[i];
					if(sliceNear <= 0.0f) {
						memcpy(cascade.verts[0].vals, frustumCorners[0].vals, sizeof(Vector4)*4);
					} else {
						for(uint32 j = 0; j < 4; j++) {
							cascade.verts[j] = frustumCorners[j] + nearToFar[j] * sliceNear;
						}
					}
					if(sliceFar >= 1.0f) {
						memcpy(cascade.verts[4].vals, frustumCorners[4].vals, sizeof(Vector4)*4);
					} else {
						for(uint32 j = 0; j < 4; j++) {
							cascade.verts[4+j] = frustumCorners[j] + nearToFar[j] * sliceFar;
						}
					}
				}
			}

			void CalculateLightFrustum(uint32 cascadeIndex, const Vector2 &shadowMapSize, const Matrix44 &lightView, Frustum &out)
			{
				// Transform the specified cascade into light view space
				assert(cascadeIndex < CASCADE_COUNT);
				Cascade &cascade = cascades[cascadeIndex];
				Cascade lightCascade;
				for(uint32 i = 0; i < 8; i++) {
					lightCascade.verts[i] = lightView * cascade.verts[i];
				}

				// Calculate the bounds of these points in light view space
				out.left = FLT_MAX;
				out.right = -FLT_MAX;
				out.bottom = FLT_MAX;
				out.top = -FLT_MAX;
				out.nearPlane = -FLT_MAX;
				out.farPlane = FLT_MAX;
				for(uint32 i = 0; i < 8; i++) {
					const Vector4 &v = lightCascade.verts[i];
					if(v.x < out.left)			{ out.left = v.x; }
					if(v.x > out.right)			{ out.right = v.x; }
					if(v.y < out.bottom)		{ out.bottom = v.y; }
					if(v.y > out.top)			{ out.top = v.y; }
					if(v.z > out.nearPlane)		{ out.nearPlane = v.z; }
					if(v.z < out.farPlane)		{ out.farPlane = v.z; }
				}

				out.nearPlane = 0.0f;
				out.farPlane = -out.farPlane;
				out.fov = 0.0f;

				// Adjust to pixel increments to avoid shadow edge jittering
				float cascadeMapWidth = shadowMapSize.x / CASCADE_COUNT;
				float widthUnitsPerTexel = (out.right - out.left) / cascadeMapWidth;
				float heightUnitsPerTexel = (out.top - out.bottom) / shadowMapSize.y;

				out.left /= widthUnitsPerTexel;
				out.left = (float)(int32)out.left;
				out.left *= widthUnitsPerTexel;

				out.right /= widthUnitsPerTexel;
				out.right = (float)(int32)out.right;
				out.right += 1.0f;
				out.right *= widthUnitsPerTexel;

				out.bottom /= heightUnitsPerTexel;
				out.bottom = (float)(int32)out.bottom;
				out.bottom *= heightUnitsPerTexel;

				out.top /= heightUnitsPerTexel;
				out.top = (float)(int32)out.top;
				out.top += 1.0f;
				out.top *= heightUnitsPerTexel;
			}
	
		public:
			float splitDistances[CASCADE_COUNT-1];

		private:
			float fractionalSplitDistances[CASCADE_COUNT-1];
			Cascade cascades[CASCADE_COUNT];
		};

	} // namespace Framework

} // namespace Maki