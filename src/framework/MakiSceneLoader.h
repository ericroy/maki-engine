#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiScene.h"

namespace Maki
{
	class MeshEntity;
	class SkinnedMeshEntity;
	class CharacterEntity;
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
		Entity *CreateEntity(Document::Node *n);
		MeshEntity *CreateMeshEntity(Document::Node *n);
		SkinnedMeshEntity *CreateSkinnedMeshEntity(Document::Node *n);
		CharacterEntity *CreateCharacterEntity(Document::Node *n);
		Light *CreateLight(Document::Node *n);
		Camera *CreateCamera(Document::Node *n);
		bool ReadFloats(Document::Node *n, float *out, uint32 count);
		bool ReadInts(Document::Node *n, int32 *out, uint32 count);
		void UpdateFlags(Document::Node *flagsNode, uint32 &flags);
	};

} // namespace Maki