#pragma once
#include "core/core_stdafx.h"
#include "core/MakiPseudoSingleton.h"

namespace Maki
{
	namespace Core
	{
		class Config;
		class DrawCommandManager;
		class TextureManager;
		class TextureSetManager;
		class MaterialManager;
		class MeshManager;
		class VertexFormatManager;
		class ShaderProgramManager;
		class FontManager;
		class SkeletonManager;
		class SkeletonAnimationManager;

		class MAKI_CORE_API CoreManagers : public PseudoSingleton<CoreManagers>
		{
		public:
			CoreManagers();
			CoreManagers(const Config *config);
			virtual ~CoreManagers();
			void DumpManagerStats();
			virtual void ReloadAssets();
			virtual void ReloadAsset(Rid rid);

		public:
			TextureManager *textureManager;
			TextureSetManager *textureSetManager;
			MaterialManager *materialManager;
			MeshManager *meshManager;
			VertexFormatManager *vertexFormatManager;
			ShaderProgramManager *shaderProgramManager;
			FontManager *fontManager;
			SkeletonManager *skeletonManager;
			SkeletonAnimationManager *skeletonAnimationManager;
		};

	} // namespace Core

} // namespace Maki