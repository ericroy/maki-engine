#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{

	class CameraComponent : public Component
	{
	public:
		CameraComponent()
			: Component(Type_Camera)
		{
		}
		virtual ~CameraComponent()
		{
		}
		virtual bool Init(Document::Node *node)
		{
			return true;
		}
	};

}