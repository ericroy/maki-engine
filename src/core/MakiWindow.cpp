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

		inline float SignedAnalogInputToFloat(int16 v, int16 dz) {
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

		inline float AnalogInputToFloat(uint8 v, uint8 dz) {
			if(v > dz) {
				return (v-dz) / (float)(255-dz);
			} else {
				return 0.0f;
			}
		}

		input_state_t::button_t SDLButtonToMakiButton[SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_MAX] = {
			input_state_t::Button_A, // SDL_CONTROLLER_BUTTON_A,
			input_state_t::Button_B, // SDL_CONTROLLER_BUTTON_B,
			input_state_t::Button_X, // SDL_CONTROLLER_BUTTON_X,
			input_state_t::Button_Y, // SDL_CONTROLLER_BUTTON_Y,
			input_state_t::Button_Back, // SDL_CONTROLLER_BUTTON_BACK,
			input_state_t::Button_Invalid, // SDL_CONTROLLER_BUTTON_GUIDE,
			input_state_t::Button_Start, // SDL_CONTROLLER_BUTTON_START,
			input_state_t::Button_LeftThumb, // SDL_CONTROLLER_BUTTON_LEFTSTICK,
			input_state_t::Button_RightThumb, // SDL_CONTROLLER_BUTTON_RIGHTSTICK,
			input_state_t::Button_LeftShoulder, // SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
			input_state_t::Button_RightShoulder, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
			input_state_t::Button_DPadUp, // SDL_CONTROLLER_BUTTON_DPAD_UP,
			input_state_t::Button_DPadDown, // SDL_CONTROLLER_BUTTON_DPAD_DOWN,
			input_state_t::Button_DPadLeft, // SDL_CONTROLLER_BUTTON_DPAD_LEFT,
			input_state_t::Button_DPadRight, // SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
		};

		input_state_t::button_t SDLAxisToMakiButton[6] = {
			input_state_t::Button_LeftThumbX,
			input_state_t::Button_LeftThumbY,
			input_state_t::Button_RightThumbX,
			input_state_t::Button_RightThumbY,
			input_state_t::Button_LeftTrigger,
			input_state_t::Button_RightTrigger,
		};

		static const uint16 GAMEPAD_LEFT_THUMB_DEADZONE = 7849;
		static const uint16 GAMEPAD_RIGHT_THUMB_DEADZONE = 8689;

		int16 MakiButtonDeadzones[6] = {
			GAMEPAD_LEFT_THUMB_DEADZONE,
			GAMEPAD_LEFT_THUMB_DEADZONE,
			GAMEPAD_RIGHT_THUMB_DEADZONE,
			GAMEPAD_RIGHT_THUMB_DEADZONE,
			0,
			0,
		};

		inline input_state_t::key_t SDLKeyToMakiKey(SDL_Keycode k)
		{
			switch(k) {			
			/*case SDLK_: return input_state_t::key_LButton;
			case SDLK_: return input_state_t::key_RButton;
			case SDLK_: return input_state_t::key_Cancel;
			case SDLK_: return input_state_t::key_MButton;
			case SDLK_: return input_state_t::key_XButton1;
			case SDLK_: return input_state_t::key_XButton2;
			case SDLK_: return input_state_t::key_Back;
			*/
			case SDLK_TAB: return input_state_t::key_Tab;
			case SDLK_CLEAR: return input_state_t::key_Clear;
			case SDLK_RETURN: return input_state_t::key_Return;
			//case SDLK_: return input_state_t::key_Shift;
			//case SDLK_: return input_state_t::key_Control;
			case SDLK_MENU: return input_state_t::key_Menu;
			case SDLK_PAUSE: return input_state_t::key_Pause;
			case SDLK_CAPSLOCK: return input_state_t::key_Capital;
			case SDLK_ESCAPE: return input_state_t::key_Escape;
			//case SDLK_: return input_state_t::key_Convert;
			//case SDLK_: return input_state_t::key_Nonconvert;
			//case SDLK_: return input_state_t::key_Accept;
			//case SDLK_: return input_state_t::key_Modechange;
			case SDLK_SPACE: return input_state_t::key_Space;
			case SDLK_PAGEUP: return input_state_t::key_Prior;
			case SDLK_PAGEDOWN: return input_state_t::key_Next;
			case SDLK_END: return input_state_t::key_End;
			case SDLK_HOME: return input_state_t::key_Home;
			case SDLK_LEFT: return input_state_t::key_Left;
			case SDLK_UP: return input_state_t::key_Up;
			case SDLK_RIGHT: return input_state_t::key_Right;
			case SDLK_DOWN: return input_state_t::key_Down;
			case SDLK_SELECT: return input_state_t::key_Select;
			//case SDLK_: return input_state_t::key_Print;
			case SDLK_EXECUTE: return input_state_t::key_Execute;
			case SDLK_PRINTSCREEN: return input_state_t::key_Snapshot;
			case SDLK_INSERT: return input_state_t::key_Insert;
			case SDLK_DELETE: return input_state_t::key_Delete;
			case SDLK_HELP: return input_state_t::key_Help;
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
			case SDLK_a: return input_state_t::key_A;
			case SDLK_b: return input_state_t::key_B;
			case SDLK_c: return input_state_t::key_C;
			case SDLK_d: return input_state_t::key_D;
			case SDLK_e: return input_state_t::key_E;
			case SDLK_f: return input_state_t::key_F;
			case SDLK_g: return input_state_t::key_G;
			case SDLK_h: return input_state_t::key_H;
			case SDLK_i: return input_state_t::key_I;
			case SDLK_j: return input_state_t::key_J;
			case SDLK_k: return input_state_t::key_K;
			case SDLK_l: return input_state_t::key_L;
			case SDLK_m: return input_state_t::key_M;
			case SDLK_n: return input_state_t::key_N;
			case SDLK_o: return input_state_t::key_O;
			case SDLK_p: return input_state_t::key_P;
			case SDLK_q: return input_state_t::key_Q;
			case SDLK_r: return input_state_t::key_R;
			case SDLK_s: return input_state_t::key_S;
			case SDLK_t: return input_state_t::key_T;
			case SDLK_u: return input_state_t::key_U;
			case SDLK_v: return input_state_t::key_V;
			case SDLK_w: return input_state_t::key_W;
			case SDLK_x: return input_state_t::key_X;
			case SDLK_y: return input_state_t::key_Y;
			case SDLK_z: return input_state_t::key_Z;
			//case SDLK_: return input_state_t::key_Lwin;
			//case SDLK_: return input_state_t::key_Rwin;
			case SDLK_APPLICATION: return input_state_t::key_Apps;
			case SDLK_SLEEP: return input_state_t::key_Sleep;
			case SDLK_KP_0: return input_state_t::key_Numpad0;
			case SDLK_KP_1: return input_state_t::key_Numpad1;
			case SDLK_KP_2: return input_state_t::key_Numpad2;
			case SDLK_KP_3: return input_state_t::key_Numpad3;
			case SDLK_KP_4: return input_state_t::key_Numpad4;
			case SDLK_KP_5: return input_state_t::key_Numpad5;
			case SDLK_KP_6: return input_state_t::key_Numpad6;
			case SDLK_KP_7: return input_state_t::key_Numpad7;
			case SDLK_KP_8: return input_state_t::key_Numpad8;
			case SDLK_KP_9: return input_state_t::key_Numpad9;
			case SDLK_KP_MULTIPLY: return input_state_t::key_Multiply;
			case SDLK_KP_PLUS: return input_state_t::key_Add;
			case SDLK_KP_VERTICALBAR: return input_state_t::key_Separator;
			case SDLK_KP_MINUS: return input_state_t::key_Subtract;
			case SDLK_KP_DECIMAL: return input_state_t::key_Decimal;
			case SDLK_KP_DIVIDE: return input_state_t::key_Divide;
			case SDLK_F1: return input_state_t::key_F1;
			case SDLK_F2: return input_state_t::key_F2;
			case SDLK_F3: return input_state_t::key_F3;
			case SDLK_F4: return input_state_t::key_F4;
			case SDLK_F5: return input_state_t::key_F5;
			case SDLK_F6: return input_state_t::key_F6;
			case SDLK_F7: return input_state_t::key_F7;
			case SDLK_F8: return input_state_t::key_F8;
			case SDLK_F9: return input_state_t::key_F9;
			case SDLK_F10: return input_state_t::key_F10;
			case SDLK_F11: return input_state_t::key_F11;
			case SDLK_F12: return input_state_t::key_F12;
			case SDLK_F13: return input_state_t::key_F13;
			case SDLK_F14: return input_state_t::key_F14;
			case SDLK_F15: return input_state_t::key_F15;
			case SDLK_F16: return input_state_t::key_F16;
			case SDLK_F17: return input_state_t::key_F17;
			case SDLK_F18: return input_state_t::key_F18;
			case SDLK_F19: return input_state_t::key_F19;
			case SDLK_F20: return input_state_t::key_F20;
			case SDLK_F21: return input_state_t::key_F21;
			case SDLK_F22: return input_state_t::key_F22;
			case SDLK_F23: return input_state_t::key_F23;
			case SDLK_F24: return input_state_t::key_F24;
			case SDLK_NUMLOCKCLEAR: return input_state_t::key_Numlock;
			case SDLK_SCROLLLOCK: return input_state_t::key_Scroll;
			case SDLK_LSHIFT: return input_state_t::key_LShift;
			case SDLK_RSHIFT: return input_state_t::key_RShift;
			case SDLK_LCTRL: return input_state_t::key_LControl;
			case SDLK_RCTRL: return input_state_t::key_RControl;
			//case SDLK_: return input_state_t::key_LMenu;
			//case SDLK_: return input_state_t::key_RMenu;
			case SDLK_AC_BACK: return input_state_t::key_Browser_back;
			case SDLK_AC_FORWARD: return input_state_t::key_BrowserForward;
			case SDLK_AC_REFRESH: return input_state_t::key_BrowserRefresh;
			case SDLK_AC_STOP: return input_state_t::key_BrowserStop;
			case SDLK_AC_SEARCH: return input_state_t::key_BrowserSearch;
			case SDLK_AC_BOOKMARKS: return input_state_t::key_BrowserFavorites;
			case SDLK_AC_HOME: return input_state_t::key_BrowserHome;
			case SDLK_MUTE: return input_state_t::key_VolumeMute;
			case SDLK_VOLUMEDOWN: return input_state_t::key_VolumeDown;
			case SDLK_VOLUMEUP: return input_state_t::key_VolumeUp;
			case SDLK_AUDIONEXT: return input_state_t::key_MediaNextTrack;
			case SDLK_AUDIOPREV: return input_state_t::key_MediaPrevTrack;
			case SDLK_AUDIOSTOP: return input_state_t::key_MediaStop;
			case SDLK_AUDIOPLAY: return input_state_t::key_MediaPlayPause;
			}
			return input_state_t::key_Unknown;
		}
		


		window_t::window_t(render_core_t::type_t renderCoreType, const config_t *config)
			: window(nullptr),
			width(0),
			height(0),
			fullscreen(false)
		{
			memset(keyStates, 0, sizeof(keyStates));
			memset(controllerHandles, 0, sizeof(controllerHandles));
			memset(controllers, 0, sizeof(controllers));
			memset(controllerInstanceIds, 0xff, sizeof(controllerInstanceIds));

			SDL_SetHint(SDL_HINT_RENDER_VSYNC, 0);
			SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

			// load up any additional controller mappings that are specified in the config
			std::stringstream mappings;
			char controllerMappingKey[64];
			for(uint32 i = 0; ; i++) {
				sprintf(controllerMappingKey, "engine.controller_support_%d", i);
				const char *value = config->get_string(controllerMappingKey, nullptr);
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

			if(renderCoreType == render_core_t::type_ogl_) {
				flags |= SDL_WINDOW_OPENGL;
			}

			fullscreen = config->get_bool("engine.full_screen", false);
			if(fullscreen) {
				flags |= SDL_WINDOW_FULLSCREEN;
			}

			width = config->get_int("engine.width_", 800);
			height = config->get_int("engine.height_", 600);
			std::string title = config->get_string("engine.title", "");

			window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,  height, flags);
			const char *err = SDL_GetError();
			if(window == nullptr || strlen(err) != 0) {
				console_t::error("Failed to create SDL window: %s", SDL_GetError());
				SDL_ClearError();
			}

			// Discover already-connected game controllers
			int32 joystickCount = std::min(SDL_NumJoysticks(), input_state_t::max_players_);
			for(int32 i = 0; i < joystickCount; i++) {
				if(SDL_IsGameController(i)) {
					ConnectGameController(i);
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
			if(window != nullptr) {
				SDL_DestroyWindow(window);
			}
			SDL_Quit();
		}

		void window_t::poll_input(input_state_t *state)
		{
			for(uint32 i = 0; i < input_state_t::max_players_; i++) {
				state->GetPlayer(i)->set_key_states(keyStates);

				if(controllerHandles[i] != nullptr) {
					if(!state->GetController(i).connected) {
						// Just connected
						state->ConnectController(i);
					}
					memcpy(state->GetController(i).values_, controllers[i].values_, sizeof(controllers[i].values_));
				} else if(state->GetController(i).connected) {
					// Just disconnected
					state->DisconnectController(i);
				}
			}
		}

		void window_t::ConnectGameController(int32 i)
		{
			controllerHandles[i] = SDL_GameControllerOpen(i);
			if(controllerHandles[i] == nullptr) {
				console_t::error("Failed to open SDL game controller (%d): %s", i, SDL_GetError());
				SDL_ClearError();
			} else {
				controllerInstanceIds[i] = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controllerHandles[i]));
				console_t::info("controller_t connected (%d, \"%s\")", i, SDL_GameControllerName(controllerHandles[i]));
			}
		}

		int window_t::Pump(engine_t *engine)
		{
			SDL_Event e;
			while(true) {
				while(SDL_PollEvent(&e) != 0) {
					switch(e.type_) {
					case SDL_CONTROLLERAXISMOTION:
						{
							int32 i = GetControllerIndex(e.caxis.which);
							if(i >= 0 && e.caxis.axis < sizeof(SDLAxisToMakiButton)/sizeof(SDLAxisToMakiButton[0])) {
								controllers[i].values_[SDLAxisToMakiButton[e.caxis.axis]] = SignedAnalogInputToFloat(e.caxis.value, MakiButtonDeadzones[e.caxis.axis]);
							}
						}
						break;
					case SDL_CONTROLLERBUTTONDOWN:
					case SDL_CONTROLLERBUTTONUP:
						{
							int32 i = GetControllerIndex(e.cbutton.which);
							if(i >= 0) {
								input_state_t::button_t btn = SDLButtonToMakiButton[e.cbutton.button];
								if(btn != input_state_t::Button_Invalid) {
									controllers[i].values_[btn] = e.cbutton.state_ * 1.0f;
								}
							}
						}
						break;
					case SDL_CONTROLLERDEVICEADDED:
						ConnectGameController(e.cdevice.which);
						break;
					case SDL_CONTROLLERDEVICEREMOVED:
						{
							int32 i = GetControllerIndex(e.cdevice.which);
							if(i >= 0) {
								SDL_GameControllerClose(controllerHandles[i]);
								controllerHandles[i] = nullptr;
								controllerInstanceIds[i] = -1;
								memset(controllers[i].values_, 0, sizeof(controllers[i].values_));
								console_t::info("controller_t disconnected (%d)", i);
							}
						}
						break;

					case SDL_KEYDOWN:
						// Support alt-f4 killing of the window
						if(e.key.keysym.sym == SDLK_F4 && (e.key.keysym.mod & KMOD_ALT) != 0) {
							goto quit;
						}
						keyStates[SDLKeyToMakiKey(e.key.keysym.sym)].set(true);
						break;
					case SDL_KEYUP:
						keyStates[SDLKeyToMakiKey(e.key.keysym.sym)].set(false);
						break;
					
					case SDL_WINDOWEVENT:
						if(e.window_.event == SDL_WINDOWEVENT_RESIZED) {
							width = e.window_.data1;
							height = e.window_.data2;
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
