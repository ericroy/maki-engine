#pragma once
#include "core/MakiAllocator.h"
#include "core/MakiEngine.h"
#include "core/MakiResourcePool.h"
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
				// These fields contain the actual handle value to a resource.
				uint64_t mesh : bits_per_mesh;
				uint64_t material : bits_per_material;
				uint64_t texture_set : bits_per_texture_set;
				uint64_t shader_program : bits_per_shader_program;
				uint64_t vertex_format : bits_per_vertex_format;

				uint64_t inverse_depth : 11;

				// Other higher priority sorting properties
				uint64_t translucency_type : 1;
			};
			static_assert(sizeof(key_fields_t) <= sizeof(uint64_t), "Draw command fields have exceeded one uint64 in size!");

		public:
			draw_command_t() = default;
			draw_command_t(draw_command_t &&other);
			~draw_command_t();

			inline uint64_t get_key() const { return key_; }
			void set_mesh(const ref_t<mesh_t> &mesh);
			void set_material(const ref_t<material_t> &material);
			inline void clear();
			inline void copy(const draw_command_t &other);

		private:
			union {
				key_fields_t fields_;	
				uint64_t key_ = 0;
			};
			ref_t<mesh_t> mesh_;
			ref_t<material_t> material_;
			ref_t<texture_set_t> texture_set_;
			ref_t<shader_program_t> shader_program_;
			ref_t<vertex_format_t> vertex_format_;
		};

		static_assert(sizeof(draw_command_t::key_fields_t) == 8, "draw_command_t key has exceeded 64 bits");



		inline void draw_command_t::copy(const draw_command_t &other) {
			key_ = other.key_;
			mesh_ = other.mesh_;
			material_ = other.material_;
			shader_program_ = other.shader_program_;
			texture_set_ = other.texture_set_;
			vertex_format_ = other.vertex_format_;
		}
	
		inline void draw_command_t::clear() {
			key_ = 0;
			mesh_.release();
			material_.release();
			texture_set_.release();
			shader_program_.release();
			vertex_format_.release();
		}


	} // namespace core
} // namespace maki
