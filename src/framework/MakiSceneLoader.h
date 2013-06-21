#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiScene.h"

namespace Maki
{

	class SceneLoader
	{
	public:
		SceneLoader();
		virtual ~SceneLoader();
		bool Load(Rid rid, Entity *parentNodeForScene, std::vector<Entity *> *cameras = nullptr, std::vector<Entity *> *lights = nullptr, std::vector<Entity *> *physicsHulls = nullptr);

	private:
		void Recurse(Document::Node *n, Entity *container, std::vector<Entity *> *cameras, std::vector<Entity *> *lights, std::vector<Entity *> *physicsHulls);
	};

} // namespace Maki