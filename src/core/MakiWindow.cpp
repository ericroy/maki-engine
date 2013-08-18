#include "core/core_stdafx.h"
#include "core/MakiWindow.h"
#include "core/MakiConfig.h"
#include "core/MakiEngine.h"
#include "core/MakiInputState.h"
#include <sstream>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <fcntl.h>
#endif

namespace Maki
{
	namespace Core
	{

#if defined(_WIN32) || defined(_WIN64)
		void ShowConsole()
		{
			::AllocConsole();
			HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
			int hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
			FILE* hf_out = _fdopen(hCrt, "w");
			setvbuf(hf_out, NULL, _IONBF, 1);
			*stdout = *hf_out;
		}
#endif


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

		InputState::Button SDLButtonToMakiButton[SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_MAX] = {
			InputState::Button_A, // SDL_CONTROLLER_BUTTON_A,
			InputState::Button_B, // SDL_CONTROLLER_BUTTON_B,
			InputState::Button_X, // SDL_CONTROLLER_BUTTON_X,
			InputState::Button_Y, // SDL_CONTROLLER_BUTTON_Y,
			InputState::Button_Back, // SDL_CONTROLLER_BUTTON_BACK,
			InputState::Button_Invalid, // SDL_CONTROLLER_BUTTON_GUIDE,
			InputState::Button_Start, // SDL_CONTROLLER_BUTTON_START,
			InputState::Button_LeftThumb, // SDL_CONTROLLER_BUTTON_LEFTSTICK,
			InputState::Button_RightThumb, // SDL_CONTROLLER_BUTTON_RIGHTSTICK,
			InputState::Button_LeftShoulder, // SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
			InputState::Button_RightShoulder, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
			InputState::Button_DPadUp, // SDL_CONTROLLER_BUTTON_DPAD_UP,
			InputState::Button_DPadDown, // SDL_CONTROLLER_BUTTON_DPAD_DOWN,
			InputState::Button_DPadLeft, // SDL_CONTROLLER_BUTTON_DPAD_LEFT,
			InputState::Button_DPadRight, // SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
		};

