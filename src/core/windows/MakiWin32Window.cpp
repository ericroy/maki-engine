#include "core/core_stdafx.h"
#include "core/MakiConfig.h"
#include "core/MakiEngine.h"
#include "core/windows/MakiWin32Window.h"

#include <io.h>
#include <fcntl.h>

namespace Maki
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





	// Dispatch to appropriate instance's member function
	LRESULT CALLBACK WndProcDispatcher(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		Win32Window *instance;
		if(message == WM_CREATE) {
			instance = (Win32Window *)((CREATESTRUCT *)lparam)->lpCreateParams;
			SetWindowLongPtr(hwnd, 0, reinterpret_cast<LONG_PTR>(instance));
		} else {
			instance = reinterpret_cast<Win32Window *>(GetWindowLongPtr(hwnd, 0));
		}
		return instance->WndProc(hwnd, message, wparam, lparam);
	}




	Win32Window::Win32Window(HINSTANCE hinst, LPCTSTR title, const Config *config)
	:	Window(config->GetInt("engine.width", 800), config->GetInt("engine.height", 600)),
		hinst(hinst),
		hdc(nullptr),
		connectedFlags(0)
	{
		memset(keyStates, 0, sizeof(keyStates));

		if(config->GetBool("engine.show_console", false)) {
			AllocConsole();
		}

		RegisterClass();

		DWORD dwExStyle = 0;
		DWORD dwStyle = WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX;

		if(config->GetBool("engine.full_screen", false)) {
			fullscreen = true;

			DEVMODE dmScreenSettings;                   // Device Mode
			memset(&dmScreenSettings,0,sizeof(dmScreenSettings));       // Makes Sure Memory's Cleared
			dmScreenSettings.dmSize=sizeof(dmScreenSettings);       // Size Of The Devmode Structure
			dmScreenSettings.dmPelsWidth    = width;            // Selected Screen Width
			dmScreenSettings.dmPelsHeight   = height;           // Selected Screen Height
			dmScreenSettings.dmBitsPerPel   = 32;             // Selected Bits Per Pixel
			dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
			ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP;
			ShowCursor(FALSE);
		}

		RECT r;
		r.left = 0;
		r.top = 0;
		r.right = width;
		r.bottom = height;
		AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle);

		if(CreateWindowEx(dwExStyle, TEXT("MakiWindow"), title, dwStyle,
			CW_USEDEFAULT, CW_USEDEFAULT, r.right-r.left, r.bottom-r.top, NULL, NULL, hinst, this) == 0) {
			assert(false && "CreateWindow failed");
		}

		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
	}

	Win32Window::~Win32Window() {
	}

	int Win32Window::Pump(Engine *engine) {
		MSG msg;
		memset(&msg, 0, sizeof(msg));

		while(true) {
			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if(msg.message == WM_QUIT) {
					goto quit;
				}			
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			engine->Update();
			engine->Draw();
		}

	quit:
		return (int)msg.wParam;;
	}

	void Win32Window::AllocConsole() {
		::AllocConsole();
		HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
		int hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
		FILE* hf_out = _fdopen(hCrt, "w");
		setvbuf(hf_out, NULL, _IONBF, 1);
		*stdout = *hf_out;
	}

	void Win32Window::RegisterClass() {
		WNDCLASSEX wcex;
		memset(&wcex, 0, sizeof(WNDCLASSEX));

		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wcex.lpfnWndProc	= WndProcDispatcher;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hinst;
		//wcex.hIcon			= LoadIcon(hinst, MAKEINTRESOURCE(IDI_MAKIAPP));
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		//wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		//wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MAKIAPP);
		wcex.lpszClassName	= TEXT("MakiWindow");
		//wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
		wcex.cbWndExtra		= sizeof(Win32Window *);

		RegisterClassEx(&wcex);
	}

	LRESULT Win32Window::WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
		switch (message) {
		case WM_CREATE:
			this->hwnd = hwnd;
			hdc = GetDC(hwnd);
			break;
		case WM_ERASEBKGND:
			return 1;
		case WM_CLOSE:
			/*if(OnClose.cb) {
				OnClose.cb(OnClose.instance);
			}*/
			if(hdc != INVALID_HANDLE_VALUE) {
				ReleaseDC(hwnd, hdc);
			}
			DestroyWindow(hwnd);
			break;
		case WM_SIZE:
			width = LOWORD(lparam);
			height = HIWORD(lparam);
			if(width <= 0 || height <= 0) { width = 1; height = 1; }
			break;
		case WM_KEYDOWN:
			keyStates[wparam].Set(true);
			break;
		case WM_KEYUP:
			keyStates[wparam].Set(false);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, message, wparam, lparam);
		}
		return 0;
	}

	void Win32Window::PollInput(InputState *state) {
		for(uint8 i = 0; i < InputState::MAX_PLAYERS; i++) {
			// Copy keystate to player
			state->GetPlayer(i)->SetKeyStates(keyStates);

			bool wasConnected = (connectedFlags & (1<<i)) != 0;
			uint32 packetNum = wasConnected ? controllers[i].dwPacketNumber : 0;
			
			ZeroMemory(&controllers[i], sizeof(XINPUT_STATE));
			DWORD res = XInputGetState(i, &controllers[i]);
			if(res != ERROR_SUCCESS) {
				if(wasConnected) {
					Console::Info("Controller %d disconnected", i);
					state->DisconnectController(i);
				}
				connectedFlags &= ~(1<<i);
				continue;
			}
			
			const XINPUT_STATE &xs = controllers[i];

			// Mark as connected
			connectedFlags |= 1<<i;
			if(!wasConnected) {
				Console::Info("Controller %d connected", i);
				state->ConnectController(i);
			}
			
			if(xs.dwPacketNumber == packetNum) {
				// State has not changed
				continue;
			}
			// Else, look for state changes
			
			InputState::Controller &c = state->GetController(i);

			// Binary stuff
			c.values[InputState::Button_DPadUp] = (xs.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0 ? 1.0f : 0.0f;
			c.values[InputState::Button_DPadDown] = (xs.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0 ? 1.0f : 0.0f;
			c.values[InputState::Button_DPadLeft] = (xs.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0 ? 1.0f : 0.0f;
			c.values[InputState::Button_DPadRight] = (xs.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0 ? 1.0f : 0.0f;

			c.values[InputState::Button_A] = (xs.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0 ? 1.0f : 0.0f;
			c.values[InputState::Button_B] = (xs.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0 ? 1.0f : 0.0f;
			c.values[InputState::Button_X] = (xs.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0 ? 1.0f : 0.0f;
			c.values[InputState::Button_Y] = (xs.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0 ? 1.0f : 0.0f;

			c.values[InputState::Button_LeftShoulder] = (xs.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0 ? 1.0f : 0.0f;
			c.values[InputState::Button_RightShoulder] = (xs.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0 ? 1.0f : 0.0f;
			
			c.values[InputState::Button_LeftThumb] = (xs.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0 ? 1.0f : 0.0f;
			c.values[InputState::Button_RightThumb] = (xs.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0 ? 1.0f : 0.0f;
			
			c.values[InputState::Button_Back] = (xs.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0 ? 1.0f : 0.0f;
			c.values[InputState::Button_Start] = (xs.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0 ? 1.0f : 0.0f;

			// Analog stuff
			c.values[InputState::Button_LeftTrigger] = AnalogInputToFloat(xs.Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
			c.values[InputState::Button_RightTrigger] = AnalogInputToFloat(xs.Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
			c.values[InputState::Button_LeftThumbX] = -SignedAnalogInputToFloat(xs.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
			c.values[InputState::Button_LeftThumbY] = SignedAnalogInputToFloat(xs.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
			c.values[InputState::Button_RightThumbX] = -SignedAnalogInputToFloat(xs.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
			c.values[InputState::Button_RightThumbY] = SignedAnalogInputToFloat(xs.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		}

		// Clear transition counts for next frame
		for(uint32 i = 0; i < 256; i++) {
			keyStates[i].transitionCount = 0;
		}
	}

} // namespace Maki
