#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			class Transform : public Component
			{
			public:
				static const Type TYPE = Type_Transform;
				static const TypeFlag DEPENDENCIES = 0;

			public:
				Transform();
				virtual ~Transform();
				bool Init(Document::Node *node);
				bool Init(const Vector4 &pos, const Vector4 &eulerAngles = Vector4(0.0f));
				Transform *Clone(bool prototype);

				inline const Matrix44 &GetMatrix() const { return matrix; }
				inline const Quaternion &GetOrientation() const { return orientation; }
				inline const Vector4 &GetPosition() const { return position; }

				inline void SetPosition(const Vector4 &pos) { SetMatrix(pos, orientation); }
				inline void SetPosition(float x, float y, float z) { SetMatrix(Vector4(x, y, z, 1.0f), orientation); }
				inline void SetOrientation(const Quaternion &orient) { SetMatrix(position, orient); }
				inline void SetMatrix(const Matrix44 &m);
				inline void SetMatrix(const Vector4 &pos, const Quaternion &orient);
				inline void LookAt(const Vector4 &target);

				inline void SetTransformRelative(bool relative) { relativeMode = relative; }
				inline const Matrix44 &GetWorldMatrix() const { return relativeMode ? world : matrix; }
				inline void SetWorldMatrix(const Matrix44 &m);
				
			private:
				inline void UpdateMatrix();
				inline void UpdatePositionOrientation();
				
			private:
				Matrix44 world;
				Matrix44 matrix;
				Vector4 position;
				Quaternion orientation;

				// When <relativeMode> is false (the default), the <matrix> member is a world matrix.  When <relativeMode>
				// is true, the <matrix> member is considered to be relative to a parent coordinate system and the <world> member will
				// be treated as the world matrix.
				// 
				// To get the world matrix that you need to draw or reason in world space, simply call GetWorldMatrix().
				// <relativeMode> is set to true if this entity is added to a scene graph.
				bool relativeMode;
			};



			inline void Transform::SetMatrix(const Matrix44 &m)
			{
				matrix = m;
				UpdatePositionOrientation();
			}

			inline void Transform::SetMatrix(const Vector4 &pos, const Quaternion &orient)
			{
				position = pos;
				orientation = orient;
				UpdateMatrix();
			}

			inline void Transform::LookAt(const Vector4 &target)
			{
				Matrix44 lookAt;
				Matrix44::LookAt(position, target, Vector4::UnitZ, lookAt);
				Matrix44::Inverse(lookAt, lookAt);
				SetMatrix(lookAt);
			}

			inline void Transform::SetWorldMatrix(const Matrix44 &m)
			{
				if(relativeMode) {
					world = m;
				} else {
					matrix = m;
				}
			}

			inline void Transform::UpdateMatrix()
			{
				orientation.ToMatrix(matrix);
				Matrix44::Translation(position, matrix);
			}

			inline void Transform::UpdatePositionOrientation()
			{
				position.x = matrix.cols[3][0];
				position.y = matrix.cols[3][1];
				position.z = matrix.cols[3][2];
				position.w = 1.0f;
				orientation.FromMatrix(matrix);
			}

		} // namespace Components

	} // namespace Framework

} // namespace Maki