#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{

	class BillboardComponent : public Component
	{
	public:
		static const Component::Type COMPONENT_TYPE = Type::Type_Billboard;

	public:
		BillboardComponent()
			: Component(COMPONENT_TYPE)
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