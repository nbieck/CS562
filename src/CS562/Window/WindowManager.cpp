////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "WindowManager.h"

#include "../Application/Application.h"
#include "../Input/InputManager.h"

#include <Windows.h>

namespace CS562
{
	namespace
	{
		LRESULT CALLBACK WindowProc(
			HWND window_handle,
			UINT message,
			WPARAM w_param,
			LPARAM l_param
			)
		{
			//get the pointer to the input manager from the window to have it check for input messages
			LONG_PTR data = GetWindowLongPtr(window_handle, 0);
			InputManager *input = reinterpret_cast<InputManager *>(data);

			if (input->HandleMessage(message, w_param, l_param))
				return 0;

			switch (message)
			{
			case WM_DESTROY:
				{
				//this is so that we do not quit because the temporary window was destroyed
				LONG_PTR data = GetWindowLongPtr(window_handle, sizeof(LONG_PTR));
				if (data == 0)
				{
					PostQuitMessage(0);
				}
				return 0;
				} break;
			default: 
				{
				return DefWindowProc(window_handle, message, w_param, l_param);
				} break;
			}
		}
	}

	struct WindowManager::PImpl
	{
		WindowManager& owner;
		Application& app;
		InputManager& input;

		const char* window_title;
		WNDCLASSEX wnd_class;
		HWND window_handle;

		int width, height;

		PImpl(WindowManager& own, Application& app, InputManager& input, const char* class_name, int width, int height) 
			: owner(own), app(app), input(input), window_title(class_name), wnd_class({ 0 }), window_handle(0), width(width), height(height)
		{}

		~PImpl()
		{
			DestroyWindow(window_handle);
			UnregisterClass(window_title, GetModuleHandle(NULL));
		}

		void MakeWindowClass()
		{
			wnd_class.cbSize = sizeof(WNDCLASSEX);							// The default size of our window class structure
			wnd_class.style = CS_HREDRAW | CS_VREDRAW;						// the style of the window
			wnd_class.lpfnWndProc = WindowProc;								// Pointer to our message handler function (declared above, but defined below)
			wnd_class.cbClsExtra = 0;										// The number of extra bytes you want to allocate for this window class structure. the default is 0
			wnd_class.cbWndExtra = sizeof(LONG_PTR) + sizeof(LONG_PTR);		// The number of extra bytes you want to allocate for the window instance. Going to use this to tell the window about the input manager
			wnd_class.hInstance = GetModuleHandle(NULL);					// Handle to the instance that contains the window procedure for the class
			wnd_class.hIcon = NULL;											// Let the system provide the default icon.
			wnd_class.hCursor = LoadCursor(NULL, IDC_ARROW);				// Use default cursor. The value of hinstance in that function is null in order to use one of the predefined cursors.
			wnd_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	// Handle tot eh calss background brush. It can be used as a color
			wnd_class.lpszMenuName = NULL;									// No menu for that window
			wnd_class.hIconSm = NULL;										// A handle to a small icon that is associated with the window class. We will use the same icon as the default one.
			wnd_class.lpszClassName = window_title;							// A string that specifies the window class name. The name must be less than 255.

			if (!RegisterClassEx(&wnd_class))
			{
				MessageBox(NULL, "Could not Register Window Class.", "Fatal Error", MB_OK | MB_ICONERROR);
				throw "Window Class Registration failed";
			}
		}

		void MakeWindow(bool is_temp_window)
		{
			// IMPORTANT STEP - The width you pass to the create window function is 
			// the width of the window including the title bar and the borders
			// to get the actual size of a window given the desired CLIENT area
			// use AdjustWindowRect function 
			// http://msdn.microsoft.com/en-us/library/ms632665(v=vs.85).aspx
			RECT winrect = { 0, 0, width, height };
			AdjustWindowRect(&winrect, WS_OVERLAPPEDWINDOW, FALSE);

			// Create the window - http://msdn.microsoft.com/en-us/library/windows/desktop/ms632679(v=vs.85).aspx
			window_handle = CreateWindow(
				window_title,						// The name of the window class - we used the same name as the window title 
				window_title,						// The window title - This will appear in the title bar and can be changed at run time
				WS_BORDER | WS_CAPTION | WS_SYSMENU,// The style of the window
				0,									// position of the window
				0,									// position of the window
				winrect.right - winrect.left,		// Width of the window in window size NOT client size
				winrect.bottom - winrect.top,		// Height of the windowin window size NOT client size
				NULL,								// Parent Window
				NULL,								// Handle to the menu
				GetModuleHandle(NULL),				// Instance of the application to be associated with that window
				NULL);								// Pointer to the value to be passed to the window when it's created and when it receives the WM_CREATE message. No need here.

			// NOTE: If the window was not created successfully, then the handle returned by CreateWindow should be null. 

			// Post an error message if the window was not created successfully
			if (!window_handle)
			{
				MessageBox(NULL, "Failed to Create the window!", "Fatal Error", MB_OK | MB_ICONERROR);
				throw "Window Creation Failed";
			}
			
			if (!is_temp_window)
			{
				// Creating the window was successful -> Show the window
				ShowWindow(window_handle, SW_SHOW);
				// Updates the client area of the window by sending a paint message to the window.
				UpdateWindow(window_handle);	
			}

			//set the input manager into the extra window data so the window procedure can access it
			SetWindowLongPtr(window_handle, 0, reinterpret_cast<LONG_PTR>(&input));
			SetWindowLongPtr(window_handle, sizeof(LONG_PTR), is_temp_window);
		}
	};

	WindowManager::~WindowManager() = default;

	WindowManager::WindowManager(int width, int height, const char* window_title, Application& app, InputManager& input)
		: impl(std::make_unique<PImpl>(*this, app, input, window_title, width, height))
	{
		impl->MakeWindowClass();
		impl->MakeWindow(true);
	}

	void WindowManager::Update()
	{
		MSG msg;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				impl->app.Quit();
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	HWND WindowManager::GetWindowHandle()
	{
		return impl->window_handle;
	}

	void WindowManager::MakeNewWindow()
	{
		impl->MakeWindow(false);
	}

	int WindowManager::GetWindowHeight() const
	{
		return impl->height;
	}

	int WindowManager::GetWindowWidth() const
	{
		return impl->width;
	}
}
