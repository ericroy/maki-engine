#pragma once
#include "core/MakiTypes.h"

namespace maki {
	namespace core {

		enum key_id_t : uint8_t {
			key_l_button = 0x01,
			key_r_button = 0x02,
			key_cancel = 0x03,
			key_m_button = 0x04,
			key_x_button1 = 0x05,
			key_x_button2 = 0x06,
			key_back = 0x08,
			key_tab = 0x09,
			key_clear = 0x0c,
			key_return = 0x0d,
			key_shift = 0x10,
			key_control = 0x11,
			key_menu = 0x12,
			key_pause = 0x13,
			key_capital = 0x14,
			key_escape = 0x1b,
			key_convert = 0x1c,
			key_nonconvert = 0x1d,
			key_accept = 0x1e,
			key_modechange = 0x1f,
			key_space = 0x20,
			key_prior = 0x21,
			key_next = 0x22,
			key_end = 0x23,
			key_home = 0x24,
			key_left = 0x25,
			key_up = 0x26,
			key_right = 0x27,
			key_down = 0x28,
			key_select = 0x29,
			key_print = 0x2a,
			key_execute = 0x2b,
			key_snapshot = 0x2c,
			key_insert = 0x2d,
			key_delete = 0x2e,
			key_help = 0x2f,
			key_0 = 0x30,
			key_1 = 0x31,
			key_2 = 0x32,
			key_3 = 0x33,
			key_4 = 0x34,
			key_5 = 0x35,
			key_6 = 0x36,
			key_7 = 0x37,
			key_8 = 0x38,
			key_9 = 0x39,
			key_a = 0x41,
			key_b = 0x42,
			key_c = 0x43,
			key_d = 0x44,
			key_e = 0x45,
			key_f = 0x46,
			key_g = 0x47,
			key_h = 0x48,
			key_i = 0x49,
			key_j = 0x4a,
			key_k = 0x4b,
			key_l = 0x4c,
			key_m = 0x4d,
			key_n = 0x4e,
			key_o = 0x4f,
			key_p = 0x50,
			key_q = 0x51,
			key_r = 0x52,
			key_s = 0x53,
			key_t = 0x54,
			key_u = 0x55,
			key_v = 0x56,
			key_w = 0x57,
			key_x = 0x58,
			key_y = 0x59,
			key_z = 0x5a,
			key_l_win = 0x5b,
			key_r_win = 0x5c,
			key_apps = 0x5d,
			key_sleep = 0x5f,
			key_numpad0 = 0x60,
			key_numpad1 = 0x61,
			key_numpad2 = 0x62,
			key_numpad3 = 0x63,
			key_numpad4 = 0x64,
			key_numpad5 = 0x65,
			key_numpad6 = 0x66,
			key_numpad7 = 0x67,
			key_numpad8 = 0x68,
			key_numpad9 = 0x69,
			key_multiply = 0x6a,
			key_add = 0x6b,
			key_separator = 0x6c,
			key_subtract = 0x6d,
			key_decimal = 0x6e,
			key_divide = 0x6f,
			key_f1 = 0x70,
			key_f2 = 0x71,
			key_f3 = 0x72,
			key_f4 = 0x73,
			key_f5 = 0x74,
			key_f6 = 0x75,
			key_f7 = 0x76,
			key_f8 = 0x77,
			key_f9 = 0x78,
			key_f10 = 0x79,
			key_f11 = 0x7a,
			key_f12 = 0x7b,
			key_f13 = 0x7c,
			key_f14 = 0x7d,
			key_f15 = 0x7e,
			key_f16 = 0x7f,
			key_f17 = 0x80,
			key_f18 = 0x81,
			key_f19 = 0x82,
			key_f20 = 0x83,
			key_f21 = 0x84,
			key_f22 = 0x85,
			key_f23 = 0x86,
			key_f24 = 0x87,
			key_numlock = 0x90,
			key_scroll = 0x91,
			key_l_shift = 0xa0,
			key_r_shift = 0xa1,
			key_l_control = 0xa2,
			key_r_control = 0xa3,
			key_l_menu = 0xa4,
			key_r_menu = 0xa5,
			key_browser_back = 0xa6,
			key_browser_forward = 0xa7,
			key_browser_refresh = 0xa8,
			key_browser_stop = 0xa9,
			key_browser_search = 0xaa,
			key_browser_favorites = 0xab,
			key_browser_home = 0xac,
			key_volume_mute = 0xad,
			key_volume_down = 0xae,
			key_volume_up = 0xaf,
			key_media_next_track = 0xb0,
			key_media_prev_track = 0xb1,
			key_media_stop = 0xb2,
			key_media_play_pause = 0xb3,

