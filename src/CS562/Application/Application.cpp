////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Application.h"

#include <iostream>
#include <algorithm>
#include <unordered_set>

#include "../ImGui/imgui.h"

#include "../ResourceLoader/ResourceLoader.h"

#include <dirent.h>

namespace CS562
{
	namespace
	{
		const int WIDTH = 1600;
		const int HEIGHT = 900;


		unsigned SkyboxesInDirectory(const char* directory, std::unique_ptr<char[]>& files)
		{
			DIR *dir;
			dirent *ent;

			if (dir = opendir(directory))
			{
				std::vector<std::string> reflectance_files;
				std::unordered_set<std::string> irradiance_files;
				std::string filename;

				while (ent = readdir(dir))
				{
					filename = ent->d_name;
					if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".hdr") == 0)
					{
						if (filename.size() >= 8 && filename.compare(filename.size() - 8, 4, ".irr") == 0)
						{
							irradiance_files.emplace(filename.substr(0, filename.size() - 8));
						}
						else
							reflectance_files.emplace_back(filename.substr(0, filename.size() - 4));
					}
				}

				closedir(dir);

				std::vector<std::string> matched_files;

				for (auto s : reflectance_files)
				{
					if (irradiance_files.find(s) != irradiance_files.end())
						matched_files.emplace_back(s);
				}

				unsigned total_size = 1;
				for (auto s : matched_files)
					total_size += static_cast<unsigned>(s.size() + 1);

				files = std::unique_ptr<char[]>(new char[total_size]);
				char *loc = files.get();
				for (auto file : matched_files)
				{
					std::strcpy(loc, file.c_str());
					loc += file.size() + 1;
				}
				*loc = 0;

				return static_cast<unsigned>(matched_files.size());
			}

			return 0;
		}
	}

	Application::Application()
		: input(wnd), wnd(WIDTH, HEIGHT, "CS562", *this, input), gfx(WIDTH, HEIGHT, wnd), scene(gfx, time),
		cam_control(gfx, scene, input, time), gui(WIDTH, HEIGHT, time, input), running_(true)
	{}

	void Application::Run()
	{
		scene.InitializeScene();
		cam_control.Init(WIDTH, HEIGHT);

		int show_buffer = DrawBuffers::LightAccum;
		const char* buffers[] = { "Light Accumulation","Position","Normal","Diffuse","Specular", "Shininess"};

		std::unique_ptr<char[]> files;
		unsigned num_files = SkyboxesInDirectory("skyboxes/", files);
		int curr_file = 0;

		gfx.SetSkybox(std::string(files.get()));

		while (running_)
		{
			time.Update();
			wnd.Update();
			input.Update();
			gui.Update();
			cam_control.Update();
			scene.Update();

			if (input.isKeyTriggered('O'))
				scene.PushLight();
			if (input.isKeyTriggered('P'))
				scene.PopLight();

			if (gui.GuiVisible())
			{
				gui.StartGuiWindow();		

				if (ImGui::Combo("Buffer to show:", &show_buffer, buffers, 6))
					gfx.SetShownBuffer(show_buffer);

				ImGui::Combo("Skybox", &curr_file, files.get());
				if (ImGui::Button("Load Skybox"))
				{
					std::string path = "skyboxes/";
					char *filename = files.get();
					for (int i = 0; i < curr_file; ++i)
					{
						filename += std::strlen(filename) + 1;
					}
					path += filename;
					gfx.SetSkybox(path);
				}
				if (ImGui::Button("Recheck Directory"))
				{
					files.reset();
					num_files = SkyboxesInDirectory("skyboxes/", files);
					curr_file = 0;
				}

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
}
