#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"

namespace maki
{
	namespace core
	{

		class texture_t : public resource_t
		{
		public:
			enum texture_type_t
			{
				texture_type_regular_ = 0,
				texture_type_depth_stencil_,
				texture_type_render_target_,
			};

		public:
			texture_t();
			texture_t(const move_token_t<texture_t> &) { assert(false && "texture_t move construction not allowed"); }
			texture_t(const texture_t &) { assert(false && "texture_t copy construction not allowed"); }
			~texture_t();

		public:
			texture_type_t type_;
			uint32 width_;
			uint32 height_;
			uint32 handle_;
		};

	} // namespace core

} // namespace maki
