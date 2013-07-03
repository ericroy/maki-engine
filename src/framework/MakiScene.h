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

			inline void Update()
			{
				UpdateRecursive(root, Matrix44::Identity);
			}

		private:
			void UpdateRecursive(Entity *e, const Matrix44 &current);

		public:
			Entity *root;

		private:
			Entity *drawListHead;
		};

	} // namespace Framework

} // namespace Maki