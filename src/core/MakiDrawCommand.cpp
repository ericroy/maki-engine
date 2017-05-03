#include "core/MakiDrawCommand.h"
#include "core/MakiEngine.h"
#include "core/MakiMeshManager.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiTextureSetManager.h"
#include "core/MakiShaderProgramManager.h"


namespace maki {
	namespace core {

		draw_command_t::draw_command_t(draw_command_t &&other) {
			mesh_ = move(other.mesh_);
			material_ = move(other.material_);
			shader_program_ = move(other.shader_program_);
			texture_set_ = move(other.texture_set_);
			vertex_format_ = move(other.vertex_format_);
		}
	
		draw_command_t::~draw_command_t() {
			clear();
		}
	
		void draw_command_t::set_mesh(const ref_t<mesh_t> &mesh) {
			mesh_.release();
			vertex_format_.release();
			if(mesh) {
				mesh_ = mesh;
				fields_.mesh = mesh_.handle();
				if(mesh_->has_mesh_flag(mesh_t::mesh_flag_has_translucency))
					fields_.translucency_type = translucency_type_translucent;
				if (mesh_->vertex_format()) {
					vertex_format_ = mesh_->vertex_format();
					fields_.vertex_format = vertex_format_.handle();
				}
			}
		}

		void draw_command_t::set_material(const ref_t<material_t> &material) {
			material_.release();
			texture_set_.release();
			shader_program_.release();
			if(material) {
				material_ = material;
				fields_.material = material_.handle();
				if(material->texture_set()) {
					texture_set_ = material->texture_set();
					fields_.texture_set = texture_set_.handle();
				}
				if(material->shader_program()) {
					shader_program_ = material->shader_program();
					fields_.shader_program = shader_program_.handle();
				}
			}
		}

	} // namespace core
} // namespace maki
