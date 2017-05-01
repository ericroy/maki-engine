#pragma once
#include "core/MakiResource.h"
#include "core/MakiShaderProgram.h"

namespace maki {
	namespace core {

		class material_t : public resource_t
		{
		public:
			static const int32_t max_uniforms_ = 16;

			struct uniform_value_t
			{
				uniform_value_t()
					: data_(nullptr), bytes_(0), vs_location_(-1), ps_location_(-1)
				{
				}
				~uniform_value_t()
				{
					MAKI_SAFE_FREE(data_);
				}

				// Offset within the shader
				int32_t vs_location_;
				int32_t ps_location_;
			
				// Actual data for this constant
				uint32_t bytes_;
				char *data_;
			};

		public:
			material_t();
			material_t(material_t &&other);
			explicit material_t(const material_t &other);
			virtual ~material_t();
			bool load(rid_t rid);
			void set_shader_program(rid_t shader_rid);
			void set_textures(uint8_t count, rid_t *texture_rids);

			// Push a constant buffer and give ownership of the pointer to this object.
			// Pointer must be allocated with Maki::allocator_t.
			// Returns the index of the constant value in this material.
			// Returns -1 on failure (provided pointer will be freed in this case).
			int32_t push_constant(const char *key, uint32_t bytes, char *data);
		
		public:
			handle_t texture_set_;
			handle_t shader_program_;
			uint8_t uniform_count_;
			uniform_value_t uniform_values_[max_uniforms_];
		};



	} // namespace core
} // namespace maki
