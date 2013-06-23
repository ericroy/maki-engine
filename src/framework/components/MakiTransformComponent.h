#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{

	class TransformComponent : public Component
	{
		friend class SceneGraphSystem;

	public:
		static const Component::Type COMPONENT_TYPE = Type_Transform;

	public:
		TransformComponent();
		virtual ~TransformComponent();
		virtual bool Init(Document::Node *node);

		inline const Matrix44 &GetMatrix() const { return matrix; }
		inline const Quaternion &GetOrientation() const { return orientation; }
		inline const Vector4 &GetPosition() const { return position; }

		inline void SetPosition(const Vector4 &pos) { SetMatrix(pos, orientation); }
		inline void SetPosition(float x, float y, float z) { SetMatrix(Vector4(x, y, z, 1.0f), orientation); }
		inline void SetOrientation(const Quaternion &orient) { SetMatrix(position, orient); }
		inline void SetMatrix(const Matrix44 &m);
		inline void SetMatrix(const Vector4 &pos, const Quaternion &orient);

	private:
		inline void UpdateMatrix();
		inline void UpdatePositionOrientation();

	public:
		bool useWorldMatrix;
		Matrix44 world;

	private:
		Matrix44 matrix;
		Vector4 position;
		Quaternion orientation;
	};



	inline void TransformComponent::SetMatrix(const Matrix44 &m)
	{
		matrix = m;
		UpdatePositionOrientation();
	}

	inline void TransformComponent::SetMatrix(const Vector4 &pos, const Quaternion &orient)
	{
		position = pos;
		orientation = orient;
		UpdateMatrix();
	}	

	inline void TransformComponent::UpdateMatrix()
	{
		orientation.ToMatrix(matrix);
		Matrix44::Translation(position, matrix);
	}

	inline void TransformComponent::UpdatePositionOrientation()
	{
		position.x = matrix.cols[3][0];
		position.y = matrix.cols[3][1];
		position.z = matrix.cols[3][2];
		position.w = 1.0f;
		orientation.FromMatrix(matrix);
	}



}