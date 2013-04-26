#pragma once
#include "core/core_stdafx.h"
#include "core/MakiWindow.h"
#include "core/MakiInputState.h"

#include <windows.h>
#include <XInput.h>

namespace Maki
{
	class Config;
	class Engine;

	class Win32Window : public Window
	{
		friend LRESULT CALLBACK WndProcDispatcher(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	public:
		Win32Window(HINSTANCE hinst, LPCTSTR title, const Config *config);
		virtual ~Win32Window();
		int Pump(Engine *engine);
		void PollInput(InputState *state);

	protected:
		void AllocConsole();
		void RegisterClass();
		LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		void Resize(int32 w, int32 h);

	public:
		HINSTANCE hinst;
		HWND hwnd;
		HDC hdc;
		
		XINPUT_STATE controllers[InputState::MAX_PLAYERS];
		uint8 connectedFlags;

		Maki::InputState::KeyStateReport keyStates[256];
	};

} // namespace Maki
