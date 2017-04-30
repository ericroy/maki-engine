#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		class input_state_t
		{
		public:
			static const int32_t max_players_ = 4;

			enum key_t
			{
				key_l_button_ = 0x01,
				key_r_button_ = 0x02,
				key_cancel_ = 0x03,
				key_m_button_ = 0x04,
				key_x_button1_ = 0x05,
				key_x_button2_ = 0x06,
				key_back_ = 0x08,
				key_tab_ = 0x09,
				key_clear_ = 0x0c,
				key_return_ = 0x0d,
				key_shift_ = 0x10,
				key_control_ = 0x11,
				key_menu_ = 0x12,
				key_pause_ = 0x13,
				key_capital_ = 0x14,
				key_escape_ = 0x1b,
				key_convert_ = 0x1c,
				key_nonconvert_ = 0x1d,
				key_accept_ = 0x1e,
				key_modechange_ = 0x1f,
				key_space_ = 0x20,
				key_prior_ = 0x21,
				key_next_ = 0x22,
				key_end_ = 0x23,
				key_home_ = 0x24,
				key_left_ = 0x25,
				key_up_ = 0x26,
				key_right_ = 0x27,
				key_down_ = 0x28,
				key_select_ = 0x29,
				key_print_ = 0x2a,
				key_execute_ = 0x2b,
				key_snapshot_ = 0x2c,
				key_insert_ = 0x2d,
				key_delete_ = 0x2e,
				key_help_ = 0x2f,
				key_0_ = 0x30,
				key_1_ = 0x31,
				key_2_ = 0x32,
				key_3_ = 0x33,
				key_4_ = 0x34,
				key_5_ = 0x35,
				key_6_ = 0x36,
				key_7_ = 0x37,
				key_8_ = 0x38,
				key_9_ = 0x39,
				key_a_ = 0x41,
				key_b_ = 0x42,
				key_c_ = 0x43,
				key_d_ = 0x44,
				key_e_ = 0x45,
				key_f_ = 0x46,
				key_g_ = 0x47,
				key_h_ = 0x48,
				key_i_ = 0x49,
				key_j_ = 0x4a,
				key_k_ = 0x4b,
				key_l_ = 0x4c,
				key_m_ = 0x4d,
				key_n_ = 0x4e,
				key_o_ = 0x4f,
				key_p_ = 0x50,
				key_q_ = 0x51,
				key_r_ = 0x52,
				key_s_ = 0x53,
				key_t_ = 0x54,
				key_u_ = 0x55,
				key_v_ = 0x56,
				key_w_ = 0x57,
				key_x_ = 0x58,
				key_y_ = 0x59,
				key_z_ = 0x5a,
				key_l_win_ = 0x5b,
				key_r_win_ = 0x5c,
				key_apps_ = 0x5d,
				key_sleep_ = 0x5f,
				key_numpad0_ = 0x60,
				key_numpad1_ = 0x61,
				key_numpad2_ = 0x62,
				key_numpad3_ = 0x63,
				key_numpad4_ = 0x64,
				key_numpad5_ = 0x65,
				key_numpad6_ = 0x66,
				key_numpad7_ = 0x67,
				key_numpad8_ = 0x68,
				key_numpad9_ = 0x69,
				key_multiply_ = 0x6a,
				key_add_ = 0x6b,
				key_separator_ = 0x6c,
				key_subtract_ = 0x6d,
				key_decimal_ = 0x6e,
				key_divide_ = 0x6f,
				key_f1_ = 0x70,
				key_f2_ = 0x71,
				key_f3_ = 0x72,
				key_f4_ = 0x73,
				key_f5_ = 0x74,
				key_f6_ = 0x75,
				key_f7_ = 0x76,
				key_f8_ = 0x77,
				key_f9_ = 0x78,
				key_f10_ = 0x79,
				key_f11_ = 0x7a,
				key_f12_ = 0x7b,
				key_f13_ = 0x7c,
				key_f14_ = 0x7d,
				key_f15_ = 0x7e,
				key_f16_ = 0x7f,
				key_f17_ = 0x80,
				key_f18_ = 0x81,
				key_f19_ = 0x82,
				key_f20_ = 0x83,
				key_f21_ = 0x84,
				key_f22_ = 0x85,
				key_f23_ = 0x86,
				key_f24_ = 0x87,
				key_numlock_ = 0x90,
				key_scroll_ = 0x91,
				key_l_shift_ = 0xa0,
				key_r_shift_ = 0xa1,
				key_l_control_ = 0xa2,
				key_r_control_ = 0xa3,
				key_l_menu_ = 0xa4,
				key_r_menu_ = 0xa5,
				key_browser_back_ = 0xa6,
				key_browser_forward_ = 0xa7,
				key_browser_refresh_ = 0xa8,
				key_browser_stop_ = 0xa9,
				key_browser_search_ = 0xaa,
				key_browser_favorites_ = 0xab,
				key_browser_home_ = 0xac,
				key_volume_mute_ = 0xad,
				key_volume_down_ = 0xae,
				key_volume_up_ = 0xaf,
				key_media_next_track_ = 0xb0,
				key_media_prev_track_ = 0xb1,
				key_media_stop_ = 0xb2,
				key_media_play_pause_ = 0xb3,

				key_unknown_ = 0xff
			};

			enum button_t
			{
				button_invalid_ = -1,

				// digital stuff, values_ 0 or 1
				button_dpad_up_ = 0, button_dpad_down_, button_dpad_left_, button_dpad_right_,
				button_start_, button_back_,
				button_left_thumb_, button_right_thumb_,
				button_left_shoulder_, button_right_shoulder_,
				button_a_, button_b_, button_x_, button_y_,

				// Potentially analog stuff, values_ [0, 1]
				button_left_trigger_, button_right_trigger_,
				button_left_thumb_x_, button_left_thumb_y_,
				button_right_thumb_x_, button_right_thumb_y_,

				button_count_
			};

			// Reports the history of a key since the last tick
			struct key_state_report_t
			{
				inline void set(bool on)
				{
					on_ = on ? 1 : 0;
					transition_count_++;
				}

				uint8_t on_ : 1;
				uint8_t transition_count_ : 7;
			};


			struct controller_t
			{
			public:
				inline float get_float(button_t b) const { return values_[mapping_[b]]; }
				inline bool get_bool(button_t b) const { return values_[mapping_[b]] == 1.0f; }
				inline void map_button(button_t hardware_button, button_t virtual_button) { mapping_[virtual_button] = hardware_button; }

				inline void set_identity_mapping()
				{
					for(uint8_t i = 0; i < button_count_; i++) {
						mapping_[i] = i;
					}
				}

			public:
				bool connected_;
				uint8_t player_index_;

				float values_[button_count_];
				uint8_t mapping_[button_count_];
			};

			struct player_t
			{
			private:
				struct key_state_t
				{
					// True if the key is currently depressed
					uint8_t on_ : 1;

					// True if the key went down in the last tick
					uint8_t pressed_ : 1;
			
					// True if the key went up in the last tick
					uint8_t released_ : 1;
				};

			public:

				// Returns true if the key is currently held down
				inline bool get_key(key_t k) const { return key_states_[key_mapping_[k]].on_ != 0; }
			
				// Returns true if the key became depressed during the last tick
				inline bool get_key_pressed(key_t k) const { return key_states_[key_mapping_[k]].pressed_ != 0; }

				// Returns true if the key was released during the last tick
				inline bool get_key_released(key_t k) const { return key_states_[key_mapping_[k]].released_ != 0; }

				inline void map_key(key_t hardware_key, key_t virtual_key) { key_mapping_[virtual_key] = hardware_key; }

				inline void set_identity_mapping()
				{
					for(uint32_t i = 0; i < 256; i++) {
						key_mapping_[i] = i;
					}
				}

				void set_key_states(const key_state_report_t states[256])
				{
					for(uint32_t i = 0; i < 256; i++) {
						if(key_states_[i].on_ == states[i].on_ && states[i].transition_count_ == 0) {
							// Nothing happened with this key
							key_states_[i].pressed_ = 0;
							key_states_[i].released_ = 0;
						} else {
							// Something happened
							// The key may have changed its 'on' state since last frame.
							// It is also possible that the key has the same 'on' state as last frame, but there was one
							// (or more) intra-frame up-down events.
							key_states_[i].on_ = states[i].on_;
							if(states[i].on_) {
								if(states[i].transition_count_ == 1) {
									key_states_[i].pressed_ = 1;
									key_states_[i].released_ = 0;
								} else {
									key_states_[i].pressed_ = 1;
									key_states_[i].released_ = 1;
								}
							} else {
								if(states[i].transition_count_ == 1) {
									key_states_[i].released_ = 1;
									key_states_[i].pressed_ = 0;
								} else {
									key_states_[i].released_ = 1;
									key_states_[i].pressed_ = 1;
								}
							}
						}
					}
				}

			public:
				controller_t *controller_;
		
			private:
				uint8_t key_mapping_[256];
				key_state_t key_states_[256];
			};

		public:
			input_state_t();
			virtual ~input_state_t();
		
			// Hold a reference to the player object
			inline player_t *get_player(uint8_t index) { assert(index < max_players_); return &players_[index]; }

			// Don't hold a reference to the controller object - get it each time from the player
			// in case the controller associated with the player changes
			inline controller_t &get_controller(uint8_t index) { assert(index < max_players_); return controllers_[index]; }

			void connect_controller(uint8_t controller_index);
			void disconnect_controller(uint8_t controller_index);

		private:
			uint8_t controller_count_;
			controller_t controllers_[max_players_];
			player_t players_[max_players_];
		};

	} // namespace core

} // namespace maki
