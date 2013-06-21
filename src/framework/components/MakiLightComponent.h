#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{

	class LightComponent : public Component
	{
	public:
		LightComponent()
			: Component(Type_Light)
		{
		}
		virtual ~LightComponent()
		{
		}
		virtual bool Init(Document::Node *node)
		{
			return true;
		}
	};

}