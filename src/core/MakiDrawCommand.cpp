#include "core/core_stdafx.h"
#include "core/MakiDrawCommand.h"
#include "core/MakiEngine.h"
#include "core/MakiMeshManager.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiTextureSetManager.h"
#include "core/MakiShaderProgramManager.h"


namespace maki
{
	namespace core
	{

		draw_command_t::draw_command_t()
			: key(0),
			mesh(HANDLE_NONE),
			material(HANDLE_NONE),
			shader_program(HANDLE_NONE),
			texture_set(HANDLE_NONE),
			vertex_format_(HANDLE_NONE)
		{
		}

		draw_command_t::draw_command_t(const move_token_t<draw_command_t> &other)
			: key(other.obj->key),
			mesh(HANDLE_NONE),
			material(HANDLE_NONE),
			shader_program(HANDLE_NONE),
			texture_set(HANDLE_NONE),
			vertex_format_(HANDLE_NONE)
		{
			std::swap(mesh, other.obj->mesh_);
			std::swap(material, other.obj->material_);
			std::swap(shader_program, other.obj->shader_program_);
			std::swap(texture_set, other.obj->texture_set_);
			std::swap(vertex_format_, other.obj->vertex_format_);
		}
	
		draw_command_t::~draw_command_t()
		{
			clear();
		}

	
		void draw_command_t::set_mesh(handle_t meshHandle)
		{
			mesh_manager_t::free(mesh);
			vertex_format_manager_t::free(vertex_format_);
		
			if(meshHandle != HANDLE_NONE) {
				mesh_manager_t::add_ref(meshHandle);
				mesh = meshHandle;
				fields.mesh_ = mesh & mesh_manager_t::handle_value_mask_;
				fields.mesh_manager_id_ = mesh >> mesh_manager_t::manager_id_shift_;
		
				const mesh_t *m = mesh_manager_t::get(mesh);
				if(m->GetMeshFlag(mesh_t::MeshFlag_HasTranslucency)) {
					fields.translucency_type_ = translucency_type_translucent_;
				}
			
				if(m->vertex_format_ != HANDLE_NONE) {
					vertex_format_manager_t::add_ref(m->vertex_format_);
					vertex_format_ = m->vertex_format_;
					fields.vertex_format_ = vertex_format_ & vertex_format_manager_t::handle_value_mask_;
					fields.vertex_format_manager_id_ = vertex_format_ >> vertex_format_manager_t::manager_id_shift_;
				}
			}
		}

		void draw_command_t::set_material(handle_t materialHandle)
		{
			material_manager_t::free(material);
			texture_set_manager_t::free(texture_set);
			shader_program_manager_t::free(shader_program);

			if(materialHandle != HANDLE_NONE) {
				material_manager_t::add_ref(materialHandle);
				material = materialHandle;
				fields.material_ = material & material_manager_t::handle_value_mask_;
				fields.material_manager_id_ = material >> material_manager_t::manager_id_shift_;

				const material_t *mat = material_manager_t::get(material);

				if(mat->texture_set_ != HANDLE_NONE) {
					texture_set_manager_t::add_ref(mat->texture_set_);
					texture_set = mat->texture_set_;
					fields.texture_set_ = texture_set & texture_set_manager_t::handle_value_mask_;
					fields.texture_set_manager_id_ = texture_set >> texture_set_manager_t::manager_id_shift_;
				}

				if(mat->shader_program_ != HANDLE_NONE) {
					shader_program_manager_t::add_ref(mat->shader_program_);
					shader_program = mat->shader_program_;
					fields.shader_program_ = shader_program & shader_program_manager_t::handle_value_mask_;
					fields.shader_program_manager_id_ = shader_program >> shader_program_manager_t::manager_id_shift_;
				}
			}
		}

	} // namespace core

} // namespace maki
