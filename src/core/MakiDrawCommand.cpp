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
			: key_(0),
			mesh_(HANDLE_NONE),
			material_(HANDLE_NONE),
			shader_program_(HANDLE_NONE),
			texture_set_(HANDLE_NONE),
			vertex_format_(HANDLE_NONE)
		{
		}

		draw_command_t::draw_command_t(const move_token_t<draw_command_t> &other)
			: key_(other.obj_->key_),
			mesh_(HANDLE_NONE),
			material_(HANDLE_NONE),
			shader_program_(HANDLE_NONE),
			texture_set_(HANDLE_NONE),
			vertex_format_(HANDLE_NONE)
		{
			std::swap(mesh_, other.obj_->mesh_);
			std::swap(material_, other.obj_->material_);
			std::swap(shader_program_, other.obj_->shader_program_);
			std::swap(texture_set_, other.obj_->texture_set_);
			std::swap(vertex_format_, other.obj_->vertex_format_);
		}
	
		draw_command_t::~draw_command_t()
		{
			clear();
		}

	
		void draw_command_t::set_mesh(handle_t meshHandle)
		{
			mesh_manager_t::free(mesh_);
			vertex_format_manager_t::free(vertex_format_);
		
			if(meshHandle != HANDLE_NONE) {
				mesh_manager_t::add_ref(meshHandle);
				mesh_ = meshHandle;
				fields_.mesh_ = mesh_ & mesh_manager_t::handle_value_mask_;
				fields_.mesh_manager_id_ = mesh_ >> mesh_manager_t::manager_id_shift_;
		
				const mesh_t *m = mesh_manager_t::get(mesh_);
				if(m->GetMeshFlag(mesh_t::MeshFlag_HasTranslucency)) {
					fields_.translucency_type_ = translucency_type_translucent_;
				}
			
				if(m->vertex_format_ != HANDLE_NONE) {
					vertex_format_manager_t::add_ref(m->vertex_format_);
					vertex_format_ = m->vertex_format_;
					fields_.vertex_format_ = vertex_format_ & vertex_format_manager_t::handle_value_mask_;
					fields_.vertex_format_manager_id_ = vertex_format_ >> vertex_format_manager_t::manager_id_shift_;
				}
			}
		}

		void draw_command_t::set_material(handle_t materialHandle)
		{
			material_manager_t::free(material_);
			texture_set_manager_t::free(texture_set_);
			shader_program_manager_t::free(shader_program_);

			if(materialHandle != HANDLE_NONE) {
				material_manager_t::add_ref(materialHandle);
				material_ = materialHandle;
				fields_.material_ = material_ & material_manager_t::handle_value_mask_;
				fields_.material_manager_id_ = material_ >> material_manager_t::manager_id_shift_;

				const material_t *mat = material_manager_t::get(material_);

				if(mat->texture_set_ != HANDLE_NONE) {
					texture_set_manager_t::add_ref(mat->texture_set_);
					texture_set_ = mat->texture_set_;
					fields_.texture_set_ = texture_set_ & texture_set_manager_t::handle_value_mask_;
					fields_.material_manager_id_ = texture_set_ >> texture_set_manager_t::manager_id_shift_;
				}

				if(mat->shader_program_ != HANDLE_NONE) {
					shader_program_manager_t::add_ref(mat->shader_program_);
					shader_program_ = mat->shader_program_;
					fields_.shader_program_ = shader_program_ & shader_program_manager_t::handle_value_mask_;
					fields_.shader_program_manager_id_ = shader_program_ >> shader_program_manager_t::manager_id_shift_;
				}
			}
		}

	} // namespace core

} // namespace maki
