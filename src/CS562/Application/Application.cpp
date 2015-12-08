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
		const char* buffers[] = { "Light Accumulation","Position","Normal","Diffuse","Specular", "Shininess", "AO (no blur)", "AO (horizontal blur)", "AO (final)", "Hi-Z-Buffer", "Reflections"};

		const char* presets[] = { "Custom", "Black Non-Metal", "Gold", "Silver", "Copper", "Iron", "Aluminum" };

		std::unique_ptr<char[]> files;
		unsigned num_files = SkyboxesInDirectory("skyboxes/", files);
		int curr_file = 0;

		float e = 1.0f;
		float c = 1.0f;

		MaterialPresets preset = Custom;

		int num_samples = 30;

		gfx.SetSkybox(std::string("skyboxes/") + files.get());

		float ao_R = 1.0;
		float ao_delta = 0.001f;
		float ao_s = 1.0;
		float ao_k = 1.0;
		int ao_samples = 20;
		int ao_blur = 5;

		int mip_level = 0;

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

				if (ImGui::Combo("Buffer to show:", &show_buffer, buffers, 11))
					gfx.SetShownBuffer(show_buffer);

				if (show_buffer == 9)
				{
					if (ImGui::SliderInt("MipLevel", &mip_level, 0, 10))
						gfx.SetHiZMip(mip_level);
				}

				if (ImGui::CollapsingHeader("Tone mapping controls"))
				{
					if (ImGui::SliderFloat("Tone mapping exposure control", &e, 0.1f, 10000.f, "%.3f", 2.f))
						gfx.SetExposure(e);

					if (ImGui::SliderFloat("Contrast##ToneMapping", &c, 0.1f, 10.f))
						gfx.SetContrast(c);
				}

				if (ImGui::CollapsingHeader("Material Properties"))
				{
					if (ImGui::ColorEdit3("Diffuse", reinterpret_cast<float*>(&(scene.mat->k_d))))
						preset = Custom;
					if (ImGui::ColorEdit3("Specular", reinterpret_cast<float *>(&scene.mat->k_s)))
						preset = Custom;
					ImGui::SliderFloat("Roughness", &scene.mat->shininess, 0.f, 1000.f);
					if (ImGui::Combo("Preset", reinterpret_cast<int*>(&preset), presets, sizeof(presets) / sizeof(*presets)))
						UseMaterialPreset(preset);
					if (ImGui::SliderInt("Samples", &num_samples, 1, 100))
						gfx.SetNumSamples(num_samples);
				}

				if (ImGui::CollapsingHeader("Ambient Occlusion"))
				{
					if (ImGui::SliderFloat("Radius", &ao_R, 0.01f, 10.0f, "%.3f", 2.0f))
						gfx.SetAORadius(ao_R);
					if (ImGui::SliderFloat("Delta", &ao_delta, 0.0001f, 1.0f, "%.4f", 2.0f))
						gfx.SetAODelta(ao_delta);
					if (ImGui::SliderInt("# Samples", &ao_samples, 1, 50))
						gfx.SetAOSamples(ao_samples);
					if (ImGui::SliderFloat("Scale", &ao_s, 0.1f, 10.f))
						gfx.SetAOScale(ao_s);
					if (ImGui::SliderFloat("Contrast", &ao_k, 0.1f, 10.f))
						gfx.SetAOContrast(ao_k);
					if (ImGui::SliderInt("Blur Width", &ao_blur, 1, 50))
						gfx.SetShadowBlurWidth(ao_blur);
				}

				if (ImGui::CollapsingHeader("Skybox"))
				{
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
					ImGui::SameLine();
					if (ImGui::Button("Recheck Directory"))
					{
						files.reset();
						num_files = SkyboxesInDirectory("skyboxes/", files);
						curr_file = 0;
					}
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

	void Application::UseMaterialPreset(MaterialPresets preset)
	{
		switch (preset)
		{
		case Custom:
			break;
		case NonMetalBlack:
			scene.mat->k_d = glm::vec3(0);
			scene.mat->k_s = glm::vec3(0.05f);
			break;
		case Gold:
			scene.mat->k_d = glm::vec3(0);
			scene.mat->k_s = glm::vec3(1, 0.71f, 0.29f);
			break;
		case Silver:
			scene.mat->k_d = glm::vec3(0);
			scene.mat->k_s = glm::vec3(0.95f, 0.93f, 0.88f);
			break;
		case Copper:
			scene.mat->k_d = glm::vec3(0);
			scene.mat->k_s = glm::vec3(0.95f, 0.64f, 0.54f);
			break;
		case Iron:
			scene.mat->k_d = glm::vec3(0);
			scene.mat->k_s = glm::vec3(0.56f, 0.57f, 0.58f);
			break;
		case Aluminum:
			scene.mat->k_d = glm::vec3(0);
			scene.mat->k_s = glm::vec3(0.91f, 0.92f, 0.92f);
			break;
		}
	}
}
