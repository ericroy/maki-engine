#include "core/core_stdafx.h"
#include "core/MakiWindow.h"
#include "core/MakiConfig.h"
#include "core/MakiEngine.h"
#include "core/MakiInputState.h"
#include <sstream>


namespace maki
{
	namespace core
	{

		inline float signed_analog_input_to_float(int16 v, int16 dz) {
			if(v > dz) {
				float f = (v-dz) / (float)(32767-dz);
				return f < -1.0f ? -1.0f : f;
			} else if(v < -dz) {
				float f = (v+dz) / (float)(32767-dz);
				return f < -1.0f ? -1.0f : f;
			} else {
				return 0.0f;
			}
		}

		inline float analog_input_to_float(uint8 v, uint8 dz) {
			if(v > dz) {
				return (v-dz) / (float)(255-dz);
			} else {
				return 0.0f;
			}
		}

		input_state_t::button_t SDLButtonToMakiButton[SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_MAX] = {
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

		input_state_t::button_t sdl_axis_to_maki_button_[6] = {
			input_state_t::button_left_thumb_x,
			input_state_t::button_left_thumb_y,
			input_state_t::button_right_thumb_x,
			input_state_t::button_right_thumb_y,
			input_state_t::button_left_trigger,
			input_state_t::button_right_trigger,
		};

		static const uint16 gamepad_left_thumb_deadzone_ = 7849;
		static const uint16 gamepad_right_thumb_deadzone_ = 8689;

		int16 maki_button_deadzones_[6] = {
			gamepad_left_thumb_deadzone_,
			gamepad_left_thumb_deadzone_,
			gamepad_right_thumb_deadzone_,
			gamepad_right_thumb_deadzone_,
			0,
			0,
		};

		inline input_state_t::key_t sdl_key_to_maki_key(SDL_Keycode k)
		{
			switch(k) {			
			/*case SDLK_: return input_state_t::key_l_button_;
			case SDLK_: return input_state_t::key_r_button_;
			case SDLK_: return input_state_t::key_cancel_;
			case SDLK_: return input_state_t::key_m_button_;
			case SDLK_: return input_state_t::key_x_button1_;
			case SDLK_: return input_state_t::key_x_button2_;
			case SDLK_: return input_state_t::key_back_;
			*/
			case SDLK_TAB: return input_state_t::key_tab_;
			case SDLK_CLEAR: return input_state_t::key_clear_;
			case SDLK_RETURN: return input_state_t::key_return_;
			//case SDLK_: return input_state_t::key_shift_;
			//case SDLK_: return input_state_t::key_control_;
			case SDLK_MENU: return input_state_t::key_menu_;
			case SDLK_PAUSE: return input_state_t::key_pause_;
			case SDLK_CAPSLOCK: return input_state_t::key_capital_;
			case SDLK_ESCAPE: return input_state_t::key_escape_;
			//case SDLK_: return input_state_t::key_convert_;
			//case SDLK_: return input_state_t::key_nonconvert_;
			//case SDLK_: return input_state_t::key_accept_;
			//case SDLK_: return input_state_t::key_modechange_;
			case SDLK_SPACE: return input_state_t::key_space_;
			case SDLK_PAGEUP: return input_state_t::key_prior_;
			case SDLK_PAGEDOWN: return input_state_t::key_next_;
			case SDLK_END: return input_state_t::key_end_;
			case SDLK_HOME: return input_state_t::key_home_;
			case SDLK_LEFT: return input_state_t::key_left_;
			case SDLK_UP: return input_state_t::key_up_;
			case SDLK_RIGHT: return input_state_t::key_right_;
			case SDLK_DOWN: return input_state_t::key_down_;
			case SDLK_SELECT: return input_state_t::key_select_;
			//case SDLK_: return input_state_t::key_print_;
			case SDLK_EXECUTE: return input_state_t::key_execute_;
			case SDLK_PRINTSCREEN: return input_state_t::key_snapshot_;
			case SDLK_INSERT: return input_state_t::key_insert_;
			case SDLK_DELETE: return input_state_t::key_delete_;
			case SDLK_HELP: return input_state_t::key_help_;
			case SDLK_0: return input_state_t::key_0_;
			case SDLK_1: return input_state_t::key_1_;
			case SDLK_2: return input_state_t::key_2_;
			case SDLK_3: return input_state_t::key_3_;
			case SDLK_4: return input_state_t::key_4_;
			case SDLK_5: return input_state_t::key_5_;
			case SDLK_6: return input_state_t::key_6_;
			case SDLK_7: return input_state_t::key_7_;
			case SDLK_8: return input_state_t::key_8_;
			case SDLK_9: return input_state_t::key_9_;
			case SDLK_a: return input_state_t::key_a_;
			case SDLK_b: return input_state_t::key_b_;
			case SDLK_c: return input_state_t::key_c_;
			case SDLK_d: return input_state_t::key_d_;
			case SDLK_e: return input_state_t::key_e_;
			case SDLK_f: return input_state_t::key_f_;
			case SDLK_g: return input_state_t::key_g_;
			case SDLK_h: return input_state_t::key_h_;
			case SDLK_i: return input_state_t::key_i_;
			case SDLK_j: return input_state_t::key_j_;
			case SDLK_k: return input_state_t::key_k_;
			case SDLK_l: return input_state_t::key_l_;
			case SDLK_m: return input_state_t::key_m_;
			case SDLK_n: return input_state_t::key_n_;
			case SDLK_o: return input_state_t::key_o_;
			case SDLK_p: return input_state_t::key_p_;
			case SDLK_q: return input_state_t::key_q_;
			case SDLK_r: return input_state_t::key_r_;
			case SDLK_s: return input_state_t::key_s_;
			case SDLK_t: return input_state_t::key_t_;
			case SDLK_u: return input_state_t::key_u_;
			case SDLK_v: return input_state_t::key_v_;
			case SDLK_w: return input_state_t::key_w_;
			case SDLK_x: return input_state_t::key_x_;
			case SDLK_y: return input_state_t::key_y_;
			case SDLK_z: return input_state_t::key_z_;
			//case SDLK_: return input_state_t::key_l_win_;
			//case SDLK_: return input_state_t::key_r_win_;
			case SDLK_APPLICATION: return input_state_t::key_apps_;
			case SDLK_SLEEP: return input_state_t::key_sleep_;
			case SDLK_KP_0: return input_state_t::key_numpad0_;
			case SDLK_KP_1: return input_state_t::key_numpad1_;
			case SDLK_KP_2: return input_state_t::key_numpad2_;
			case SDLK_KP_3: return input_state_t::key_numpad3_;
			case SDLK_KP_4: return input_state_t::key_numpad4_;
			case SDLK_KP_5: return input_state_t::key_numpad5_;
			case SDLK_KP_6: return input_state_t::key_numpad6_;
			case SDLK_KP_7: return input_state_t::key_numpad7_;
			case SDLK_KP_8: return input_state_t::key_numpad8_;
			case SDLK_KP_9: return input_state_t::key_numpad9_;
			case SDLK_KP_MULTIPLY: return input_state_t::key_multiply_;
			case SDLK_KP_PLUS: return input_state_t::key_add_;
			case SDLK_KP_VERTICALBAR: return input_state_t::key_separator_;
			case SDLK_KP_MINUS: return input_state_t::key_subtract_;
			case SDLK_KP_DECIMAL: return input_state_t::key_decimal_;
			case SDLK_KP_DIVIDE: return input_state_t::key_divide_;
			case SDLK_F1: return input_state_t::key_f1_;
			case SDLK_F2: return input_state_t::key_f2_;
			case SDLK_F3: return input_state_t::key_f3_;
			case SDLK_F4: return input_state_t::key_f4_;
			case SDLK_F5: return input_state_t::key_f5_;
			case SDLK_F6: return input_state_t::key_f6_;
			case SDLK_F7: return input_state_t::key_f7_;
			case SDLK_F8: return input_state_t::key_f8_;
			case SDLK_F9: return input_state_t::key_f9_;
			case SDLK_F10: return input_state_t::key_f10_;
			case SDLK_F11: return input_state_t::key_f11_;
			case SDLK_F12: return input_state_t::key_f12_;
			case SDLK_F13: return input_state_t::key_f13_;
			case SDLK_F14: return input_state_t::key_f14_;
			case SDLK_F15: return input_state_t::key_f15_;
			case SDLK_F16: return input_state_t::key_f16_;
			case SDLK_F17: return input_state_t::key_f17_;
			case SDLK_F18: return input_state_t::key_f18_;
			case SDLK_F19: return input_state_t::key_f19_;
			case SDLK_F20: return input_state_t::key_f20_;
			case SDLK_F21: return input_state_t::key_f21_;
			case SDLK_F22: return input_state_t::key_f22_;
			case SDLK_F23: return input_state_t::key_f23_;
			case SDLK_F24: return input_state_t::key_f24_;
			case SDLK_NUMLOCKCLEAR: return input_state_t::key_numlock_;
			case SDLK_SCROLLLOCK: return input_state_t::key_scroll_;
			case SDLK_LSHIFT: return input_state_t::key_l_shift_;
			case SDLK_RSHIFT: return input_state_t::key_r_shift_;
			case SDLK_LCTRL: return input_state_t::key_l_control_;
			case SDLK_RCTRL: return input_state_t::key_r_control_;
			//case SDLK_: return input_state_t::key_l_menu_;
			//case SDLK_: return input_state_t::key_r_menu_;
			case SDLK_AC_BACK: return input_state_t::key_browser_back_;
			case SDLK_AC_FORWARD: return input_state_t::key_browser_forward_;
			case SDLK_AC_REFRESH: return input_state_t::key_browser_refresh_;
			case SDLK_AC_STOP: return input_state_t::key_browser_stop_;
			case SDLK_AC_SEARCH: return input_state_t::key_browser_search_;
			case SDLK_AC_BOOKMARKS: return input_state_t::key_browser_favorites_;
			case SDLK_AC_HOME: return input_state_t::key_browser_home_;
			case SDLK_MUTE: return input_state_t::key_volume_mute_;
			case SDLK_VOLUMEDOWN: return input_state_t::key_volume_down_;
			case SDLK_VOLUMEUP: return input_state_t::key_volume_up_;
			case SDLK_AUDIONEXT: return input_state_t::key_media_next_track_;
			case SDLK_AUDIOPREV: return input_state_t::key_media_prev_track_;
			case SDLK_AUDIOSTOP: return input_state_t::key_media_stop_;
			case SDLK_AUDIOPLAY: return input_state_t::key_media_play_pause_;
			}
			return input_state_t::key_unknown_;
		}
		


		window_t::window_t(render_core_t::type_t render_core_type, const config_t *config)
			: window_(nullptr),
			width_(0),
			height_(0),
			fullscreen_(false)
		{
			memset(key_states_, 0, sizeof(key_states_));
			memset(controller_handles_, 0, sizeof(controller_handles_));
			memset(controllers_, 0, sizeof(controllers_));
			memset(controller_instance_ids_, 0xff, sizeof(controller_instance_ids_));

			SDL_SetHint(SDL_HINT_RENDER_VSYNC, 0);
			SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

			// load up any additional controller mappings that are specified in the config
			std::stringstream mappings;
			char controller_mapping_key[64];
			for(uint32 i = 0; ; i++) {
				sprintf(controller_mapping_key, "engine.controller_support_%d", i);
				const char *value = config->get_string(controller_mapping_key, nullptr);
				if(value == nullptr) {
					break;
				}
				mappings << value << std::endl;
			}
			SDL_SetHint(SDL_HINT_GAMECONTROLLERCONFIG, mappings.str().c_str());

			if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
				console_t::error("Failed to init SDL video subsystem: %s", SDL_GetError());
				SDL_ClearError();
			}			
			
			uint32 flags = SDL_WINDOW_INPUT_FOCUS|SDL_WINDOW_SHOWN;

			if(render_core_type == render_core_t::type_ogl_) {
				flags |= SDL_WINDOW_OPENGL;
			}

			fullscreen = config->get_bool("engine.full_screen", false);
			if(fullscreen) {
				flags |= SDL_WINDOW_FULLSCREEN;
			}

			width = config->get_int("engine.width_", 800);
			height = config->get_int("engine.height_", 600);
			std::string title = config->get_string("engine.title", "");

			window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,  height, flags);
			const char *err = SDL_GetError();
			if(window_ == nullptr || strlen(err) != 0) {
				console_t::error("Failed to create SDL window_: %s", SDL_GetError());
				SDL_ClearError();
			}