		InputState::Button SDLAxisToMakiButton[6] = {
			InputState::Button_LeftThumbX,
			InputState::Button_LeftThumbY,
			InputState::Button_RightThumbX,
			InputState::Button_RightThumbY,
			InputState::Button_LeftTrigger,
			InputState::Button_RightTrigger,
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

		inline InputState::Key SDLKeyToMakiKey(SDL_Keycode k)
		{
			switch(k) {			
			/*case SDLK_: return InputState::Key_LButton;
			case SDLK_: return InputState::Key_RButton;
			case SDLK_: return InputState::Key_Cancel;
			case SDLK_: return InputState::Key_MButton;
			case SDLK_: return InputState::Key_XButton1;
			case SDLK_: return InputState::Key_XButton2;
			case SDLK_: return InputState::Key_Back;
			*/
			case SDLK_TAB: return InputState::Key_Tab;
			case SDLK_CLEAR: return InputState::Key_Clear;
			case SDLK_RETURN: return InputState::Key_Return;
			//case SDLK_: return InputState::Key_Shift;
			//case SDLK_: return InputState::Key_Control;
			case SDLK_MENU: return InputState::Key_Menu;
			case SDLK_PAUSE: return InputState::Key_Pause;
			case SDLK_CAPSLOCK: return InputState::Key_Capital;
			case SDLK_ESCAPE: return InputState::Key_Escape;
			//case SDLK_: return InputState::Key_Convert;
			//case SDLK_: return InputState::Key_Nonconvert;
			//case SDLK_: return InputState::Key_Accept;
			//case SDLK_: return InputState::Key_Modechange;
			case SDLK_SPACE: return InputState::Key_Space;
			case SDLK_PAGEUP: return InputState::Key_Prior;
			case SDLK_PAGEDOWN: return InputState::Key_Next;
			case SDLK_END: return InputState::Key_End;
			case SDLK_HOME: return InputState::Key_Home;
			case SDLK_LEFT: return InputState::Key_Left;
			case SDLK_UP: return InputState::Key_Up;
			case SDLK_RIGHT: return InputState::Key_Right;
			case SDLK_DOWN: return InputState::Key_Down;
			case SDLK_SELECT: return InputState::Key_Select;
			//case SDLK_: return InputState::Key_Print;
			case SDLK_EXECUTE: return InputState::Key_Execute;
			case SDLK_PRINTSCREEN: return InputState::Key_Snapshot;
			case SDLK_INSERT: return InputState::Key_Insert;
			case SDLK_DELETE: return InputState::Key_Delete;
			case SDLK_HELP: return InputState::Key_Help;
			case SDLK_0: return InputState::Key_0;
			case SDLK_1: return InputState::Key_1;
			case SDLK_2: return InputState::Key_2;
			case SDLK_3: return InputState::Key_3;
			case SDLK_4: return InputState::Key_4;
			case SDLK_5: return InputState::Key_5;
			case SDLK_6: return InputState::Key_6;
			case SDLK_7: return InputState::Key_7;
			case SDLK_8: return InputState::Key_8;
			case SDLK_9: return InputState::Key_9;
			case SDLK_a: return InputState::Key_A;
			case SDLK_b: return InputState::Key_B;
			case SDLK_c: return InputState::Key_C;
			case SDLK_d: return InputState::Key_D;
			case SDLK_e: return InputState::Key_E;
			case SDLK_f: return InputState::Key_F;
			case SDLK_g: return InputState::Key_G;
			case SDLK_h: return InputState::Key_H;
			case SDLK_i: return InputState::Key_I;
			case SDLK_j: return InputState::Key_J;
			case SDLK_k: return InputState::Key_K;
			case SDLK_l: return InputState::Key_L;
			case SDLK_m: return InputState::Key_M;
			case SDLK_n: return InputState::Key_N;
			case SDLK_o: return InputState::Key_O;
			case SDLK_p: return InputState::Key_P;
			case SDLK_q: return InputState::Key_Q;
			case SDLK_r: return InputState::Key_R;
			case SDLK_s: return InputState::Key_S;
			case SDLK_t: return InputState::Key_T;
			case SDLK_u: return InputState::Key_U;
			case SDLK_v: return InputState::Key_V;
			case SDLK_w: return InputState::Key_W;
			case SDLK_x: return InputState::Key_X;
			case SDLK_y: return InputState::Key_Y;
			case SDLK_z: return InputState::Key_Z;
			//case SDLK_: return InputState::Key_Lwin;
			//case SDLK_: return InputState::Key_Rwin;
			case SDLK_APPLICATION: return InputState::Key_Apps;
			case SDLK_SLEEP: return InputState::Key_Sleep;
			case SDLK_KP_0: return InputState::Key_Numpad0;
			case SDLK_KP_1: return InputState::Key_Numpad1;
			case SDLK_KP_2: return InputState::Key_Numpad2;
			case SDLK_KP_3: return InputState::Key_Numpad3;
			case SDLK_KP_4: return InputState::Key_Numpad4;
			case SDLK_KP_5: return InputState::Key_Numpad5;
			case SDLK_KP_6: return InputState::Key_Numpad6;
			case SDLK_KP_7: return InputState::Key_Numpad7;
			case SDLK_KP_8: return InputState::Key_Numpad8;
			case SDLK_KP_9: return InputState::Key_Numpad9;
			case SDLK_KP_MULTIPLY: return InputState::Key_Multiply;
			case SDLK_KP_PLUS: return InputState::Key_Add;
			case SDLK_KP_VERTICALBAR: return InputState::Key_Separator;
			case SDLK_KP_MINUS: return InputState::Key_Subtract;
			case SDLK_KP_DECIMAL: return InputState::Key_Decimal;
			case SDLK_KP_DIVIDE: return InputState::Key_Divide;
			case SDLK_F1: return InputState::Key_F1;
			case SDLK_F2: return InputState::Key_F2;
			case SDLK_F3: return InputState::Key_F3;
			case SDLK_F4: return InputState::Key_F4;
			case SDLK_F5: return InputState::Key_F5;
			case SDLK_F6: return InputState::Key_F6;
			case SDLK_F7: return InputState::Key_F7;
			case SDLK_F8: return InputState::Key_F8;
			case SDLK_F9: return InputState::Key_F9;
			case SDLK_F10: return InputState::Key_F10;
			case SDLK_F11: return InputState::Key_F11;
			case SDLK_F12: return InputState::Key_F12;
			case SDLK_F13: return InputState::Key_F13;
			case SDLK_F14: return InputState::Key_F14;
			case SDLK_F15: return InputState::Key_F15;
			case SDLK_F16: return InputState::Key_F16;
			case SDLK_F17: return InputState::Key_F17;
			case SDLK_F18: return InputState::Key_F18;
			case SDLK_F19: return InputState::Key_F19;
			case SDLK_F20: return InputState::Key_F20;
			case SDLK_F21: return InputState::Key_F21;
			case SDLK_F22: return InputState::Key_F22;
			case SDLK_F23: return InputState::Key_F23;
			case SDLK_F24: return InputState::Key_F24;
			case SDLK_NUMLOCKCLEAR: return InputState::Key_Numlock;
			case SDLK_SCROLLLOCK: return InputState::Key_Scroll;
			case SDLK_LSHIFT: return InputState::Key_LShift;
			case SDLK_RSHIFT: return InputState::Key_RShift;
			case SDLK_LCTRL: return InputState::Key_LControl;
			case SDLK_RCTRL: return InputState::Key_RControl;
			//case SDLK_: return InputState::Key_LMenu;
			//case SDLK_: return InputState::Key_RMenu;
			case SDLK_AC_BACK: return InputState::Key_Browser_back;
			case SDLK_AC_FORWARD: return InputState::Key_BrowserForward;
			case SDLK_AC_REFRESH: return InputState::Key_BrowserRefresh;
			case SDLK_AC_STOP: return InputState::Key_BrowserStop;
			case SDLK_AC_SEARCH: return InputState::Key_BrowserSearch;
			case SDLK_AC_BOOKMARKS: return InputState::Key_BrowserFavorites;
			case SDLK_AC_HOME: return InputState::Key_BrowserHome;
			case SDLK_MUTE: return InputState::Key_VolumeMute;
			case SDLK_VOLUMEDOWN: return InputState::Key_VolumeDown;
			case SDLK_VOLUMEUP: return InputState::Key_VolumeUp;
			case SDLK_AUDIONEXT: return InputState::Key_MediaNextTrack;
			case SDLK_AUDIOPREV: return InputState::Key_MediaPrevTrack;
			case SDLK_AUDIOSTOP: return InputState::Key_MediaStop;
			case SDLK_AUDIOPLAY: return InputState::Key_MediaPlayPause;
			}
			return InputState::Key_Unknown;
		}
		


		Window::Window(RenderCore::Type renderCoreType, const Config *config)
			: window(nullptr),
			width(0),
			height(0),
			fullscreen(false)
		{
			memset(keyStates, 0, sizeof(keyStates));
			memset(controllerHandles, 0, sizeof(controllerHandles));
			memset(controllers, 0, sizeof(controllers));
			memset(controllerInstanceIds, 0xff, sizeof(controllerInstanceIds));

#if defined(_WIN32) || defined(_WIN64)
			if(config->GetBool("engine.show_console", false)) {
				ShowConsole();
			}
#endif

			SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

			// Load up any additional controller mappings that are specified in the config
			std::stringstream mappings;
			char controllerMappingKey[64];
			for(uint32 i = 0; ; i++) {
				sprintf_s(controllerMappingKey, "engine.controller_support_%d", i);
				const char *value = config->GetString(controllerMappingKey, nullptr);
				if(value == nullptr) {
					break;
				}
				mappings << value << std::endl;
			}
			SDL_SetHint(SDL_HINT_GAMECONTROLLERCONFIG, mappings.str().c_str());

			if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
				Console::Error("Failed to init SDL video subsystem: %s", SDL_GetError());
				SDL_ClearError();
			}			
			
			uint32 flags = SDL_WINDOW_INPUT_FOCUS|SDL_WINDOW_RESIZABLE|SDL_WINDOW_SHOWN;

			if(renderCoreType == RenderCore::Type_OGL) {
				flags |= SDL_WINDOW_OPENGL;
			}

			fullscreen = config->GetBool("engine.full_screen", false);
			if(fullscreen) {
				flags |= SDL_WINDOW_FULLSCREEN;
			}

			width = config->GetInt("engine.width", 800);
			height = config->GetInt("engine.height", 600);
			std::string title = config->GetString("engine.title", "");

			window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,  height, flags);
			const char *err = SDL_GetError();
			if(window == nullptr || strlen(err) != 0) {
				Console::Error("Failed to create SDL window: %s", SDL_GetError());
				SDL_ClearError();
			}

