#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"

namespace Maki
{
	namespace Framework
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
		
			void Draw(Renderer *renderer, uint32 requiredFlags = 0, uint32 disallowedFlags = 0, const Frustum *cullingFrustum = nullptr, const Matrix44 *view = nullptr);
		
			inline void Update(float dt)
			{
				UpdateRecursive(root, Matrix44::Identity, dt);
			}

		private:
			void UpdateRecursive(Entity *e, const Matrix44 &current, float dt);

		public:
			Entity *root;

		private:
			Entity *drawListHead;
		};

	} // namespace Framework

} // namespace Maki