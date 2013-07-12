#include "core/core_stdafx.h"
#include "core/MakiWindow.h"
#include "core/MakiConfig.h"
#include "core/MakiEngine.h"
#include "core/MakiInputState.h"

#if defined(_WIN32) || defined(_WIN64)
#define NOMINMAX 1
#include <Windows.h>
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
		


		Window::Window(const Config *config)
			: window(nullptr),
			width(0),
			height(0),
			fullscreen(false),
			joystickConnectedFlags(0)
		{
			memset(keyStates, 0, sizeof(keyStates));
			memset(joysticks, 0, sizeof(joysticks));

#if defined(_WIN32) || defined(_WIN64)
			if(config->GetBool("engine.show_console", false)) {
				ShowConsole();
			}
#endif

			if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
				Console::Error("Failed to init SDL video subsystem: %s", SDL_GetError());
				SDL_ClearError();
			}			
			
			uint32 flags = SDL_WINDOW_INPUT_FOCUS|SDL_WINDOW_SHOWN;

#if MAKI_USE_OGL
			flags |= SDL_WINDOW_OPENGL;
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
#endif

			fullscreen = config->GetBool("engine.full_screen", false);
			if(fullscreen) {
				flags |= SDL_WINDOW_FULLSCREEN;
			}

			width = config->GetInt("engine.width", 800);
			height = config->GetInt("engine.height", 600);
			std::string title = config->GetString("engine.title", "");

			window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,  height, flags);
			if(window == nullptr) {
				Console::Error("Failed to create SDL window: %s", SDL_GetError());
				SDL_ClearError();
			}

			int32 joystickCount = std::min(SDL_NumJoysticks(), InputState::MAX_PLAYERS);
			for(int32 i = 0; i < joystickCount; i++) {
				joysticks[i] = SDL_JoystickOpen(i);

				if(SDL_JoystickNumButtons(joysticks[i]) < 8 || SDL_JoystickNumAxes(joysticks[i]) < 4 || SDL_JoystickNumHats(joysticks[i]) < 1) {
					// Incompatible joystick - we require a gamepad basically
					SDL_JoystickClose(joysticks[i]);
					joysticks[i] = nullptr;
					continue;
				}
				
				if(SDL_JoystickEventState(SDL_QUERY) < 0) {
					Console::Error("Failed to put SDL joystick in query mode: %s", SDL_GetError());
					SDL_ClearError();
					SDL_JoystickClose(joysticks[i]);
					joysticks[i] = nullptr;
					continue;
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

		int Window::Pump(Engine *engine)
		{
			SDL_Event e;
			while(true) {
				while(SDL_PollEvent(&e) != 0) {
					switch(e.type) {
					case SDL_QUIT:
						goto quit;
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
					case SDL_WINDOWEVENT_RESIZED:
						width = e.window.data1;
						height = e.window.data2;
						if(width <= 0 || height <= 0) {
							width = 1;
							height = 1;
						}
						break;
					}
				}
				engine->Tick();
			}

		quit:
			return 0;
		}

		void Window::PollInput(InputState *state)
		{
			for(uint8 i = 0; i < InputState::MAX_PLAYERS; i++) {
				// Copy keystate to player
				state->GetPlayer(i)->SetKeyStates(keyStates);

				SDL_Joystick *joy = joysticks[i];
				if(joy == nullptr) {
					continue;
				}
				
				if((joystickConnectedFlags & (1 << i)) == 0) {
					// Just connected
					state->ConnectController(i);
					joystickConnectedFlags |= 1 << i;
				}

				// Poll the joystick before we check its state
				SDL_JoystickUpdate();

				InputState::Controller &c = state->GetController(i);
				
				c.values[InputState::Button_A] = (float)SDL_JoystickGetButton(joy, 0);
				c.values[InputState::Button_B] = (float)SDL_JoystickGetButton(joy, 1);
				c.values[InputState::Button_X] = (float)SDL_JoystickGetButton(joy, 2);
				c.values[InputState::Button_Y] = (float)SDL_JoystickGetButton(joy, 3);

				c.values[InputState::Button_LeftShoulder] = (float)SDL_JoystickGetButton(joy, 4);
				c.values[InputState::Button_RightShoulder] = (float)SDL_JoystickGetButton(joy, 5);

				// TODO:
				// An xbox controller has analog triggers (probably treated as an axis by SDL), but a older style gamepad probably has buttons
				// Make sure this works with both kinds of controllers
				c.values[InputState::Button_LeftTrigger] = (float)SDL_JoystickGetButton(joy, 6);
				c.values[InputState::Button_RightTrigger] = (float)SDL_JoystickGetButton(joy, 7);

				c.values[InputState::Button_Back] = (float)SDL_JoystickGetButton(joy, 8);
				c.values[InputState::Button_Start] = (float)SDL_JoystickGetButton(joy, 9);

				
				uint8 dpad = SDL_JoystickGetHat(joy, 0);
				c.values[InputState::Button_DPadUp] = (dpad & SDL_HAT_UP) != 0 ? 1.0f : 0.0f;
				c.values[InputState::Button_DPadDown] = (dpad & SDL_HAT_DOWN) != 0 ? 1.0f : 0.0f;
				c.values[InputState::Button_DPadLeft] = (dpad & SDL_HAT_LEFT) != 0 ? 1.0f : 0.0f;
				c.values[InputState::Button_DPadRight] = (dpad & SDL_HAT_RIGHT) != 0 ? 1.0f : 0.0f;


				// Analog stuff
				static const uint16 GAMEPAD_LEFT_THUMB_DEADZONE = 7849;
				static const uint16 GAMEPAD_RIGHT_THUMB_DEADZONE = 8689;
				c.values[InputState::Button_LeftThumbX] = -SignedAnalogInputToFloat(SDL_JoystickGetAxis(joy, 0), GAMEPAD_LEFT_THUMB_DEADZONE);
				c.values[InputState::Button_LeftThumbY] = -SignedAnalogInputToFloat(SDL_JoystickGetAxis(joy, 1), GAMEPAD_LEFT_THUMB_DEADZONE);
				c.values[InputState::Button_RightThumbX] = -SignedAnalogInputToFloat(SDL_JoystickGetAxis(joy, 2),GAMEPAD_RIGHT_THUMB_DEADZONE);
				c.values[InputState::Button_RightThumbY] = -SignedAnalogInputToFloat(SDL_JoystickGetAxis(joy, 3), GAMEPAD_RIGHT_THUMB_DEADZONE);
			}
		}


	} // namespace Core

} // namespace Maki