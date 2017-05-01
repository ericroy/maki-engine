#include "core/MakiDrawCommand.h"
#include "core/MakiEngine.h"
#include "core/MakiMeshManager.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiTextureSetManager.h"
#include "core/MakiShaderProgramManager.h"


namespace maki {
	namespace core {

		draw_command_t::draw_command_t(draw_command_t &&other)
			: key_(other.key_),
			mesh_(HANDLE_NONE),
			material_(HANDLE_NONE),
			shader_program_(HANDLE_NONE),
			texture_set_(HANDLE_NONE),
			vertex_format_(HANDLE_NONE) {
			std::swap(mesh_, other.mesh_);
			std::swap(material_, other.material_);
			std::swap(shader_program_, other.shader_program_);
			std::swap(texture_set_, other.texture_set_);
			std::swap(vertex_format_, other.vertex_format_);
		}
	
		draw_command_t::~draw_command_t() {
			clear();
		}
	
		void draw_command_t::set_mesh(handle_t mesh_handle) {
			mesh_manager_t::free(mesh_);
			vertex_format_manager_t::free(vertex_format_);
		
			if(mesh_handle != HANDLE_NONE) {
				mesh_manager_t::add_ref(mesh_handle);
				mesh_ = mesh_handle;
				fields_.mesh_ = mesh_ & mesh_manager_t::handle_value_mask;
				fields_.mesh_manager_id_ = mesh_ >> mesh_manager_t::manager_id_shift;
		
				const mesh_t *m = mesh_manager_t::get(mesh_);
				if(m->get_mesh_flag(mesh_t::mesh_flag_has_translucency_)) {
					fields_.translucency_type_ = translucency_type_translucent;
				}
			
				if(m->vertex_format_ != HANDLE_NONE) {
					vertex_format_manager_t::add_ref(m->vertex_format_);
					vertex_format_ = m->vertex_format;
					fields_.vertex_format_ = vertex_format_ & vertex_format_manager_t::handle_value_mask;
					fields_.vertex_format_manager_id_ = vertex_format_ >> vertex_format_manager_t::manager_id_shift;
				}
			}
		}

		void draw_command_t::set_material(handle_t material_handle) {
			material_manager_t::free(material_);
			texture_set_manager_t::free(texture_set_);
			shader_program_manager_t::free(shader_program_);

			if(material_handle != HANDLE_NONE) {
				material_manager_t::add_ref(material_handle);
				material_ = material_handle;
				fields_.material_ = material_ & material_manager_t::handle_value_mask;
				fields_.material_manager_id_ = material_ >> material_manager_t::manager_id_shift;

				const material_t *mat = material_manager_t::get(material_);

				if(mat->texture_set_ != HANDLE_NONE) {
					texture_set_manager_t::add_ref(mat->texture_set_);
					texture_set_ = mat->texture_set;
					fields_.texture_set_ = texture_set_ & texture_set_manager_t::handle_value_mask;
					fields_.material_manager_id_ = texture_set_ >> texture_set_manager_t::manager_id_shift;
				}

				if(mat->shader_program_ != HANDLE_NONE) {
					shader_program_manager_t::add_ref(mat->shader_program_);
					shader_program_ = mat->shader_program;
					fields_.shader_program_ = shader_program_ & shader_program_manager_t::handle_value_mask;
					fields_.shader_program_manager_id_ = shader_program_ >> shader_program_manager_t::manager_id_shift;
				}
			}
		}

	} // namespace core
} // namespace maki
