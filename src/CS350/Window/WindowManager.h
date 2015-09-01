////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_WINDOW_MANAGER_H_
#define CS350_WINDOW_MANAGER_H_

#include <memory>

//forward declare this here, HWND is a pointer to this structure, I don't really want to include Windows.h in here
struct HWND__;

namespace CS350
{
	class Application;
	class InputManager;

	class WindowManager
	{
	public:

		WindowManager(int width, int height, const char* window_title, Application& app, InputManager& input);
		~WindowManager();

		void Update();

		int GetWindowWidth() const;
		int GetWindowHeight() const;

		HWND__ *GetWindowHandle();

		//this should only be called during initialization
		void MakeNewWindow();

	private:

		// use pimpl idiom to keep windows out of the header
		struct PImpl;
		std::unique_ptr<PImpl> impl;
	};
}

#endif
