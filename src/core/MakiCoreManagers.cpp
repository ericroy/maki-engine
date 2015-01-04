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
			texture_manager_(nullptr),
			texture_set_manager_(nullptr),
			material_manager_(nullptr),
			mesh_manager_(nullptr),
			vertex_format_manager_(nullptr),
			shader_program_manager_(nullptr),
			font_manager_(nullptr),
			skeleton_manager_(nullptr),
			skeleton_animation_manager_(nullptr)
		{
		}

		core_managers_t::core_managers_t(const config_t *config)
			: pseudo_singleton_t<core_managers_t>(),
			texture_manager_(nullptr),
			texture_set_manager_(nullptr),
			material_manager_(nullptr),
			mesh_manager_(nullptr),
			vertex_format_manager_(nullptr),
			shader_program_manager_(nullptr),
			font_manager_(nullptr),
			skeleton_manager_(nullptr),
			skeleton_animation_manager_(nullptr)
		{
			texture_manager_ = new texture_manager_t(config->get_uint("engine.max_textures", texture_manager_t::default_size_));
			texture_set_manager_ = new texture_set_manager_t(config->get_uint("engine.max_texture_sets", texture_manager_t::default_size_));
			material_manager_ = new material_manager_t(config->get_uint("engine.max_materials", texture_manager_t::default_size_));
			mesh_manager_ = new mesh_manager_t(config->get_uint("engine.max_meshes", texture_manager_t::default_size_));
			vertex_format_manager_ = new vertex_format_manager_t(config->get_uint("engine.max_vertex_formats", vertex_format_manager_t::default_size_));
			shader_program_manager_ = new shader_program_manager_t(config->get_uint("engine.max_shader_programs", shader_program_manager_t::default_size_));
			font_manager_ = new font_manager_t(config->get_uint("engine.max_fonts", font_manager_t::default_size_));
			skeleton_manager_ = new skeleton_manager_t(config->get_uint("engine.max_skeletons", skeleton_manager_t::default_size_));
			skeleton_animation_manager_ = new skeleton_animation_manager_t(config->get_uint("engine.max_skeleton_animations", skeleton_animation_manager_t::default_size_));

			// Check that the draw command bitfield (which is used for sorting draw commands) has enough bits allocated to each type
			// so that it can represent every possible handle_ value for that type.
			assert(mesh_manager_->get_capacity() <= (1<<draw_command_t::bits_per_mesh_) && "assign more bits to mesh in draw command bitfield");
			assert(material_manager_->get_capacity() <= (1<<draw_command_t::bits_per_material_) && "assign more bits to material in draw command bitfield");
			assert(texture_set_manager_->get_capacity() <= (1<<draw_command_t::bits_per_texture_set_) && "assign more bits to texture set in draw command bitfield");
			assert(vertex_format_manager_->get_capacity() <= (1<<draw_command_t::bits_per_vertex_format_) && "assign more bits to vertex format in draw command bitfield");
			assert(shader_program_manager_->get_capacity() <= (1<<draw_command_t::bits_per_shader_program_) && "assign more bits to shader program in draw command bitfield");
		}

		core_managers_t::~core_managers_t()
		{
			MAKI_SAFE_DELETE(skeleton_animation_manager_);
			MAKI_SAFE_DELETE(skeleton_manager_);
			MAKI_SAFE_DELETE(font_manager_);
			MAKI_SAFE_DELETE(shader_program_manager_);
			MAKI_SAFE_DELETE(vertex_format_manager_);
			MAKI_SAFE_DELETE(mesh_manager_);
			MAKI_SAFE_DELETE(material_manager_);
			MAKI_SAFE_DELETE(texture_set_manager_);
			MAKI_SAFE_DELETE(texture_manager_);
		}

		void core_managers_t::dump_manager_stats()
		{
			vertex_format_manager_->dump_stats		("Vertex formats  ");
			shader_program_manager_->dump_stats		("Shader programs ");
			texture_set_manager_->dump_stats		("Texture sets    ");
			texture_manager_->dump_stats			("Textures        ");
			material_manager_->dump_stats			("Materials       ");
			mesh_manager_->dump_stats				("Meshes          ");
			font_manager_->dump_stats				("Fonts           ");
			skeleton_manager_->dump_stats			("Skeletons       ");
			skeleton_animation_manager_->dump_stats	("Skeleton anims  ");
		}

		void core_managers_t::reload_assets()
		{
			engine_t *eng = engine_t::get();
			if(eng != nullptr) {
				eng->renderer_->sync_with_core();
			}

			console_t::info("Reloading all hot-swappable assets");
			mesh_manager_->reload_assets();
			material_manager_->reload_assets();
			texture_manager_->reload_assets();
			shader_program_manager_->reload_assets();
			skeleton_manager_->reload_assets();
			skeleton_animation_manager_->reload_assets();
		
			dump_manager_stats();
		}

		bool core_managers_t::reload_asset(rid_t rid)
		{
			engine_t *eng = engine_t::get();
			if(eng != nullptr) {
				eng->renderer_->sync_with_core();
			}

			// Caution, relies on short-circuit evaluation
			if(	texture_manager_->reload_asset(rid) ||
				shader_program_manager_->reload_asset(rid) ||
				material_manager_->reload_asset(rid) ||
				mesh_manager_->reload_asset(rid) ||
				skeleton_manager_->reload_asset(rid) ||
				skeleton_animation_manager_->reload_asset(rid)
			) {
				return true;
			}
			return false;
		}

	} // namespace core

} // namespace maki
