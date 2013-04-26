#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	class BoundingSphere
	{
	public:
		BoundingSphere()
			: empty(true), pos(0.0f), radius(0.0f)
		{
		}
		BoundingSphere(const Vector4 &pos, float radius)
			: empty(false), pos(pos), radius(radius)
		{
		}
		BoundingSphere(const Vector3 &pos, float radius)
			: empty(false), pos(pos.x, pos.y, pos.z, 1.0f), radius(radius)
		{
		}

		inline void Merge(const BoundingSphere &other)
		{
			if(other.empty) {
				return;
			}
			if(empty) {
				pos = other.pos;
				radius = other.radius;
				empty = false;
				return;
			}

			Vector4 dir = other.pos - pos;
			float dist = dir.Length();
			if(dist < 0.000001f) {
				if(other.radius > radius) {
					radius = other.radius;
				}
			} else {
				dir /= dist;
				Vector4 minPoint = dir * -radius;
				Vector4 maxPoint = dir * (dist + other.radius);
				pos = (minPoint + maxPoint) / 2.0f;
				radius = (radius + dist + other.radius) / 2.0f;
			}
		}

		inline void Merge(const Vector4 &p)
		{
			if(empty) {
				pos = p;
				empty = false;
				return;
			}
			
			Vector4 dir = p - pos;
			float dist = dir.Length();
			if(dist < 0.000001f) {
				if(dist > radius) {
					radius = dist;
				}
			} else {
				dir /= dist;
				Vector4 minPoint = dir * -radius;
				Vector4 maxPoint = dir * dist;
				pos = (minPoint + maxPoint) / 2.0f;
				radius = (radius + dist) / 2.0f;
			}
		}

		inline void Reset()
		{
			pos = Vector4(0.0f);
			radius = 0.0f;
			empty = true;
		}

	public:
		Vector4 pos;
		float radius;
		bool empty;
	};

} // namespace Maki