#pragma once
#include "core/MakiTypes.h"
#include "core/MakiPseudoSingleton.h"

namespace maki {
	namespace core {

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

		class core_managers_t : public pseudo_singleton_t<core_managers_t> {
			MAKI_NO_COPY(core_managers_t);
		public:
			core_managers_t() = default;
			core_managers_t(const config_t *config);

		public:
			unique_ptr<texture_manager_t> texture_manager;
			unique_ptr<texture_set_manager_t> texture_set_manager;
			unique_ptr<material_manager_t> material_manager;
			unique_ptr<mesh_manager_t> mesh_manager;
			unique_ptr<vertex_format_manager_t> vertex_format_manager;
			unique_ptr<shader_program_manager_t> shader_program_manager;
			unique_ptr<font_manager_t> font_manager;
			unique_ptr<skeleton_manager_t> skeleton_manager;
			unique_ptr<skeleton_animation_manager_t> skeleton_animation_manager;
		};

	} // namespace core
} // namespace maki
