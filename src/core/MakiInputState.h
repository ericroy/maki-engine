#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{

		class InputState
		{
		public:
			static const int32 MAX_PLAYERS = 4;

			enum Key
			{
				Key_LButton = 0x01,
				Key_RButton = 0x02,
				Key_Cancel = 0x03,
				Key_MButton = 0x04,
				Key_XButton1 = 0x05,
				Key_XButton2 = 0x06,
				Key_Back = 0x08,
				Key_Tab = 0x09,
				Key_Clear = 0x0c,
				Key_Return = 0x0d,
				Key_Shift = 0x10,
				Key_Control = 0x11,
				Key_Menu = 0x12,
				Key_Pause = 0x13,
				Key_Capital = 0x14,
				Key_Escape = 0x1b,
				Key_Convert = 0x1c,
				Key_Nonconvert = 0x1d,
				Key_Accept = 0x1e,
				Key_Modechange = 0x1f,
				Key_Space = 0x20,
				Key_Prior = 0x21,
				Key_Next = 0x22,
				Key_End = 0x23,
				Key_Home = 0x24,
				Key_Left = 0x25,
				Key_Up = 0x26,
				Key_Right = 0x27,
				Key_Down = 0x28,
				Key_Select = 0x29,
				Key_Print = 0x2a,
				Key_Execute = 0x2b,
				Key_Snapshot = 0x2c,
				Key_Insert = 0x2d,
				Key_Delete = 0x2e,
				Key_Help = 0x2f,
				Key_0 = 0x30,
				Key_1 = 0x31,
				Key_2 = 0x32,
				Key_3 = 0x33,
				Key_4 = 0x34,
				Key_5 = 0x35,
				Key_6 = 0x36,
				Key_7 = 0x37,
				Key_8 = 0x38,
				Key_9 = 0x39,
				Key_A = 0x41,
				Key_B = 0x42,
				Key_C = 0x43,
				Key_D = 0x44,
				Key_E = 0x45,
				Key_F = 0x46,
				Key_G = 0x47,
				Key_H = 0x48,
				Key_I = 0x49,
				Key_J = 0x4a,
				Key_K = 0x4b,
				Key_L = 0x4c,
				Key_M = 0x4d,
				Key_N = 0x4e,
				Key_O = 0x4f,
				Key_P = 0x50,
				Key_Q = 0x51,
				Key_R = 0x52,
				Key_S = 0x53,
				Key_T = 0x54,
				Key_U = 0x55,
				Key_V = 0x56,
				Key_W = 0x57,
				Key_X = 0x58,
				Key_Y = 0x59,
				Key_Z = 0x5a,
				Key_Lwin = 0x5b,
				Key_Rwin = 0x5c,
				Key_Apps = 0x5d,
				Key_Sleep = 0x5f,
				Key_Numpad0 = 0x60,
				Key_Numpad1 = 0x61,
				Key_Numpad2 = 0x62,
				Key_Numpad3 = 0x63,
				Key_Numpad4 = 0x64,
				Key_Numpad5 = 0x65,
				Key_Numpad6 = 0x66,
				Key_Numpad7 = 0x67,
				Key_Numpad8 = 0x68,
				Key_Numpad9 = 0x69,
				Key_Multiply = 0x6a,
				Key_Add = 0x6b,
				Key_Separator = 0x6c,
				Key_Subtract = 0x6d,
				Key_Decimal = 0x6e,
				Key_Divide = 0x6f,
				Key_F1 = 0x70,
				Key_F2 = 0x71,
				Key_F3 = 0x72,
				Key_F4 = 0x73,
				Key_F5 = 0x74,
				Key_F6 = 0x75,
				Key_F7 = 0x76,
				Key_F8 = 0x77,
				Key_F9 = 0x78,
				Key_F10 = 0x79,
				Key_F11 = 0x7a,
				Key_F12 = 0x7b,
				Key_F13 = 0x7c,
				Key_F14 = 0x7d,
				Key_F15 = 0x7e,
				Key_F16 = 0x7f,
				Key_F17 = 0x80,
				Key_F18 = 0x81,
				Key_F19 = 0x82,
				Key_F20 = 0x83,
				Key_F21 = 0x84,
				Key_F22 = 0x85,
				Key_F23 = 0x86,
				Key_F24 = 0x87,
				Key_Numlock = 0x90,
				Key_Scroll = 0x91,
				Key_LShift = 0xa0,
				Key_RShift = 0xa1,
				Key_LControl = 0xa2,
				Key_RControl = 0xa3,
				Key_LMenu = 0xa4,
				Key_RMenu = 0xa5,
				Key_Browser_back = 0xa6,
				Key_BrowserForward = 0xa7,
				Key_BrowserRefresh = 0xa8,
				Key_BrowserStop = 0xa9,
				Key_BrowserSearch = 0xaa,
				Key_BrowserFavorites = 0xab,
				Key_BrowserHome = 0xac,
				Key_VolumeMute = 0xad,
				Key_VolumeDown = 0xae,
				Key_VolumeUp = 0xaf,
				Key_MediaNextTrack = 0xb0,
				Key_MediaPrevTrack = 0xb1,
				Key_MediaStop = 0xb2,
				Key_MediaPlayPause = 0xb3
			};

			enum Button
			{
				// Digital stuff, values 0 or 1
				Button_DPadUp = 0, Button_DPadDown, Button_DPadLeft, Button_DPadRight,
				Button_Start, Button_Back,
				Button_LeftThumb, Button_RightThumb,
				Button_LeftShoulder, Button_RightShoulder,
				Button_A, Button_B, Button_X, Button_Y,

				// Potentially analog stuff, values [0, 1]
				Button_LeftTrigger, Button_RightTrigger,
				Button_LeftThumbX, Button_LeftThumbY,
				Button_RightThumbX, Button_RightThumbY,

				ButtonCount
			};

			// Reports the history of a key since the last tick
			struct KeyStateReport
			{
				inline void Set(bool on)
				{
					this->on = on ? 1 : 0;
					transitionCount++;
				}

				uint8 on : 1;
				uint8 transitionCount : 7;
			};


			struct Controller
			{
			public:
				inline float GetFloat(Button b) const { return values[mapping[b]]; }
				inline bool GetBool(Button b) const { return values[mapping[b]] == 1.0f; }
				inline void MapButton(Button hardwareButton, Button virtualButton) { mapping[virtualButton] = hardwareButton; }

				inline void SetMappingIdentity()
				{
					for(uint8 i = 0; i < ButtonCount; i++) {
						mapping[i] = i;
					}
				}

			public:
				bool connected;
				uint8 playerIndex;

				float values[ButtonCount];
				uint8 mapping[ButtonCount];
			};

			struct Player
			{
			private:
				struct KeyState
				{
					// True if the key is currently depressed
					uint8 on : 1;

					// True if the key went down in the last tick
					uint8 pressed : 1;
			
					// True if the key went up in the last tick
					uint8 released : 1;
				};

			public:

				// Returns true if the key is currently held down
				inline bool GetKey(Key k) const { return keyStates[keyMapping[k]].on != 0; }
			
				// Returns true if the key became depressed during the last tick
				inline bool GetKeyPressed(Key k) const { return keyStates[keyMapping[k]].pressed != 0; }

				// Returns true if the key was released during the last tick
				inline bool GetKeyReleased(Key k) const { return keyStates[keyMapping[k]].released != 0; }

				inline void MapKey(Key hardwareKey, Key virtualKey) { keyMapping[virtualKey] = hardwareKey; }

				inline void SetMappingIdentity()
				{
					for(uint32 i = 0; i < 256; i++) {
						keyMapping[i] = i;
					}
				}

				void SetKeyStates(const KeyStateReport states[256])
				{
					for(uint32 i = 0; i < 256; i++) {
						if(keyStates[i].on == states[i].on && states[i].transitionCount == 0) {
							// Nothing happened with this key
							keyStates[i].pressed = 0;
							keyStates[i].released = 0;
						} else {
							// Something happened
							// The key may have changed its 'on' state since last frame.
							// It is also possible that the key has the same 'on' state as last frame, but there was one
							// (or more) intra-frame up-down events.
							keyStates[i].on = states[i].on;
							if(states[i].on) {
								if(states[i].transitionCount == 1) {
									keyStates[i].pressed = 1;
									keyStates[i].released = 0;
								} else {
									keyStates[i].pressed = 1;
									keyStates[i].released = 1;
								}
							} else {
								if(states[i].transitionCount == 1) {
									keyStates[i].released = 1;
									keyStates[i].pressed = 0;
								} else {
									keyStates[i].released = 1;
									keyStates[i].pressed = 1;
								}
							}
						}
					}
				}

			public:
				Controller *controller;
		
			private:
				uint8 keyMapping[256];
				KeyState keyStates[256];
			};

		public:
			InputState();
			virtual ~InputState();
		
			// Hold a reference to the player object
			inline Player *GetPlayer(uint8 index) { assert(index < MAX_PLAYERS); return &players[index]; }

			// Don't hold a reference to the controller object - get it each time from the player
			// in case the controller associated with the player changes
			inline Controller &GetController(uint8 index) { assert(index < MAX_PLAYERS); return controllers[index]; }

			void ConnectController(uint8 controllerIndex);
			void DisconnectController(uint8 controllerIndex);

		private:
			uint8 controllerCount;
			Controller controllers[MAX_PLAYERS];
			Player players[MAX_PLAYERS];
		};

	} // namespace Core

} // namespace Maki