			// Discover already-connected game controllers
			int32 joystickCount = std::min(SDL_NumJoysticks(), InputState::MAX_PLAYERS);
			for(int32 i = 0; i < joystickCount; i++) {
				if(SDL_IsGameController(i)) {
					ConnectGameController(i);
				} else {
					SDL_Joystick *joy = SDL_JoystickOpen(i);
					if(joy != nullptr) {
						SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
						char buffer[33];
						SDL_JoystickGetGUIDString(guid, buffer, 33);
						Console::Info("Found joystick that was not a compatible game controller (%s: %s)", buffer, SDL_JoystickName(joy));
						SDL_JoystickClose(joy);
					}
				}
			}
		}

		Window::~Window()
		{
			if(window != nullptr) {
				SDL_DestroyWindow(window);
			}
			SDL_Quit();
		}

		void Window::PollInput(InputState *state)
		{
			for(uint32 i = 0; i < InputState::MAX_PLAYERS; i++) {
				state->GetPlayer(i)->SetKeyStates(keyStates);

				if(controllerHandles[i] != nullptr) {
					if(!state->GetController(i).connected) {
						// Just connected
						state->ConnectController(i);
					}
					memcpy(state->GetController(i).values, controllers[i].values, sizeof(controllers[i].values));
				} else if(state->GetController(i).connected) {
					// Just disconnected
					state->DisconnectController(i);
				}
			}
		}

