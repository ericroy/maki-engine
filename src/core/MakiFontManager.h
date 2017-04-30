#pragma once
#include "core/core_stdafx.h"
#include "core/MakiFont.h"
#include "core/MakiManager.h"

namespace maki
{
	namespace core
	{
		class font_manager_t;
		

		class font_manager_t : public manager_t<font_t, font_manager_t>
		{
		private:
			struct predicate_t : std::unary_function<const font_t *, bool>
			{
				inline bool operator()(const font_t *font) const;
				rid_t shader_program_rid_;
				rid_t font_rid_;
				uint32_t pixel_size_;
			};
		
		public:
			static const int32_t default_size_ = 8;

		public:
			font_manager_t(uint32_t size = default_size_);	
			virtual ~font_manager_t();
			handle_t load(rid_t shader_program_rid, rid_t font_rid, uint32_t pixel_size);
		};

	} // namespace core

} // namespace maki
