#pragma once

namespace maki {
	namespace core {

		class render_state_t;
		class draw_command_list_t;
		class core_managers_t;

		class render_payload_t {
		public:
			enum command_t {
				command_abort = 0,
				command_init,
				command_draw,
				command_present,
			};

		public:
			render_payload_t() = default;
			render_payload_t(command_t cmd) : cmd(cmd) {}

		public:
			command_t cmd = command_abort;
			render_state_t *state = nullptr;
			draw_command_list_t *commands = nullptr;
		};

	} // namespace core
} // namespace maki
