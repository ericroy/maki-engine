#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{

	class SceneNodeComponent : public Component
	{
	public:
		static const Type COMPONENT_TYPE = Type::Type_SceneNode;

	public:
		SceneNodeComponent();
		virtual ~SceneNodeComponent();
		virtual bool Init(Document::Node *node);

		inline void AddChild(Entity *e) { children.push_back(e); e->Get<SceneNodeComponent>()->parent = owner; }
		void RemoveChild(Entity *e);

		inline const Matrix44 &GetMatrix() const { return matrix; }
		inline const Matrix44 &GetWorldMatrix() const { return world; }
		inline const Quaternion &GetOrientation() const { return orientation; }
		inline const Vector4 &GetPosition() const { return position; }
		inline void CalculateViewMatrix(Matrix44 &out) const { Matrix44::AffineInverse(world, out); }

		inline void SetPosition(const Vector4 &pos) { SetMatrix(pos, orientation); }
		inline void SetPosition(float x, float y, float z) { SetMatrix(Vector4(x, y, z, 1.0f), orientation); }
		inline void SetOrientation(const Quaternion &orient) { SetMatrix(position, orient); }
		inline void SetMatrix(const Matrix44 &m);
		inline void SetWorldMatrix(const Matrix44 &w);
		inline void SetMatrix(const Vector4 &pos, const Quaternion &orient);
		inline void SetWorldMatrix(const Vector4 &pos, const Quaternion &orient);

	protected:
		inline void UpdateWorldMatrix();
		inline void UpdateMatrix();
		inline void UpdatePositionOrientation();

	public:
		Entity *parent;
		std::vector<Entity *> children;
		BoundingBox bounds;

	private:
		Matrix44 matrix;
		Matrix44 world;
		Vector4 position;
		Quaternion orientation;
	};



	inline void SceneNodeComponent::SetMatrix(const Matrix44 &m)
	{
		matrix = m;
		UpdatePositionOrientation();
		UpdateWorldMatrix();
	}

	inline void SceneNodeComponent::SetWorldMatrix(const Matrix44 &w)
	{
		world = w;
		if(parent != nullptr && !parent->world.IsIdentity()) {
			Matrix44 parentWorldInv;
			Matrix44::AffineInverse(parent->world, parentWorldInv);
			matrix = parentWorldInv * w;
		} else {
			matrix = world;
		}
		UpdatePositionOrientation();
	}

	inline void SceneNodeComponent::SetMatrix(const Vector4 &pos, const Quaternion &orient)
	{
		position = pos;
		orientation = orient;
		UpdateMatrix();
		UpdateWorldMatrix();
	}	

	inline void SceneNodeComponent::SetWorldMatrix(const Vector4 &pos, const Quaternion &orient)
	{
		Matrix44 m;
		orient.ToMatrix(m);
		Matrix44::Translation(pos, m);
		SetWorldMatrix(m);
	}

	inline void SceneNodeComponent::UpdateWorldMatrix()
	{
		if(parent != nullptr) {
			world = parent->world * matrix;
		} else {
			world = matrix;
		}
	}

	inline void SceneNodeComponent::UpdateMatrix()
	{
		orientation.ToMatrix(matrix);
		Matrix44::Translation(position, matrix);
	}

	inline void SceneNodeComponent::UpdatePositionOrientation()
	{
		position.x = matrix.cols[3][0];
		position.y = matrix.cols[3][1];
		position.z = matrix.cols[3][2];
		position.w = 1.0f;
		orientation.FromMatrix(matrix);
	}



}