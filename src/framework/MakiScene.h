#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"

namespace Maki
{
	class Camera;
	class Light;

	class Scene
	{
	public:
		Scene();
		virtual ~Scene();
		void Clear();
		bool Load(Rid rid);
		void Update(float dt);
		void Draw(Renderer *renderer, uint32 requiredFlags = 0, uint32 disallowedFlags = 0, const Frustum *cullingFrustum = nullptr, const Matrix44 *view = nullptr);

	public:
		Entity *root;
		std::vector<Camera *> cameras;
		std::vector<Light *> lights;
		std::vector<Entity *> physicsHulls;

	private:
		Entity *drawListHead;
	};

} // namespace Maki