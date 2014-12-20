#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		class render_state_t;
		class draw_command_list_t;
		class core_managers_t;

		class render_payload_t
		{
		public:
			enum command_t
			{
				command_abort_ = 0,
				command_init_,
				command_draw_,
				command_present_,
			};

		public:
			render_payload_t() : cmd_(command_abort_), state_(nullptr), commands_(nullptr) {}
			render_payload_t(command_t cmd) : cmd_(cmd), state_(nullptr), commands_(nullptr) {}

		public:
			command_t cmd_;
			render_state_t *state_;
			draw_command_list_t *commands_;
		};

	} // namespace core

} // namespace maki
