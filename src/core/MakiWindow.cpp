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

			input_state_t::button_t sdl_button_to_maki_button[SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_MAX] = {
				input_state_t::button_a, // SDL_CONTROLLER_BUTTON_A,
				input_state_t::button_b, // SDL_CONTROLLER_BUTTON_B,
				input_state_t::button_x, // SDL_CONTROLLER_BUTTON_X,
				input_state_t::button_y, // SDL_CONTROLLER_BUTTON_Y,
				input_state_t::button_back, // SDL_CONTROLLER_BUTTON_BACK,
				input_state_t::button_invalid, // SDL_CONTROLLER_BUTTON_GUIDE,
				input_state_t::button_start, // SDL_CONTROLLER_BUTTON_START,
				input_state_t::button_left_thumb, // SDL_CONTROLLER_BUTTON_LEFTSTICK,
				input_state_t::button_right_thumb, // SDL_CONTROLLER_BUTTON_RIGHTSTICK,
				input_state_t::button_left_shoulder, // SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
				input_state_t::button_right_shoulder, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
				input_state_t::button_dpad_up, // SDL_CONTROLLER_BUTTON_DPAD_UP,
				input_state_t::button_dpad_down, // SDL_CONTROLLER_BUTTON_DPAD_DOWN,
				input_state_t::button_dpad_left, // SDL_CONTROLLER_BUTTON_DPAD_LEFT,
				input_state_t::button_dpad_right, // SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
			};

			input_state_t::button_t sdl_axis_to_maki_button[6] = {
				input_state_t::button_left_thumb_x,
				input_state_t::button_left_thumb_y,
				input_state_t::button_right_thumb_x,
				input_state_t::button_right_thumb_y,
				input_state_t::button_left_trigger,
				input_state_t::button_right_trigger,
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

			inline input_state_t::key_t sdl_key_to_maki_key(SDL_Keycode k) {
				switch (k) {
					/*case SDLK_: return input_state_t::key_l_button;
					case SDLK_: return input_state_t::key_r_button;
					case SDLK_: return input_state_t::key_cancel;
					case SDLK_: return input_state_t::key_m_button;
					case SDLK_: return input_state_t::key_x_button1;
					case SDLK_: return input_state_t::key_x_button2;
					case SDLK_: return input_state_t::key_back;
					*/
				case SDLK_TAB: return input_state_t::key_tab;
				case SDLK_CLEAR: return input_state_t::key_clear;
				case SDLK_RETURN: return input_state_t::key_return;
					//case SDLK_: return input_state_t::key_shift;
					//case SDLK_: return input_state_t::key_control;
				case SDLK_MENU: return input_state_t::key_menu;
				case SDLK_PAUSE: return input_state_t::key_pause;
				case SDLK_CAPSLOCK: return input_state_t::key_capital;
				case SDLK_ESCAPE: return input_state_t::key_escape;
					//case SDLK_: return input_state_t::key_convert;
					//case SDLK_: return input_state_t::key_nonconvert;
					//case SDLK_: return input_state_t::key_accept;
					//case SDLK_: return input_state_t::key_modechange;
				case SDLK_SPACE: return input_state_t::key_space;
				case SDLK_PAGEUP: return input_state_t::key_prior;
				case SDLK_PAGEDOWN: return input_state_t::key_next;
				case SDLK_END: return input_state_t::key_end;
				case SDLK_HOME: return input_state_t::key_home;
				case SDLK_LEFT: return input_state_t::key_left;
				case SDLK_UP: return input_state_t::key_up;
				case SDLK_RIGHT: return input_state_t::key_right;
				case SDLK_DOWN: return input_state_t::key_down;
				case SDLK_SELECT: return input_state_t::key_select;
					//case SDLK_: return input_state_t::key_print;
				case SDLK_EXECUTE: return input_state_t::key_execute;
				case SDLK_PRINTSCREEN: return input_state_t::key_snapshot;
				case SDLK_INSERT: return input_state_t::key_insert;
				case SDLK_DELETE: return input_state_t::key_delete;
				case SDLK_HELP: return input_state_t::key_help;
				case SDLK_0: return input_state_t::key_0;
				case SDLK_1: return input_state_t::key_1;
				case SDLK_2: return input_state_t::key_2;
				case SDLK_3: return input_state_t::key_3;
				case SDLK_4: return input_state_t::key_4;
				case SDLK_5: return input_state_t::key_5;
				case SDLK_6: return input_state_t::key_6;
				case SDLK_7: return input_state_t::key_7;
				case SDLK_8: return input_state_t::key_8;
				case SDLK_9: return input_state_t::key_9;
				case SDLK_a: return input_state_t::key_a;
				case SDLK_b: return input_state_t::key_b;
				case SDLK_c: return input_state_t::key_c;
				case SDLK_d: return input_state_t::key_d;
				case SDLK_e: return input_state_t::key_e;
				case SDLK_f: return input_state_t::key_f;
				case SDLK_g: return input_state_t::key_g;
				case SDLK_h: return input_state_t::key_h;
				case SDLK_i: return input_state_t::key_i;
				case SDLK_j: return input_state_t::key_j;
				case SDLK_k: return input_state_t::key_k;
				case SDLK_l: return input_state_t::key_l;
				case SDLK_m: return input_state_t::key_m;
				case SDLK_n: return input_state_t::key_n;
				case SDLK_o: return input_state_t::key_o;
				case SDLK_p: return input_state_t::key_p;
				case SDLK_q: return input_state_t::key_q;
				case SDLK_r: return input_state_t::key_r;
				case SDLK_s: return input_state_t::key_s;
				case SDLK_t: return input_state_t::key_t;
				case SDLK_u: return input_state_t::key_u;
				case SDLK_v: return input_state_t::key_v;
				case SDLK_w: return input_state_t::key_w;
				case SDLK_x: return input_state_t::key_x;
				case SDLK_y: return input_state_t::key_y;
				case SDLK_z: return input_state_t::key_z;
					//case SDLK_: return input_state_t::key_l_win;
					//case SDLK_: return input_state_t::key_r_win;
				case SDLK_APPLICATION: return input_state_t::key_apps;
				case SDLK_SLEEP: return input_state_t::key_sleep;
				case SDLK_KP_0: return input_state_t::key_numpad0;
				case SDLK_KP_1: return input_state_t::key_numpad1;
				case SDLK_KP_2: return input_state_t::key_numpad2;
				case SDLK_KP_3: return input_state_t::key_numpad3;
				case SDLK_KP_4: return input_state_t::key_numpad4;
				case SDLK_KP_5: return input_state_t::key_numpad5;
				case SDLK_KP_6: return input_state_t::key_numpad6;
				case SDLK_KP_7: return input_state_t::key_numpad7;
				case SDLK_KP_8: return input_state_t::key_numpad8;
				case SDLK_KP_9: return input_state_t::key_numpad9;
				case SDLK_KP_MULTIPLY: return input_state_t::key_multiply;
				case SDLK_KP_PLUS: return input_state_t::key_add;
				case SDLK_KP_VERTICALBAR: return input_state_t::key_separator;
				case SDLK_KP_MINUS: return input_state_t::key_subtract;
				case SDLK_KP_DECIMAL: return input_state_t::key_decimal;
				case SDLK_KP_DIVIDE: return input_state_t::key_divide;
				case SDLK_F1: return input_state_t::key_f1;
				case SDLK_F2: return input_state_t::key_f2;
				case SDLK_F3: return input_state_t::key_f3;
				case SDLK_F4: return input_state_t::key_f4;
				case SDLK_F5: return input_state_t::key_f5;
				case SDLK_F6: return input_state_t::key_f6;
				case SDLK_F7: return input_state_t::key_f7;
				case SDLK_F8: return input_state_t::key_f8;
				case SDLK_F9: return input_state_t::key_f9;
				case SDLK_F10: return input_state_t::key_f10;
				case SDLK_F11: return input_state_t::key_f11;
				case SDLK_F12: return input_state_t::key_f12;
				case SDLK_F13: return input_state_t::key_f13;
				case SDLK_F14: return input_state_t::key_f14;
				case SDLK_F15: return input_state_t::key_f15;
				case SDLK_F16: return input_state_t::key_f16;
				case SDLK_F17: return input_state_t::key_f17;
				case SDLK_F18: return input_state_t::key_f18;
				case SDLK_F19: return input_state_t::key_f19;
				case SDLK_F20: return input_state_t::key_f20;
				case SDLK_F21: return input_state_t::key_f21;
				case SDLK_F22: return input_state_t::key_f22;
				case SDLK_F23: return input_state_t::key_f23;
				case SDLK_F24: return input_state_t::key_f24;
				case SDLK_NUMLOCKCLEAR: return input_state_t::key_numlock;
				case SDLK_SCROLLLOCK: return input_state_t::key_scroll;
				case SDLK_LSHIFT: return input_state_t::key_l_shift;
				case SDLK_RSHIFT: return input_state_t::key_r_shift;
				case SDLK_LCTRL: return input_state_t::key_l_control;
				case SDLK_RCTRL: return input_state_t::key_r_control;
					//case SDLK_: return input_state_t::key_l_menu;
					//case SDLK_: return input_state_t::key_r_menu;
				case SDLK_AC_BACK: return input_state_t::key_browser_back;
				case SDLK_AC_FORWARD: return input_state_t::key_browser_forward;
				case SDLK_AC_REFRESH: return input_state_t::key_browser_refresh;
				case SDLK_AC_STOP: return input_state_t::key_browser_stop;
				case SDLK_AC_SEARCH: return input_state_t::key_browser_search;
				case SDLK_AC_BOOKMARKS: return input_state_t::key_browser_favorites;
				case SDLK_AC_HOME: return input_state_t::key_browser_home;
				case SDLK_MUTE: return input_state_t::key_volume_mute;
				case SDLK_VOLUMEDOWN: return input_state_t::key_volume_down;
				case SDLK_VOLUMEUP: return input_state_t::key_volume_up;
				case SDLK_AUDIONEXT: return input_state_t::key_media_next_track;
				case SDLK_AUDIOPREV: return input_state_t::key_media_prev_track;
				case SDLK_AUDIOSTOP: return input_state_t::key_media_stop;
				case SDLK_AUDIOPLAY: return input_state_t::key_media_play_pause;
				}
				return input_state_t::key_unknown;
			}

		} // namespace


		window_t::window_t(render_core_t::type_t render_core_type, const config_t *config) {
			memset(controller_instance_ids_, 0xff, sizeof(controller_instance_ids_));

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

			width = config->get_int("engine.width", 800);
			height = config->get_int("engine.height", 600);
			std::string title = config->get_string("engine.title", "");

			window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,  height, flags);
			const char *err = SDL_GetError();
			if(window == nullptr || strlen(err) != 0) {
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
			if(window != nullptr)
				SDL_DestroyWindow(window);
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
					state->get_controller(i).set_values(c.values, sizeof(c.values));
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
								input_state_t::button_t btn = sdl_button_to_maki_button[e.cbutton.button];
								if(btn != input_state_t::button_invalid)
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
							width = e.window.data1;
							height = e.window.data2;
							if(width <= 0 || height <= 0) {
								width = 1;
								height = 1;
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
