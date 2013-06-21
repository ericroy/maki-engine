#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{

	class CharacterComponent : public Component
	{
	public:
		CharacterComponent()
			: Component(Type_Character)
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