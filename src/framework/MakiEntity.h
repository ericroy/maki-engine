#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiPhysicsLink.h"

namespace Maki
{
	class Renderer;

	class Entity : public Aligned<SIMD_ALIGN>
	{
	public:
		enum Flag
		{
			Flag_None = 0,
			Flag_Draw = 1<<0,
			Flag_Update = 1<<1,
			Flag_Physics = 1<<2,
			Flag_ProcessChildren = 1<<3,
			Flag_CastShadow = 1<<4,
		};

	public:
		static const uint32 DEFAULT_FLAGS = Flag_Draw|Flag_Update|Flag_ProcessChildren|Flag_CastShadow;
		
	public:
		Entity();
		Entity(uint32 flags);
		virtual ~Entity();

		virtual void Update(float dt) {}
		virtual void Draw(Renderer *renderer) {}

		void RecursivelyUpdate(Entity **drawListHead, const Matrix44 &current, float dt);
		inline void AddChild(Entity *e) { children.push_back(e); e->parent = this; }
		void RemoveChild(Entity *e);
		
		inline uint32 GetFlags() const { return flags; }
		inline void SetFlags(uint32 f) { flags = f; }
		inline bool GetFlag(Flag f) const { return (f & flags) != 0; }
		inline void SetFlag(Flag f, bool on = true) { if(on) { flags |= f; } else { flags &= ~f; } }

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
		std::function<void(float)> updateFunc;
		BoundingBox bounds;
		PhysicsLink physicsLink;
		Entity *drawListNext;

	protected:
		uint32 flags;
		Matrix44 matrix;
		Matrix44 world;
		Vector4 position;
		Quaternion orientation;
	};





	inline void Entity::SetMatrix(const Matrix44 &m)
	{
		matrix = m;
		UpdatePositionOrientation();
		UpdateWorldMatrix();
	}

	inline void Entity::SetWorldMatrix(const Matrix44 &w)
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

	inline void Entity::SetMatrix(const Vector4 &pos, const Quaternion &orient)
	{
		position = pos;
		orientation = orient;
		UpdateMatrix();
		UpdateWorldMatrix();
	}	

	inline void Entity::SetWorldMatrix(const Vector4 &pos, const Quaternion &orient)
	{
		Matrix44 m;
		orient.ToMatrix(m);
		Matrix44::Translation(pos, m);
		SetWorldMatrix(m);
	}

	inline void Entity::UpdateWorldMatrix()
	{
		if(parent != nullptr) {
			world = parent->world * matrix;
		} else {
			world = matrix;
		}
	}

	inline void Entity::UpdateMatrix()
	{
		orientation.ToMatrix(matrix);
		Matrix44::Translation(position, matrix);
	}

	inline void Entity::UpdatePositionOrientation()
	{
		position.x = matrix.cols[3][0];
		position.y = matrix.cols[3][1];
		position.z = matrix.cols[3][2];
		position.w = 1.0f;
		orientation.FromMatrix(matrix);
	}

} // namespace Maki