		void Window::ConnectGameController(int32 i)
		{
			controllerHandles[i] = SDL_GameControllerOpen(i);
			if(controllerHandles[i] == nullptr) {
				Console::Error("Failed to open SDL game controller (%d): %s", i, SDL_GetError());
				SDL_ClearError();
			} else {
				controllerInstanceIds[i] = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controllerHandles[i]));
				Console::Info("Controller connected (%d, \"%s\")", i, SDL_GameControllerName(controllerHandles[i]));
			}
		}

		int Window::Pump(Engine *engine)
		{
			SDL_Event e;
			while(true) {
				while(SDL_PollEvent(&e) != 0) {
					switch(e.type) {
					case SDL_CONTROLLERAXISMOTION:
						{
							int32 i = GetControllerIndex(e.caxis.which);
							if(i >= 0 && e.caxis.axis < sizeof(SDLAxisToMakiButton)/sizeof(SDLAxisToMakiButton[0])) {
								controllers[i].values[SDLAxisToMakiButton[e.caxis.axis]] = SignedAnalogInputToFloat(e.caxis.value, MakiButtonDeadzones[e.caxis.axis]);
							}
						}
						break;
					case SDL_CONTROLLERBUTTONDOWN:
					case SDL_CONTROLLERBUTTONUP:
						{
							int32 i = GetControllerIndex(e.cbutton.which);
							if(i >= 0) {
								InputState::Button btn = SDLButtonToMakiButton[e.cbutton.button];
								if(btn != InputState::Button_Invalid) {
									controllers[i].values[btn] = e.cbutton.state * 1.0f;
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
								memset(controllers[i].values, 0, sizeof(controllers[i].values));
								Console::Info("Controller disconnected (%d)", i);
							}
						}
						break;

					case SDL_KEYDOWN:
						// Support alt-f4 killing of the window
						if(e.key.keysym.sym == SDLK_F4 && (e.key.keysym.mod & KMOD_ALT) != 0) {
							goto quit;
						}
						keyStates[SDLKeyToMakiKey(e.key.keysym.sym)].Set(true);
						break;
					case SDL_KEYUP:
						keyStates[SDLKeyToMakiKey(e.key.keysym.sym)].Set(false);
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
				engine->Tick();
			}

		quit:
			return 0;
		}

	} // namespace Core

} // namespace Maki