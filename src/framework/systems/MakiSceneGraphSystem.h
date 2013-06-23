#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	class SceneNodeComponent;

	class SceneGraphSystem : public System
	{
	public:
		SceneGraphSystem();
		virtual ~SceneGraphSystem();
		
		void Update();
		void ProcessMessages();

	private:
		void UpdateBoundsRecurse(Entity *e);

	protected:
		void Add(Entity *e);
		virtual void Remove(Entity *e);

	private:
		std::vector<SceneNodeComponent *> items;
	};


} // namespace Maki