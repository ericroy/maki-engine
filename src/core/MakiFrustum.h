#pragma once
#include "core/core_stdafx.h"
#include "core/MakiVector2.h"

namespace Maki
{
	namespace Core
	{
	
		class Frustum
		{
		public:
			Frustum() {}
		
			Frustum(float left, float right, float bottom, float top, float nearPlane, float farPlane, float fov = 0.0f)
				: left(left), right(right), bottom(bottom), top(top), nearPlane(nearPlane), farPlane(farPlane), fov(fov)
			{
			}

			Frustum(float width, float height, float nearPlane, float farPlane, float fov = 0.0f)
				: left(-width/2), right(width/2), bottom(-height/2), top(height/2), nearPlane(nearPlane), farPlane(farPlane), fov(fov)
			{
			}

			Frustum(const Vector2 &widthHeight, float nearPlane, float farPlane, float fov = 0.0f)
				: left(-widthHeight.x/2), right(widthHeight.x/2), bottom(-widthHeight.y/2), top(widthHeight.y/2), nearPlane(nearPlane), farPlane(farPlane), fov(fov)
			{
			}

			inline void Set(float width, float height, float nearPlane, float farPlane, float fov = 0.0f)
			{
				float wo2 = width / 2.0f;
				float ho2 = height / 2.0f;
				left = -wo2;
				right = wo2;
				bottom = -ho2;
				top = ho2;
				this->nearPlane = nearPlane;
				this->farPlane = farPlane;
				this->fov = fov;
			}
			inline void Set(const Vector2 &widthHeight, float nearPlane, float farPlane, float fov = 0.0f) { Set(widthHeight.x, widthHeight.y, nearPlane, farPlane, fov); }
			inline void Set(float width, float height) { Set(width, height, this->nearPlane, this->farPlane, this->fov); }		
			inline void Set(const Vector2 &widthHeight) { Set(widthHeight.x, widthHeight.y, this->nearPlane, this->farPlane, this->fov); }

			inline void Set(float left, float right, float bottom, float top, float nearPlane, float farPlane, float fov = 0.0f)
			{
				this->left = left;
				this->right = right;
				this->bottom = bottom;
				this->top = top;
				this->nearPlane = nearPlane;
				this->farPlane = farPlane;
				this->fov = fov;
			}
			inline void Set(float left, float right, float bottom, float top) { Set(left, right, bottom, top, this->nearPlane, this->farPlane, this->fov); }


			inline float GetWidth() const { return right - left; }
			inline float GetHeight() const { return top - bottom; }
			inline float GetAspect() const
			{
				float h = top - bottom;
				return h != 0.0f ? (right - left) / h : 1.0f;
			}

			void ExtractPlanes(Vector4 planes[6]) const
			{
				// Near, Far
				planes[0] = Vector4(0.0f, 0.0f, 1.0f, -nearPlane); 
				planes[1] = Vector4(0.0f, 0.0f, -1.0f, farPlane);

				if(fov == 0.0f) {
					// Left, right
					planes[2] = Vector4(-1.0f, 0.0f, 0.0f, -left); 
					planes[3] = Vector4(1.0f, 0.0f, 0.0f, right);

					// Top, bottom
					planes[4] = Vector4(0.0f, 1.0f, 0.0f, top); 
					planes[5] = Vector4(0.0f, -1.0f, 0.0f, -bottom);
				} else {
					float hh = farPlane * std::tan(fov * MAKI_DEG_TO_RAD / 2.0f);
					float hw = hh * GetAspect();

					Vector4 farTopRight(hw, hh, farPlane, 1.0f);
					Vector4 farTopLeft(-hw, hh, farPlane, 1.0f);
					Vector4 farBottomLeft(-hw, -hh, farPlane, 1.0f);
					Vector4 farBottomRight(hw, -hh, farPlane, 1.0f);

					// Left, right
					planes[2] = farTopLeft.Cross(farBottomLeft);
					planes[2].Normalize();
					planes[2].w = 0.0f;
					planes[3] = farBottomRight.Cross(farTopRight);
					planes[3].Normalize();
					planes[3].w = 0.0f;

					// Top, bottom
					planes[4] = farTopRight.Cross(farTopLeft);
					planes[4].Normalize();
					planes[4].w = 0.0f;
					planes[5] = farBottomLeft.Cross(farBottomRight);
					planes[5].Normalize();
					planes[5].w = 0.0f;
				}
			}

		public:
			float left;
			float right;
			float bottom;
			float top;
			float nearPlane;
			float farPlane;
			// Zero implies orthographic
			float fov;
		};

	} // namespace Core

} // namespace Maki