			// Discover already-connected game controllers_
			int32 joystick_count = std::min(SDL_NumJoysticks(), input_state_t::max_players_);
			for(int32 i = 0; i < joystick_count; i++) {
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

		window_t::~window_t()
		{
			if(window_ != nullptr) {
				SDL_DestroyWindow(window_);
			}
			SDL_Quit();
		}

		void window_t::poll_input(input_state_t *state)
		{
			for(uint32 i = 0; i < input_state_t::max_players_; i++) {
				state->GetPlayer(i)->set_key_states(key_states_);

				if(controller_handles_[i] != nullptr) {
					if(!state->get_controller(i).connected_) {
						// Just connected
						state->connect_controller(i);
					}
					memcpy(state->get_controller(i).values_, controllers_[i].values_, sizeof(controllers_[i].values_));
				} else if(state->get_controller(i).connected_) {
					// Just disconnected
					state->DisconnectController(i);
				}
			}
		}

		void window_t::connect_game_controller(int32 i)
		{
			controller_handles_[i] = SDL_GameControllerOpen(i);
			if(controller_handles_[i] == nullptr) {
				console_t::error("Failed to open SDL game controller (%d): %s", i, SDL_GetError());
				SDL_ClearError();
			} else {
				controller_instance_ids_[i] = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller_handles_[i]));
				console_t::info("controller_t connected (%d, \"%s\")", i, SDL_GameControllerName(controller_handles_[i]));
			}
		}

