#pragma once
#include "lualayer/lualayer_stdafx.h"
#include <lua.hpp>

namespace maki
{
	namespace lualayer
	{

		class lua_context_t
		{
		public:
			lua_context_t();
			virtual ~lua_context_t();
			bool eval(rid_t main_script);
			void update(float dt);
			void draw();

		private:
			lua_State *L_;
			int32 update_function_ref_;
			int32 draw_function_ref_;
		};

	} // namespace lualayer

} // namespace maki
