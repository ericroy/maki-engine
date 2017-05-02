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

namespace maki {
	namespace core {

		core_managers_t::core_managers_t(const config_t *config) {
			texture_manager.reset(new texture_manager_t(config->get_uint("engine.max_textures", 64)));
			texture_set_manager.reset(new texture_set_manager_t(config->get_uint("engine.max_texture_sets", 64)));
			material_manager.reset(new material_manager_t(config->get_uint("engine.max_materials", 64)));
			mesh_manager.reset(new mesh_manager_t(config->get_uint("engine.max_meshes", 64)));
			vertex_format_manager.reset(new vertex_format_manager_t(config->get_uint("engine.max_vertex_formats", 64)));
			shader_program_manager.reset(new shader_program_manager_t(config->get_uint("engine.max_shader_programs", 64)));
			font_manager.reset(new font_manager_t(config->get_uint("engine.max_fonts", 64)));
			skeleton_manager.reset(new skeleton_manager_t(config->get_uint("engine.max_skeletons", 64)));
			skeleton_animation_manager.reset(new skeleton_animation_manager_t(config->get_uint("engine.max_skeleton_animations", 64)));

			// Check that the draw command bitfield (which is used for sorting draw commands) has enough bits allocated to each type
			// so that it can represent every possible handle_ value for that type.
			MAKI_ASSERT(mesh_manager_->get_capacity() <= (1 << draw_command_t::bits_per_mesh) && "assign more bits to mesh in draw command bitfield");
			MAKI_ASSERT(material_manager_->get_capacity() <= (1 << draw_command_t::bits_per_material) && "assign more bits to material in draw command bitfield");
			MAKI_ASSERT(texture_set_manager_->get_capacity() <= (1 << draw_command_t::bits_per_texture_set) && "assign more bits to texture set in draw command bitfield");
			MAKI_ASSERT(vertex_format_manager_->get_capacity() <= (1 << draw_command_t::bits_per_vertex_format) && "assign more bits to vertex format in draw command bitfield");
			MAKI_ASSERT(shader_program_manager_->get_capacity() <= (1 << draw_command_t::bits_per_shader_program) && "assign more bits to shader program in draw command bitfield");
		}

	} // namespace core
} // namespace maki
