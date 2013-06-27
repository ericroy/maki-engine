#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiScene.h"

namespace Maki
{
	namespace Framework
	{

		class SceneLoader
		{
		public:
			SceneLoader();
			virtual ~SceneLoader();
			bool Load(Rid rid, Entity *parentNodeForScene);

		private:
			void Recurse(Document::Node *n, Entity *container);
		};

	} // namespace Framework

} // namespace Maki