		int window_t::pump(engine_t *engine)
		{
			SDL_Event e;
			while(true) {
				while(SDL_PollEvent(&e) != 0) {
					switch(e.type_) {
					case SDL_CONTROLLERAXISMOTION:
						{
							int32 i = get_controller_index(e.caxis.which);
							if(i >= 0 && e.caxis.axis < sizeof(sdl_axis_to_maki_button_)/sizeof(sdl_axis_to_maki_button_[0])) {
								controllers_[i].values_[sdl_axis_to_maki_button_[e.caxis.axis]] = signed_analog_input_to_float(e.caxis.value, maki_button_deadzones_[e.caxis.axis]);
							}
						}
						break;
					case SDL_CONTROLLERBUTTONDOWN:
					case SDL_CONTROLLERBUTTONUP:
						{
							int32 i = get_controller_index(e.cbutton.which);
							if(i >= 0) {
								input_state_t::button_t btn = SDLButtonToMakiButton[e.cbutton.button];
								if(btn != input_state_t::button_invalid_) {
									controllers_[i].values_[btn] = e.cbutton.state * 1.0f;
								}
							}
						}
						break;
					case SDL_CONTROLLERDEVICEADDED:
						connect_game_controller(e.cdevice.which);
						break;
					case SDL_CONTROLLERDEVICEREMOVED:
						{
							int32 i = get_controller_index(e.cdevice.which);
							if(i >= 0) {
								SDL_GameControllerClose(controller_handles_[i]);
								controller_handles_[i] = nullptr;
								controller_instance_ids_[i] = -1;
								memset(controllers_[i].values_, 0, sizeof(controllers_[i].values_));
								console_t::info("controller_t disconnected (%d)", i);
							}
						}
						break;

					case SDL_KEYDOWN:
						// Support alt-f4 killing of the window_
						if(e.key.keysym.sym == SDLK_F4 && (e.key.keysym.mod & KMOD_ALT) != 0) {
							goto quit;
						}
						key_states_[sdl_key_to_maki_key(e.key.keysym.sym)].set(true);
						break;
					case SDL_KEYUP:
						key_states_[sdl_key_to_maki_key(e.key.keysym.sym)].set(false);
						break;
					
					case SDL_WINDOWEVENT:
						if(e.window_.event == SDL_WINDOWEVENT_RESIZED) {
							width_ = e.window_.data1;
							height_ = e.window_.data2;
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
