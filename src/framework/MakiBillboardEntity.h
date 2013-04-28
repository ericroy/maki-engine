#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiMeshEntity.h"

namespace Maki
{
	class BillboardEntity : public MeshEntity
	{
	public:
		enum BillboardMode
		{
			BillboardMode_None = 0,
			BillboardMode_Face,
			BillboardMode_Pivot,
		};

	public:
		BillboardEntity(HandleOrRid mesh, HandleOrRid material)
			: MeshEntity(mesh, material), mode(BillboardMode_None), facingAxis(Vector4::UnitX), pivotAxis(Vector4::UnitZ)
		{
		}

		BillboardEntity(HandleOrRid mesh, HandleOrRid material, BillboardMode mode, const Vector4 &facingAxis = Vector4::UnitX, const Vector4 &pivotAxis = Vector4::UnitZ)
			: MeshEntity(mesh, material), mode(mode), facingAxis(facingAxis), pivotAxis(pivotAxis)
		{
		}

		virtual ~BillboardEntity()
		{
		}

		inline void SetMode(BillboardMode mode, const Vector4 &facingAxis = Vector4::UnitX, const Vector4 &pivotAxis = Vector4::UnitZ)
		{
			this->mode = mode;
			switch(mode) {
			case BillboardMode_Face:
				this->facingAxis = facingAxis;
				break;
			case BillboardMode_Pivot:
				this->facingAxis = facingAxis;
				this->pivotAxis = pivotAxis;
				break;
			default:
				break;
			}
		}

		void Draw(Renderer *renderer) {
			if(mode == BillboardMode_None) {
				MeshEntity::Draw(renderer);
				return;
			}
			
			Vector4 basisWorldPos(world.cols[3]);
			Vector4 toCamera = -Vector4(renderer->GetView().cols[3]) - basisWorldPos;
			toCamera.Normalize();

			if(mode == BillboardMode_Face) {
				orientation = Quaternion::Billboard(toCamera, facingAxis);
			} else {
				orientation = Quaternion::Billboard(toCamera, facingAxis, pivotAxis);
			}
			UpdateMatrix();
			UpdateWorldMatrix();

			MeshEntity::Draw(renderer);
		}

	private:
		BillboardMode mode;
		Vector4 facingAxis;
		Vector4 pivotAxis;
	};

} // namespace Maki