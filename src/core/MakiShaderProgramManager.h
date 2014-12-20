#pragma once
#include "core/core_stdafx.h"
#include "core/MakiManager.h"
#include "core/MakiShaderProgram.h"

namespace maki
{
	namespace core
	{

		class shader_program_manager_t;
		

		class shader_program_manager_t : public manager_t<shader_program_t, shader_program_manager_t>
		{
		public:
			struct find_predicate_t : public std::unary_function<const shader_program_t *, bool>
			{
				find_predicate_t(rid_t rid, shader_program_t::variant_t variant) : rid_(rid), variant_(variant) {}
				inline bool operator()(const shader_program_t *res) const
				{
					return res->rid_ == rid_ && res->variant_ == variant_;
				}
				rid_t rid_;
				shader_program_t::variant_t variant_;
			};

			static const int32 default_size_ = 64;

		public:
			shader_program_manager_t(uint32 size = default_size_);	
			virtual ~shader_program_manager_t();
			handle_t load(rid_t rid, shader_program_t::variant_t variant = shader_program_t::variant_normal_);
			void reload_assets();
			bool reload_asset(rid_t rid);
		};

	} // namespace core

} // namespace maki
