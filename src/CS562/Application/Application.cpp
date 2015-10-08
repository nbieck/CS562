////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Application.h"

#include <iostream>
#include <algorithm>

#include "../ImGui/imgui.h"

#include "../ResourceLoader/ResourceLoader.h"

namespace CS562
{
	namespace
	{
		const int WIDTH = 1600;
		const int HEIGHT = 900;
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
		bool show_shadowmap = true;

		int num_lights = 10;
		int curr_lights = 10;

		for (unsigned i = 0; i < 10; ++i)
			scene.PushLight();

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

				if (ImGui::SliderInt("NumLights", &num_lights, 0, 10000))
				{
					for (; num_lights > curr_lights; ++curr_lights)
						scene.PushLight();
					for (; num_lights < curr_lights; --curr_lights)
						scene.PopLight();
				}

				if (ImGui::Checkbox("Show Shadowmap", &show_shadowmap))
					gfx.SetShowShadowMap(show_shadowmap);

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
