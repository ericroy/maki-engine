#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	class Rect
	{
	public:
		Rect() {}
		
		Rect(float left, float right, float top, float bottom)
			: left(left), right(right), top(top), bottom(bottom)
		{
		}

		Rect(float width, float height, bool centered = false)
		{
			Set(width, height, centered);
		}

		Rect(const Vector2 &widthHeight, bool centered = false)
		{
			Set(widthHeight, centered);
		}

		inline void Set(float left, float right, float top, float bottom)
		{
			left = left;
			right = right;
			top = top;
			bottom = bottom;
		}

		inline void Set(float width, float height, bool centered = false)
		{
			if(centered) {
				float wo2 = width / 2;
				float ho2 = height / 2;
				left = -wo2;
				right = wo2;
				top = -ho2;
				bottom = ho2;
			} else {
				left = 0.0f;
				top = 0.0f;
				right = width;
				bottom = height;
			}
		}

		inline void Set(const Vector2 &widthHeight, bool centered = false)
		{
			Set(widthHeight.x, widthHeight.y, centered);
		}

		inline float GetWidth() const { return right - left; }
		inline float GetHeight() const { return bottom - top; }

	public:
		float left, right, top, bottom;
	};
} // namespace Maki