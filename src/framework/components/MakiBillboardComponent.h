#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{

	class BillboardComponent : public Component
	{
	public:
		BillboardComponent()
			: Component(Type_Billboard)
		{
		}
		virtual ~BillboardComponent()
		{
		}
		virtual bool Init(Document::Node *node)
		{
			return true;
		}
	};

}