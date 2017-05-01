#pragma once
#include "core/MakiAllocator.h"
#include "core/MakiEngine.h"
#include "core/MakiMeshManager.h"
#include "core/MakiMaterialManager.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiShaderProgramManager.h"
#include "core/MakiTextureSetManager.h"

namespace maki {

	namespace d3d {
		class d3d_render_core_t;
	}

	namespace ogl {
		class ogl_render_core_t;
	}

	namespace core {

		class mesh_t;
		class material_t;

		class __declspec(align(16)) draw_command_t : public aligned_t<16> {
			friend class render_core_t;
			friend class d3d::d3d_render_core_t;
			friend class ogl::ogl_render_core_t;
			friend class draw_command_list_t;

		public:
			static const int32_t bits_per_mesh = 12;
			static const int32_t bits_per_material = 8;
			static const int32_t bits_per_texture_set = 7;
			static const int32_t bits_per_shader_program = 6;
			static const int32_t bits_per_vertex_format = 4;

			enum translucency_type_t {
				translucency_type_opaque = 0,
				translucency_type_translucent,
			};

			// Listed from low to high bits (low to high sorting priority)
			struct key_fields_t {
				// These fields contain the actual handle_ value to a resource.
				// Make sure there are enough bits to represent every handle_ up to the maximum size of the manager's pool.
				uint64_t mesh : bits_per_mesh;
				uint64_t mesh_manager_id : mesh_manager_t::bits_per_manager_id;

				uint64_t material : bits_per_material;
				uint64_t material_manager_id : material_manager_t::bits_per_manager_id;

				uint64_t texture_set : bits_per_texture_set;
				uint64_t texture_set_manager_id_ : texture_set_manager_t::bits_per_manager_id;

				uint64_t shader_program : bits_per_shader_program;
				uint64_t shader_program_manager_id_ : shader_program_manager_t::bits_per_manager_id;

				uint64_t vertex_format : bits_per_vertex_format;
				uint64_t vertex_format_manager_id : vertex_format_manager_t::bits_per_manager_id;

				uint64_t inverse_depth : 11;

				// Other higher priority sorting properties
				uint64_t translucency_type : 1;
			};

		public:
			draw_command_t() = default;
			draw_command_t(draw_command_t &&other);
			~draw_command_t();

			inline uint64_t get_key() const { return key_; }
			void set_mesh(handle_t mesh);
			void set_material(handle_t material);
			inline void clear();
			inline void copy(const draw_command_t &other);

		private:
			union {
				key_fields_t fields_;	
				uint64_t key_ = 0;
			};
		
			handle_t mesh_ = HANDLE_NONE;
			handle_t material_ = HANDLE_NONE;
			handle_t texture_set_ = HANDLE_NONE;
			handle_t shader_program_ = HANDLE_NONE;
			handle_t vertex_format_ = HANDLE_NONE;
		};

		static_assert(sizeof(draw_command_t::key_fields_t) == 8, "draw_command_t key has exceeded 64 bits");



		inline void draw_command_t::copy(const draw_command_t &other) {
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
	
		inline void draw_command_t::clear() {
			key_ = 0;

			mesh_manager_t::free(mesh_);
			material_manager_t::free(material_);
			shader_program_manager_t::free(shader_program_);
			texture_set_manager_t::free(texture_set_);
			vertex_format_manager_t::free(vertex_format_);
		}


	} // namespace core
} // namespace maki
