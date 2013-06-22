#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"
#include "framework/MakiAnimationBlender.h"

namespace Maki
{

	class CharacterComponent : public Component
	{
	public:
		static const Type COMPONENT_TYPE = Type::Type_Character;

	private:
		static const float TURN_SPEED;
		static const float CAM_TURN_SPEED;
		static const float MOVE_SPEED;

	public:
		CharacterComponent();		
		virtual ~CharacterComponent();

		virtual void Attach(Entity *owner);
		virtual bool Init(Document::Node *node);

	private:
		AnimationBlender blender;
		btRigidBody *body;
	};

}