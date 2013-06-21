#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{

	class CharacterComponent : public Component
	{
	public:
		static const Type COMPONENT_TYPE = Type::Type_Character;

	public:
		CharacterComponent()
			: Component(COMPONENT_TYPE)
		{
		}
		virtual ~CharacterComponent()
		{
		}
		virtual bool Init(Document::Node *node)
		{
			return true;
		}
	};

}