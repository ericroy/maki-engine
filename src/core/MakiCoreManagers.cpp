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

namespace maki
{
	namespace core
	{

		core_managers_t::core_managers_t()
			: pseudo_singleton_t<core_managers_t>(),
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

		core_managers_t::core_managers_t(const config_t *config)
			: pseudo_singleton_t<core_managers_t>(),
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
			textureManager = new texture_manager_t(config->get_uint("engine.max_textures", texture_manager_t::default_size_));
			textureSetManager = new texture_set_manager_t(config->get_uint("engine.max_texture_sets", texture_manager_t::default_size_));
			materialManager = new material_manager_t(config->get_uint("engine.max_materials", texture_manager_t::default_size_));
			meshManager = new mesh_manager_t(config->get_uint("engine.max_meshes", texture_manager_t::default_size_));
			vertexFormatManager = new vertex_format_manager_t(config->get_uint("engine.max_vertex_formats", vertex_format_manager_t::default_size_));
			shaderProgramManager = new shader_program_manager_t(config->get_uint("engine.max_shader_programs", shader_program_manager_t::default_size_));
			fontManager = new font_manager_t(config->get_uint("engine.max_fonts", font_manager_t::default_size_));
			skeletonManager = new skeleton_manager_t(config->get_uint("engine.max_skeletons", skeleton_manager_t::default_size_));
			skeletonAnimationManager = new skeleton_animation_manager_t(config->get_uint("engine.max_skeleton_animations", skeleton_animation_manager_t::default_size_));

			// Check that the draw command bitfield (which is used for sorting draw commands) has enough bits allocated to each type
			// so that it can represent every possible handle value for that type.
			assert(meshManager->get_capacity() <= (1<<draw_command_t::bits_per_mesh_) && "assign more bits to mesh in draw command bitfield");
			assert(materialManager->get_capacity() <= (1<<draw_command_t::bits_per_material_) && "assign more bits to material in draw command bitfield");
			assert(textureSetManager->get_capacity() <= (1<<draw_command_t::bits_per_texture_set_) && "assign more bits to texture set in draw command bitfield");
			assert(vertexFormatManager->get_capacity() <= (1<<draw_command_t::bits_per_vertex_format_) && "assign more bits to vertex format in draw command bitfield");
			assert(shaderProgramManager->get_capacity() <= (1<<draw_command_t::bits_per_shader_program_) && "assign more bits to shader program in draw command bitfield");
		}

		core_managers_t::~core_managers_t()
		{
			MAKI_SAFE_DELETE(skeletonAnimationManager);
			MAKI_SAFE_DELETE(skeletonManager);
			MAKI_SAFE_DELETE(fontManager);
			MAKI_SAFE_DELETE(shaderProgramManager);
			MAKI_SAFE_DELETE(vertexFormatManager);
			MAKI_SAFE_DELETE(meshManager);
			MAKI_SAFE_DELETE(materialManager);
			MAKI_SAFE_DELETE(textureSetManager);
			MAKI_SAFE_DELETE(textureManager);
		}

		void core_managers_t::dump_manager_stats()
		{
			vertexFormatManager->dump_stats		("Vertex formats  ");
			shaderProgramManager->dump_stats		("shader_t programs ");
			textureSetManager->dump_stats		("texture_t sets    ");
			textureManager->dump_stats			("Textures        ");
			materialManager->dump_stats			("Materials       ");
			meshManager->dump_stats				("Meshes          ");
			fontManager->dump_stats				("Fonts           ");
			skeletonManager->dump_stats			("Skeletons       ");
			skeletonAnimationManager->dump_stats	("skeleton_t anims  ");
		}

		void core_managers_t::reload_assets()
		{
			engine_t *eng = engine_t::get();
			if(eng != nullptr) {
				eng->renderer_->sync_with_core();
			}

			console_t::info("Reloading all hot-swappable assets");
			meshManager->reload_assets();
			materialManager->reload_assets();
			textureManager->reload_assets();
			shaderProgramManager->reload_assets();
			skeletonManager->reload_assets();
			skeletonAnimationManager->reload_assets();
		
			dump_manager_stats();
		}

		bool core_managers_t::reload_asset(rid_t rid)
		{
			engine_t *eng = engine_t::get();
			if(eng != nullptr) {
				eng->renderer_->sync_with_core();
			}

			// Caution, relies on short-circuit evaluation
			if(	textureManager->reload_asset(rid) ||
				shaderProgramManager->reload_asset(rid) ||
				materialManager->reload_asset(rid) ||
				meshManager->reload_asset(rid) ||
				skeletonManager->reload_asset(rid) ||
				skeletonAnimationManager->reload_asset(rid)
			) {
				return true;
			}
			return false;
		}

	} // namespace core

} // namespace maki
