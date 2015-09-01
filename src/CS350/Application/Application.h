////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_APPLICATION_H_
#define CS350_APPLICATION_H_

#include "../Window/WindowManager.h"
#include "../Input/InputManager.h"
#include "../Graphics/GraphicsManager.h"
#include "../Scene/SceneManager.h"
#include "../CameraController/CameraController.h"
#include "../FrameTimer/FrameTimer.h"
#include "../GuiManager/GuiManager.h"

namespace CS350
{
	class Application
	{
	public:

		Application();

		void Run();

		void Quit();
		
		InputManager input;
		WindowManager wnd;
		GraphicsManager gfx;
		SceneManager scene;
		CameraController cam_control;
		FrameTimer time;
		GuiManager gui;

	private:

		size_t ObjFilesInDirectory(const char* directory, char*& files) const;
		void FreeFilenames(char* files) const;

		bool running_;
	};
}

#endif
