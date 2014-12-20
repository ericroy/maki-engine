#pragma once
#include "core/core_stdafx.h"
#include "core/MakiEngine.h"
#include "core/MakiMeshManager.h"
#include "core/MakiMaterialManager.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiShaderProgramManager.h"
#include "core/MakiTextureSetManager.h"

namespace maki
{
	namespace d3d
	{
		class d3d_render_core_t;
	}

	namespace ogl
	{
		class ogl_render_core_t;
	}

	namespace core
	{	
		class mesh_t;
		class material_t;

		class __declspec(align(16)) draw_command_t : public aligned_t<16>
		{
		friend class render_core_t;
		friend class d3d::d3d_render_core_t;
		friend class ogl::ogl_render_core_t;
		
		friend class draw_command_list_t;

		public:
			static const int32 bits_per_mesh_ = 12;
			static const int32 bits_per_material_ = 8;
			static const int32 bits_per_texture_set_ = 7;
			static const int32 bits_per_shader_program_ = 6;
			static const int32 bits_per_vertex_format_ = 4;

			enum translucency_type_t
			{
				translucency_type_opaque_ = 0,
				translucency_type_translucent_,
			};

			// Listed from low to high bits (low to high sorting priority)
			struct key_fields_t
			{
				// These fields contain the actual handle value to a resource.
				// Make sure there are enough bits to represent every handle up to the maximum size of the manager's pool.
				uint64 mesh_ : bits_per_mesh_;
				uint64 mesh_manager_id_ : mesh_manager_t::bits_per_manager_id_;

				uint64 material_ : bits_per_material_;
				uint64 material_manager_id_ : material_manager_t::bits_per_manager_id_;

				uint64 texture_set_ : bits_per_texture_set_;
				uint64 material_manager_id_ : texture_set_manager_t::bits_per_manager_id_;

				uint64 shader_program_ : bits_per_shader_program_;
				uint64 shader_program_manager_id_ : shader_program_manager_t::bits_per_manager_id_;

				uint64 vertex_format_ : bits_per_vertex_format_;
				uint64 vertex_format_manager_id_ : vertex_format_manager_t::bits_per_manager_id_;

				uint64 inverse_depth_ : 11;

				// Other higher priority sorting properties
				uint64 translucency_type_ : 1;
			};

		public:
			draw_command_t();
			draw_command_t(const move_token_t<draw_command_t> &other);
			~draw_command_t();

			inline uint64 get_key() const { return key_; }
			void set_mesh(handle_t mesh);
			void set_material(handle_t material);
			inline void clear();
			inline void copy(const draw_command_t &other);

		private:
			draw_command_t(const draw_command_t &other) {}

		private:
			union
			{
				key_fields_t fields_;	
				uint64 key_;
			};
		
			handle_t mesh_;
			handle_t material_;
			handle_t texture_set_;
			handle_t shader_program_;
			handle_t vertex_format_;
		};

		static_assert(sizeof(draw_command_t::key_fields_t) == 8, "draw_command_t key has exceeded 64 bits");



		inline void draw_command_t::copy(const draw_command_t &other)
		{
			key_ = other.key_;

			mesh_ = other.mesh_;
			material_ = other.material_;
			shader_program_ = other.shader_program_;
			texture_set_ = other.texture_set_;
			vertex_format_ = other.vertex_format_;

			mesh_manager_t::add_ref(mesh_);
			material_manager_t::add_ref(material_);
			shader_program_manager_t::add_ref(shader_program_);
			texture_set_manager_t::add_ref(texture_set_);
			vertex_format_manager_t::add_ref(vertex_format_);
		}
	
		inline void draw_command_t::clear()
		{
			key_ = 0;

			mesh_manager_t::free(mesh_);
			material_manager_t::free(material_);
			shader_program_manager_t::free(shader_program_);
			texture_set_manager_t::free(texture_set_);
			vertex_format_manager_t::free(vertex_format_);
		}


	} // namespace core

} // namespace maki
