#include "core/core_stdafx.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"
#include "core/MakiConfig.h"
#include "core/MakiDrawCommand.h"
#include "core/MakiTextureManager.h"
#include "core/MakiTextureSetManager.h"
#include "core/MakiMaterialManager.h"
#include "core/MakiMeshManager.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiShaderProgramManager.h"
#include "core/MakiFontManager.h"
#include "core/MakiSkeletonManager.h"
#include "core/MakiSkeletonAnimationManager.h"

namespace Maki
{
	namespace Core
	{

		CoreManagers::CoreManagers()
			: PseudoSingleton<CoreManagers>(),
			textureManager(nullptr),
			textureSetManager(nullptr),
			materialManager(nullptr),
			meshManager(nullptr),
			vertexFormatManager(nullptr),
			shaderProgramManager(nullptr),
			fontManager(nullptr),
			skeletonManager(nullptr),
			skeletonAnimationManager(nullptr)
		{
		}

		CoreManagers::CoreManagers(const Config *config)
			: PseudoSingleton<CoreManagers>(),
			textureManager(nullptr),
			textureSetManager(nullptr),
			materialManager(nullptr),
			meshManager(nullptr),
			vertexFormatManager(nullptr),
			shaderProgramManager(nullptr),
			fontManager(nullptr),
			skeletonManager(nullptr),
			skeletonAnimationManager(nullptr)
		{
			textureManager = new TextureManager(config->GetUint("engine.max_textures", TextureManager::DEFAULT_SIZE));
			textureSetManager = new TextureSetManager(config->GetUint("engine.max_texture_sets", TextureManager::DEFAULT_SIZE));
			materialManager = new MaterialManager(config->GetUint("engine.max_materials", TextureManager::DEFAULT_SIZE));
			meshManager = new MeshManager(config->GetUint("engine.max_meshes", TextureManager::DEFAULT_SIZE));
			vertexFormatManager = new VertexFormatManager(config->GetUint("engine.max_vertex_formats", VertexFormatManager::DEFAULT_SIZE));
			shaderProgramManager = new ShaderProgramManager(config->GetUint("engine.max_shader_programs", ShaderProgramManager::DEFAULT_SIZE));
			fontManager = new FontManager(config->GetUint("engine.max_fonts", FontManager::DEFAULT_SIZE));
			skeletonManager = new SkeletonManager(config->GetUint("engine.max_skeletons", SkeletonManager::DEFAULT_SIZE));
			skeletonAnimationManager = new SkeletonAnimationManager(config->GetUint("engine.max_skeleton_animations", SkeletonAnimationManager::DEFAULT_SIZE));

			// Check that the draw command bitfield (which is used for sorting draw commands) has enough bits allocated to each type
			// so that it can represent every possible handle value for that type.
			assert(meshManager->GetCapacity() <= (1<<DrawCommand::BITS_PER_MESH) && "assign more bits to mesh in draw command bitfield");
			assert(materialManager->GetCapacity() <= (1<<DrawCommand::BITS_PER_MATERIAL) && "assign more bits to material in draw command bitfield");
			assert(textureSetManager->GetCapacity() <= (1<<DrawCommand::BITS_PER_TEXTURE_SET) && "assign more bits to texture set in draw command bitfield");
			assert(vertexFormatManager->GetCapacity() <= (1<<DrawCommand::BITS_PER_VERTEX_FORMAT) && "assign more bits to vertex format in draw command bitfield");
			assert(shaderProgramManager->GetCapacity() <= (1<<DrawCommand::BITS_PER_SHADER_PROGRAM) && "assign more bits to shader program in draw command bitfield");
		}

		CoreManagers::~CoreManagers()
		{
			SAFE_DELETE(skeletonAnimationManager);
			SAFE_DELETE(skeletonManager);
			SAFE_DELETE(fontManager);
			SAFE_DELETE(shaderProgramManager);
			SAFE_DELETE(vertexFormatManager);
			SAFE_DELETE(meshManager);
			SAFE_DELETE(materialManager);
			SAFE_DELETE(textureSetManager);
			SAFE_DELETE(textureManager);
		}

		void CoreManagers::DumpManagerStats()
		{
			vertexFormatManager->DumpStats		("Vertex formats  ");
			shaderProgramManager->DumpStats		("Shader programs ");
			textureSetManager->DumpStats		("Texture sets    ");
			textureManager->DumpStats			("Textures        ");
			materialManager->DumpStats			("Materials       ");
			meshManager->DumpStats				("Meshes          ");
			fontManager->DumpStats				("Fonts           ");
			skeletonManager->DumpStats			("Skeletons       ");
			skeletonAnimationManager->DumpStats	("Skeleton anims  ");
		}

		void CoreManagers::ReloadAssets()
		{
			Engine *eng = Engine::Get();
			if(eng != nullptr) {
				eng->renderer->SyncWithCore();
			}

			Console::Info("Reloading all hot-swappable assets");
			meshManager->ReloadAssets();
			materialManager->ReloadAssets();
			textureManager->ReloadAssets();
			shaderProgramManager->ReloadAssets();
			skeletonManager->ReloadAssets();
			skeletonAnimationManager->ReloadAssets();
		
			DumpManagerStats();
		}

		bool CoreManagers::ReloadAsset(Rid rid)
		{
			Engine *eng = Engine::Get();
			if(eng != nullptr) {
				eng->renderer->SyncWithCore();
			}

			// Caution, relies on short-circuit evaluation
			if(	textureManager->ReloadAsset(rid) ||
				shaderProgramManager->ReloadAsset(rid) ||
				materialManager->ReloadAsset(rid) ||
				meshManager->ReloadAsset(rid) ||
				skeletonManager->ReloadAsset(rid) ||
				skeletonAnimationManager->ReloadAsset(rid)
			) {
				return true;
			}
			return false;
		}

	} // namespace Core

} // namespace Maki