			key_unknown = 0xff,
			key_max = key_unknown,
		};

		enum button_id_t {
			button_invalid = -1,

			// digital stuff, values 0 or 1
			button_dpad_up = 0, button_dpad_down, button_dpad_left, button_dpad_right,
			button_start, button_back,
			button_left_thumb, button_right_thumb,
			button_left_shoulder, button_right_shoulder,
			button_a, button_b, button_x, button_y,

			// Potentially analog stuff, values [0, 1]
			button_left_trigger, button_right_trigger,
			button_left_thumb_x, button_left_thumb_y,
			button_right_thumb_x, button_right_thumb_y,

			button_max = button_right_thumb_y
		};

		struct key_state_t {
			// True if the key is currently depressed
			uint8_t on : 1;
			// True if the key went down in the last tick
			uint8_t pressed : 1;
			// True if the key went up in the last tick
			uint8_t released : 1;
		};

		// Reports the history of a key since the last tick
		struct key_state_report_t {
			inline void set(bool on) {
				this->on = on ? 1 : 0;
				transition_count++;
			}

			uint8_t on : 1;
			uint8_t transition_count : 7;
		};

		class controller_t {
		public:
			controller_t();
			inline bool connected() const { return connected_; }
			inline void set_connected(bool connected) { connected_ = connected; }
			inline float get_float(button_id_t b) const { return values_[mapping_[b]]; }
			inline bool get_bool(button_id_t b) const { return values_[mapping_[b]] == 1.0f; }
			void map_button(button_id_t hardware_button, button_id_t virtual_button);
			void set_identity_mapping();
			void set_values(const float values[button_max + 1]);
		private:
			bool connected_ = false;
			float values_[button_max + 1] = {};
			uint8_t mapping_[button_max + 1];
		};

		class player_t {
		public:
			player_t();
			// Returns true if the key is currently held down
			inline bool get_key(key_id_t k) const { return key_states_[key_mapping_[k]].on != 0; }
			// Returns true if the key became depressed during the last tick
			inline bool get_key_pressed(key_id_t k) const { return key_states_[key_mapping_[k]].pressed != 0; }
			// Returns true if the key was released during the last tick
			inline bool get_key_released(key_id_t k) const { return key_states_[key_mapping_[k]].released != 0; }
			inline controller_t *controller() { return controller_; }
			inline void set_controller(controller_t *controller) { controller_ = controller; }
			void map_key(key_id_t hardware_key, key_id_t virtual_key);
			void set_identity_mapping();
			void set_key_states(const key_state_report_t states[key_max + 1]);
		private:
			controller_t *controller_ = nullptr;
			uint8_t key_mapping_[key_max + 1];
			key_state_t key_states_[key_max + 1];
		};


		class input_state_t {
		public:
			static const int32_t max_players = 4;

		public:
			input_state_t() = default;

			// You may hold a reference to the player object
			inline player_t *get_player(uint8_t index) {
				MAKI_ASSERT(index < max_players);
				return &players_[index];
			}

			// Don't hold a reference to the controller object - get it each time from the player
			// in case the controller associated with the player changes
			inline controller_t &get_controller(uint8_t index) {
				MAKI_ASSERT(index < max_players);
				return controllers_[index];
			}

			void connect_controller(uint8_t controller_index);
			void disconnect_controller(uint8_t controller_index);

		private:
			uint8_t controller_count_ = 0;
			controller_t controllers_[max_players];
			player_t players_[max_players];
		};

	} // namespace core
} // namespace maki
