#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"
#include "framework/MakiAnimationBlender.h"

namespace Maki
{
	class CharacterEntity : public Entity
	{
	private:
		static const float TURN_SPEED;
		static const float CAM_TURN_SPEED;
		static const float MOVE_SPEED;

	public:
		CharacterEntity();
		virtual ~CharacterEntity();
		virtual void Update(float dt);

	private:
		AnimationBlender blender;
		btRigidBody *body;
	};

} // namespace Maki