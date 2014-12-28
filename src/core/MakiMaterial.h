#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"
#include "core/MakiShaderProgram.h"

namespace maki
{
	namespace core
	{

		class material_t : public resource_t
		{
		public:
			static const int32 max_uniforms_ = 16;

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
				int32 vs_location_;
				int32 ps_location_;
			
				// Actual data for this constant
				uint32 bytes_;
				char *data_;
			};

		public:
			material_t();
			material_t(const move_token_t<material_t> &other);
			explicit material_t(const material_t &other);
			virtual ~material_t();
			bool load(rid_t rid);
			void set_shader_program(rid_t shader_rid);
			void set_textures(uint8 count, rid_t *texture_rids);

			// Push a constant buffer and give ownership of the pointer to this object.
			// Pointer must be allocated with Maki::allocator_t.
			// Returns the index of the constant value in this material.
			// Returns -1 on failure (provided pointer will be freed in this case).
			int32 push_constant(const char *key, uint32 bytes, char *data);
		
		public:
			handle_t texture_set_;
			handle_t shader_program_;
			uint8 uniform_count_;
			uniform_value_t uniform_values_[max_uniforms_];
		};



	} // namespace core

} // namespace maki
