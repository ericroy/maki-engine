#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	class BoundingBox
	{
	private:
		static const Vector4 cornerCoeffs[8];

	public:
		BoundingBox()
			: empty(true), pos(0.0f), radii(0.0f)
		{
		}
		BoundingBox(const Vector4 &pos, const Vector3 &radii)
			: empty(false), pos(pos), radii(radii.x, radii.y, radii.z, 1.0f)
		{
		}
		BoundingBox(const Vector4 &pos, const Vector4 &radii)
			: empty(false), pos(pos), radii(radii)
		{
		}

		inline void Merge(const BoundingBox &other)
		{
			if(empty) {
				pos = other.pos;
				radii = other.radii;
				empty = other.empty;
				return;
			}

			Vector4 maxCorner(
				std::max<float>(pos.x+radii.x, other.pos.x+other.radii.x),
				std::max<float>(pos.y+radii.y, other.pos.y+other.radii.y),
				std::max<float>(pos.z+radii.z, other.pos.z+other.radii.z),
				1.0f);
			Vector4 minCorner(
				std::min<float>(pos.x-radii.x, other.pos.x-other.radii.x),
				std::min<float>(pos.y-radii.y, other.pos.y-other.radii.y),
				std::min<float>(pos.z-radii.z, other.pos.z-other.radii.z),
				1.0f);
			pos = (maxCorner + minCorner) / 2.0f;
			radii = maxCorner - pos;
		}

		inline void Merge(float x, float y, float z)
		{
			if(empty) {
				pos = Vector4(x, y, z, 1.0f);
				empty = false;
				return;
			}

			Vector4 maxCorner(
				std::max<float>(pos.x+radii.x, x),
				std::max<float>(pos.y+radii.y, y),
				std::max<float>(pos.z+radii.z, z),
				1.0f);
			Vector4 minCorner(
				std::min<float>(pos.x-radii.x, x),
				std::min<float>(pos.y-radii.y, y),
				std::min<float>(pos.z-radii.z, z),
				1.0f);
			pos = (maxCorner + minCorner) / 2.0f;
			radii = maxCorner - pos;
		}
		inline void Merge(const Vector4 &point) { Merge(point.x, point.y, point.z); }
		inline void Merge(const Vector3 &point) { Merge(point.x, point.y, point.z); }
		
		inline void Merge(const Vector4 &point, float radius)
		{
			Merge(point.x+radius, point.y+radius, point.z+radius);
			Merge(point.x-radius, point.y-radius, point.z-radius);
		}

		inline void Reset()
		{
			empty = true;
			pos = Vector4(0.0f);
			radii = Vector4(0.0f);
		}

		inline Vector4 GetMaxCorner() const { return pos + radii; }
		inline Vector4 GetMinCorner() const { return pos - radii; }
		inline Vector4 GetCorner(uint32 i) const
		{
			assert(i < 8);
			return pos + cornerCoeffs[i] * radii;
		}

		inline float GetRadius() const { return radii.Length(); }
		inline float GetRadiusSq() const { return radii.LengthSquared(); }

	public:
		Vector4 pos;
		Vector4 radii;
		bool empty;
	};


	inline BoundingBox operator*(float scale, const BoundingBox &bounds)
	{
		return BoundingBox(bounds.pos*scale, bounds.radii*scale);
	}


} // namespace Maki