////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Application.h"

#include <iostream>
#include <algorithm>

#include "../ImGui/imgui.h"

#include "../ResourceLoader/ResourceLoader.h"

#include <dirent.h>

namespace CS350
{
	namespace
	{
		const int WIDTH = 1600;
		const int HEIGHT = 900;
	}

	Application::Application()
		: input(wnd), wnd(WIDTH, HEIGHT, "CS350", *this, input), gfx(WIDTH, HEIGHT, wnd), scene(gfx, time),
		cam_control(gfx, scene, input, time), gui(WIDTH, HEIGHT, time, input), running_(true)
	{}

	void Application::Run()
	{
		scene.InitializeScene();
		cam_control.Init(WIDTH, HEIGHT);

		while (running_)
		{
			time.Update();
			wnd.Update();
			input.Update();
			gui.Update();
			cam_control.Update();
			scene.Update();

			if (gui.GuiVisible())
			{
				gui.StartGuiWindow();		

				gui.EndGuiWindow();
			}

			gfx.Update();

			if (input.isKeyTriggered(0x1B)) //escape
				Quit();
		}
	}

	void Application::Quit()
	{
		running_ = false;
	}

	size_t Application::ObjFilesInDirectory(const char* directory, char*& files) const
	{
		DIR *dir;
		dirent *ent;

		if (dir = opendir(directory))
		{
			std::vector<std::string> file_names;
			std::string filename;

			while (ent = readdir(dir))
			{
				filename = ent->d_name;
				if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".obj") == 0)
					file_names.push_back(filename);
			}

			closedir(dir);

			size_t total_size = 1;
			for (unsigned i = 0; i < file_names.size(); ++i)
				total_size += file_names[i].size() + 1;
			
			files = new char[total_size];
			char *loc = files;
			for (auto file : file_names)
			{
				std::strcpy(loc, file.c_str());
				loc += file.size() + 1;
			}
			*loc = 0;

			return file_names.size();
		}

		return 0;
	}

	void Application::FreeFilenames(char* files) const
	{
		delete[] files;
	}
}
