#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"
#include "core/MakiShader.h"

namespace maki
{
	namespace core
	{

		class shader_program_t : public resource_t
		{
		public:
			enum variant_t
			{
				variant_normal_ = 0,
				variant_depth_,
				variant_shadow_,
				variant_count_
			};

		private:
			static const char *variant_data_key_[variant_count_];
			static const char *variant_meta_key_[variant_count_];

		public:
			shader_program_t();
			shader_program_t(const move_token_t<shader_program_t> &) { assert(false && "shader_program_t move construction not allowed"); }
			shader_program_t(const shader_program_t &) { assert(false && "shader_program_t copy construction not allowed"); }
			~shader_program_t();

			bool load(rid_t rid, variant_t variant);

			inline bool operator==(const shader_program_t &other) const
			{
				return rid_ == other.rid_ && variant_ == other.variant_;
			}

		public:
			uint32_t input_attribute_count_;
			shader_t vertex_shader_;
			shader_t pixel_shader_;
			variant_t variant_;
			handle_t variants_[variant_count_-1];
			intptr_t handle_;
		};

	} // namespace core

} // namespace maki
