#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiScene.h"

namespace Maki
{
	class MeshEntity;
	class SkinnedMeshEntity;
	class CharacterEntity;
	class BillboardEntity;
	class Light;
	class Camera;

	class SceneLoader
	{
	public:
		SceneLoader();
		virtual ~SceneLoader();
		bool Load(Rid rid, Entity *parentNodeForScene, std::vector<Camera *> *cameras = nullptr, std::vector<Light *> *lights = nullptr, std::vector<Entity *> *physicsHulls = nullptr);

	private:
		void Recurse(Document::Node *n, Entity *container, std::vector<Camera *> *cameras, std::vector<Light *> *lights, std::vector<Entity *> *physicsHulls);
	};

} // namespace Maki