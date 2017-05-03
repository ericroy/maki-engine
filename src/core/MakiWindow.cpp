#include "core/MakiWindow.h"
#include "core/MakiConfig.h"
#include "core/MakiEngine.h"
#include "core/MakiConsole.h"
#include <sstream>

//#if defined(_WIN32) || defined(_WIN64)
//#define NOMINMAX 1
//#include <Windows.h>
//#include <Xinput.h>
//#endif


namespace maki {
	namespace core {

		namespace {

			inline float signed_analog_input_to_float(int16_t v, int16_t dz) {
				if (v > dz) {
					float f = (v - dz) / (float)(32767 - dz);
					return f < -1.0f ? -1.0f : f;
				} else if (v < -dz) {
					float f = (v + dz) / (float)(32767 - dz);
					return f < -1.0f ? -1.0f : f;
				} else {
					return 0.0f;
				}
			}

			inline float analog_input_to_float(uint8_t v, uint8_t dz) {
				if (v > dz)
					return (v - dz) / (float)(255 - dz);
				else
					return 0.0f;
			}

			button_id_t sdl_button_to_maki_button[SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_MAX] = {
				button_a, // SDL_CONTROLLER_BUTTON_A,
				button_b, // SDL_CONTROLLER_BUTTON_B,
				button_x, // SDL_CONTROLLER_BUTTON_X,
				button_y, // SDL_CONTROLLER_BUTTON_Y,
				button_back, // SDL_CONTROLLER_BUTTON_BACK,
				button_invalid, // SDL_CONTROLLER_BUTTON_GUIDE,
				button_start, // SDL_CONTROLLER_BUTTON_START,
				button_left_thumb, // SDL_CONTROLLER_BUTTON_LEFTSTICK,
				button_right_thumb, // SDL_CONTROLLER_BUTTON_RIGHTSTICK,
				button_left_shoulder, // SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
				button_right_shoulder, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
				button_dpad_up, // SDL_CONTROLLER_BUTTON_DPAD_UP,
				button_dpad_down, // SDL_CONTROLLER_BUTTON_DPAD_DOWN,
				button_dpad_left, // SDL_CONTROLLER_BUTTON_DPAD_LEFT,
				button_dpad_right, // SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
			};

			button_id_t sdl_axis_to_maki_button[6] = {
				button_left_thumb_x,
				button_left_thumb_y,
				button_right_thumb_x,
				button_right_thumb_y,
				button_left_trigger,
				button_right_trigger,
			};

			static const uint16_t gamepad_left_thumb_deadzone = 7849;
			static const uint16_t gamepad_right_thumb_deadzone = 8689;

			int16_t maki_button_deadzones[6] = {
				gamepad_left_thumb_deadzone,
				gamepad_left_thumb_deadzone,
				gamepad_right_thumb_deadzone,
				gamepad_right_thumb_deadzone,
				0,
				0,
			};

