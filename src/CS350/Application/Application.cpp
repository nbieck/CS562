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
		: input(wnd), wnd(WIDTH, HEIGHT, "CS350", *this, input), gfx(WIDTH, HEIGHT, wnd, bound_vols), scene(gfx, time),
		cam_control(gfx, scene, input, time), gui(WIDTH, HEIGHT, time, input), running_(true), bound_vols(gfx)
	{}

	void Application::Run()
	{
		scene.InitializeScene();
		cam_control.Init(WIDTH, HEIGHT);

		auto objects = scene.GetObjectList();

		std::vector<std::shared_ptr<Object>> static_geometry(objects.size());

		std::copy_if(objects.begin(), objects.end(), static_geometry.begin(), 
			[&](std::shared_ptr<Object> o)
			{ 
				return o->drawable != nullptr && o != scene.dynamic_obj; 
			});
		
		static_geometry.erase(std::find(static_geometry.begin(), static_geometry.end(), nullptr), static_geometry.end());

		bound_vols.RecomputeBoundingVolumes(BVComputationType::AABB, objects);
		bound_vols.ComputeTree(static_geometry);

		glm::vec3 scene_min, scene_max;
		bound_vols.GetSceneExtents(scene_min, scene_max);
		scene.SetSceneExtents(scene_min, scene_max);

		bool render_tree = false;
		bool render_volumes = false;
		bool render_wireframe = false;

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

				if (ImGui::Checkbox("Render Octtree", &render_tree))
					bound_vols.SetRenderTree(render_tree);

				if (ImGui::Checkbox("Render Bounding Volumes", &render_volumes))
					bound_vols.SetDrawBoundingVolumes(render_volumes);

				if (ImGui::Checkbox("Render Wireframe", &render_wireframe))
					gfx.SetDrawmode((render_wireframe) ? Drawmode::Wireframe : Drawmode::Solid);

				ImGui::Separator();

				ImGui::Checkbox("Pause Simulation", &scene.simulation_paused);

				if (scene.simulation_paused)
					if (ImGui::Button("Single Step"))
						scene.single_step = true;

				if (ImGui::Button("Reverse Simulation"))
					scene.ReverseSimulation();

				gui.EndGuiWindow();
			}

			bound_vols.Update();

			bound_vols.CheckCollisions(scene.dynamic_obj);

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
