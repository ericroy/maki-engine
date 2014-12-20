#pragma once
#include "core/core_stdafx.h"
#include "core/MakiPseudoSingleton.h"

namespace maki
{
	namespace core
	{
		class config_t;
		class draw_command_manager_t;
		class texture_manager_t;
		class texture_set_manager_t;
		class material_manager_t;
		class mesh_manager_t;
		class vertex_format_manager_t;
		class shader_program_manager_t;
		class font_manager_t;
		class skeleton_manager_t;
		class skeleton_animation_manager_t;

		class core_managers_t : public pseudo_singleton_t<core_managers_t>
		{
		public:
			core_managers_t();
			core_managers_t(const config_t *config);
			virtual ~core_managers_t();
			void dump_manager_stats();
			virtual void reload_assets();
			virtual bool reload_asset(rid_t rid);

		public:
			texture_manager_t *texture_manager_;
			texture_set_manager_t *texture_set_manager_;
			material_manager_t *material_manager_;
			mesh_manager_t *mesh_manager_;
			vertex_format_manager_t *vertex_format_manager_;
			shader_program_manager_t *shader_program_manager_;
			font_manager_t *font_manager_;
			skeleton_manager_t *skeleton_manager_;
			skeleton_animation_manager_t *skeleton_animation_manager_;
		};

	} // namespace core

} // namespace maki