			inline key_id_t sdl_key_to_maki_key(SDL_Keycode k) {
				switch (k) {
					/*case SDLK_: return key_l_button;
					case SDLK_: return key_r_button;
					case SDLK_: return key_cancel;
					case SDLK_: return key_m_button;
					case SDLK_: return key_x_button1;
					case SDLK_: return key_x_button2;
					case SDLK_: return key_back;
					*/
				case SDLK_TAB: return key_tab;
				case SDLK_CLEAR: return key_clear;
				case SDLK_RETURN: return key_return;
					//case SDLK_: return key_shift;
					//case SDLK_: return key_control;
				case SDLK_MENU: return key_menu;
				case SDLK_PAUSE: return key_pause;
				case SDLK_CAPSLOCK: return key_capital;
				case SDLK_ESCAPE: return key_escape;
					//case SDLK_: return key_convert;
					//case SDLK_: return key_nonconvert;
					//case SDLK_: return key_accept;
					//case SDLK_: return key_modechange;
				case SDLK_SPACE: return key_space;
				case SDLK_PAGEUP: return key_prior;
				case SDLK_PAGEDOWN: return key_next;
				case SDLK_END: return key_end;
				case SDLK_HOME: return key_home;
				case SDLK_LEFT: return key_left;
				case SDLK_UP: return key_up;
				case SDLK_RIGHT: return key_right;
				case SDLK_DOWN: return key_down;
				case SDLK_SELECT: return key_select;
					//case SDLK_: return key_print;
				case SDLK_EXECUTE: return key_execute;
				case SDLK_PRINTSCREEN: return key_snapshot;
				case SDLK_INSERT: return key_insert;
				case SDLK_DELETE: return key_delete;
				case SDLK_HELP: return key_help;
				case SDLK_0: return key_0;
				case SDLK_1: return key_1;
				case SDLK_2: return key_2;
				case SDLK_3: return key_3;
				case SDLK_4: return key_4;
				case SDLK_5: return key_5;
				case SDLK_6: return key_6;
				case SDLK_7: return key_7;
				case SDLK_8: return key_8;
				case SDLK_9: return key_9;
				case SDLK_a: return key_a;
				case SDLK_b: return key_b;
				case SDLK_c: return key_c;
				case SDLK_d: return key_d;
				case SDLK_e: return key_e;
				case SDLK_f: return key_f;
				case SDLK_g: return key_g;
				case SDLK_h: return key_h;
				case SDLK_i: return key_i;
				case SDLK_j: return key_j;
				case SDLK_k: return key_k;
				case SDLK_l: return key_l;
				case SDLK_m: return key_m;
				case SDLK_n: return key_n;
				case SDLK_o: return key_o;
				case SDLK_p: return key_p;
				case SDLK_q: return key_q;
				case SDLK_r: return key_r;
				case SDLK_s: return key_s;
				case SDLK_t: return key_t;
				case SDLK_u: return key_u;
				case SDLK_v: return key_v;
				case SDLK_w: return key_w;
				case SDLK_x: return key_x;
				case SDLK_y: return key_y;
				case SDLK_z: return key_z;
					//case SDLK_: return key_l_win;
					//case SDLK_: return key_r_win;
				case SDLK_APPLICATION: return key_apps;
				case SDLK_SLEEP: return key_sleep;
				case SDLK_KP_0: return key_numpad0;
				case SDLK_KP_1: return key_numpad1;
				case SDLK_KP_2: return key_numpad2;
				case SDLK_KP_3: return key_numpad3;
				case SDLK_KP_4: return key_numpad4;
				case SDLK_KP_5: return key_numpad5;
				case SDLK_KP_6: return key_numpad6;
				case SDLK_KP_7: return key_numpad7;
				case SDLK_KP_8: return key_numpad8;
				case SDLK_KP_9: return key_numpad9;
				case SDLK_KP_MULTIPLY: return key_multiply;
				case SDLK_KP_PLUS: return key_add;
				case SDLK_KP_VERTICALBAR: return key_separator;
				case SDLK_KP_MINUS: return key_subtract;
				case SDLK_KP_DECIMAL: return key_decimal;
				case SDLK_KP_DIVIDE: return key_divide;
				case SDLK_F1: return key_f1;
				case SDLK_F2: return key_f2;
				case SDLK_F3: return key_f3;
				case SDLK_F4: return key_f4;
				case SDLK_F5: return key_f5;
				case SDLK_F6: return key_f6;
				case SDLK_F7: return key_f7;
				case SDLK_F8: return key_f8;
				case SDLK_F9: return key_f9;
				case SDLK_F10: return key_f10;
				case SDLK_F11: return key_f11;
				case SDLK_F12: return key_f12;
				case SDLK_F13: return key_f13;
				case SDLK_F14: return key_f14;
				case SDLK_F15: return key_f15;
				case SDLK_F16: return key_f16;
				case SDLK_F17: return key_f17;
				case SDLK_F18: return key_f18;
				case SDLK_F19: return key_f19;
				case SDLK_F20: return key_f20;
				case SDLK_F21: return key_f21;
				case SDLK_F22: return key_f22;
				case SDLK_F23: return key_f23;
				case SDLK_F24: return key_f24;
				case SDLK_NUMLOCKCLEAR: return key_numlock;
				case SDLK_SCROLLLOCK: return key_scroll;
				case SDLK_LSHIFT: return key_l_shift;
				case SDLK_RSHIFT: return key_r_shift;
				case SDLK_LCTRL: return key_l_control;
				case SDLK_RCTRL: return key_r_control;
					//case SDLK_: return key_l_menu;
					//case SDLK_: return key_r_menu;
				case SDLK_AC_BACK: return key_browser_back;
				case SDLK_AC_FORWARD: return key_browser_forward;
				case SDLK_AC_REFRESH: return key_browser_refresh;
				case SDLK_AC_STOP: return key_browser_stop;
				case SDLK_AC_SEARCH: return key_browser_search;
				case SDLK_AC_BOOKMARKS: return key_browser_favorites;
				case SDLK_AC_HOME: return key_browser_home;
				case SDLK_MUTE: return key_volume_mute;
				case SDLK_VOLUMEDOWN: return key_volume_down;
				case SDLK_VOLUMEUP: return key_volume_up;
				case SDLK_AUDIONEXT: return key_media_next_track;
				case SDLK_AUDIOPREV: return key_media_prev_track;
				case SDLK_AUDIOSTOP: return key_media_stop;
				case SDLK_AUDIOPLAY: return key_media_play_pause;
				}
				return key_unknown;
			}

		} // namespace


