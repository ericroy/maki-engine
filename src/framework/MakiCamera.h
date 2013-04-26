#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"

namespace Maki
{
	class Camera : public Entity
	{
	public:
		static std::function<Entity *()> CreateDebugWidget;

	public:
		Camera();
		virtual ~Camera();

	public:
		Frustum frustum;
	};

} // namespace Maki