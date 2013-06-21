#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{

	class SkinnedMeshComponent : public Component
	{
	public:
		SkinnedMeshComponent()
			: Component(Type_SkinnedMesh)
		{
		}
		virtual ~SkinnedMeshComponent()
		{
		}
		virtual bool Init(Document::Node *node)
		{
			return true;
		}
	};

}