		window_t::window_t(render_core_t::type_t render_core_type, const config_t *config) {
			SDL_SetHint(SDL_HINT_RENDER_VSYNC, 0);
			SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

			// load up any additional controller mappings that are specified in the config
			std::stringstream mappings;
			uint32_t mapping_count = 0;
			for(uint32_t i = 0; ; i++) {
				char controller_mapping_key[64];
				sprintf(controller_mapping_key, "engine.controller_support_%d", i);
				const char *value = config->get_string(controller_mapping_key, nullptr);
				if(value == nullptr) {
					break;
				}
				if(mapping_count > 0)
					mappings << std::endl;
				mapping_count++;
				mappings << value;
			}
			SDL_SetHint(SDL_HINT_GAMECONTROLLERCONFIG, mappings.str().c_str());

			if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
				console_t::error("Failed to init SDL video subsystem: %s", SDL_GetError());
				SDL_ClearError();
			}			
			
			uint32_t flags = SDL_WINDOW_INPUT_FOCUS|SDL_WINDOW_SHOWN;

			if(render_core_type == render_core_t::type_ogl) {
				flags |= SDL_WINDOW_OPENGL;
			}

			fullscreen_ = config->get_bool("engine.full_screen", false);
			if(fullscreen_) {
				flags |= SDL_WINDOW_FULLSCREEN;
			}

			width_ = config->get_int("engine.width", 800);
			height_ = config->get_int("engine.height", 600);
			std::string title = config->get_string("engine.title", "");

			window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width_, height_, flags);
			const char *err = SDL_GetError();
			if(window_ == nullptr || strlen(err) != 0) {
				console_t::error("Failed to create SDL window: %s", SDL_GetError());
				SDL_ClearError();
			}

			// Discover already-connected game controllers_
			int32_t joystick_count = std::min(SDL_NumJoysticks(), input_state_t::max_players);
			for(int32_t i = 0; i < joystick_count; i++) {
				if(SDL_IsGameController(i)) {
					connect_game_controller(i);
				} else {
					SDL_Joystick *joy = SDL_JoystickOpen(i);
					if(joy != nullptr) {
						SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
						char buffer[33];
						SDL_JoystickGetGUIDString(guid, buffer, 33);
						console_t::info("Found joystick that was not a compatible game controller (%s: %s)", buffer, SDL_JoystickName(joy));
						SDL_JoystickClose(joy);
					}
				}
			}
		}

		window_t::~window_t() {
			if(window_ != nullptr)
				SDL_DestroyWindow(window_);
			SDL_Quit();
		}

		void window_t::poll_input(input_state_t *state) {
			for(uint32_t i = 0; i < input_state_t::max_players; i++) {
				state->get_player(i)->set_key_states(key_states_);

				auto &c = controllers_[i];
				if(c.handle != nullptr) {
					if(!state->get_controller(i).connected()) {
						// Just connected
						state->connect_controller(i);
					}
					state->get_controller(i).set_values(c.values);
				} else if(state->get_controller(i).connected()) {
					// Just disconnected
					state->disconnect_controller(i);
				}
			}
		}

		void window_t::connect_game_controller(int32_t joystick_index) {
			uint32_t index = input_state_t::max_players;
			for(uint32_t i = 0; i < input_state_t::max_players; i++) {
				if (controllers_[i].handle == nullptr) {
					index = i;
					break;
				}
			}
			if (index == input_state_t::max_players)
				return;

			auto &c = controllers_[index];
			c.handle = SDL_GameControllerOpen(joystick_index);
			if(c.handle == nullptr) {
				console_t::error("Failed to open SDL game controller (%d): %s", joystick_index, SDL_GetError());
				SDL_ClearError();
			} else {
				c.instance_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(c.handle));
				console_t::info("Controller connected (%d, \"%s\")", joystick_index, SDL_GameControllerName(c.handle));
			}
		}

		int window_t::pump(engine_t *engine) {
			SDL_Event e;
			while(true) {
				while(SDL_PollEvent(&e) != 0) {
					switch(e.type) {
					case SDL_CONTROLLERAXISMOTION:
						{
							int32_t i = get_controller_index(e.caxis.which);
							if(i >= 0 && e.caxis.axis < sizeof(sdl_axis_to_maki_button) / sizeof(sdl_axis_to_maki_button[0]))
								controllers_[i].values[sdl_axis_to_maki_button[e.caxis.axis]] = signed_analog_input_to_float(e.caxis.value, maki_button_deadzones[e.caxis.axis]);
						}
						break;
					case SDL_CONTROLLERBUTTONDOWN:
					case SDL_CONTROLLERBUTTONUP:
						{
							int32_t i = get_controller_index(e.cbutton.which);
							if(i >= 0) {
								button_id_t btn = sdl_button_to_maki_button[e.cbutton.button];
								if(btn != button_invalid)
									controllers_[i].values[btn] = e.cbutton.state * 1.0f;
							}
						}
						break;
					case SDL_CONTROLLERDEVICEADDED:
						connect_game_controller(e.cdevice.which);
						break;
					case SDL_CONTROLLERDEVICEREMOVED:
						{
							int32_t i = get_controller_index(e.cdevice.which);
							if(i >= 0) {
								auto &c = controllers_[i];
								SDL_GameControllerClose(c.handle);
								c.handle = nullptr;
								c.instance_id = -1;
								memset(c.values, 0, sizeof(c.values));
								console_t::info("Controller disconnected (%d)", i);
							}
						}
						break;

					case SDL_KEYDOWN:
						// Support alt-f4 killing of the window
						if(e.key.keysym.sym == SDLK_F4 && (e.key.keysym.mod & KMOD_ALT) != 0) {
							goto quit;
						}
						key_states_[sdl_key_to_maki_key(e.key.keysym.sym)].set(true);
						break;
					case SDL_KEYUP:
						key_states_[sdl_key_to_maki_key(e.key.keysym.sym)].set(false);
						break;
					
					case SDL_WINDOWEVENT:
						if(e.window.event == SDL_WINDOWEVENT_RESIZED) {
							width_ = e.window.data1;
							height_ = e.window.data2;
							if(width_ <= 0 || height_ <= 0) {
								width_ = 1;
								height_ = 1;
							}
						}
						break;
					
					case SDL_QUIT:
						goto quit;
					}
				}
				engine->tick();
			}

		quit:
			return 0;
		}

	} // namespace core
} // namespace maki
