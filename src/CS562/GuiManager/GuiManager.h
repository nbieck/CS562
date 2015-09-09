////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_GUI_MANAGER_H_
#define CS350_GUI_MANAGER_H_

#include "../FrameTimer/FrameTimer.h"
#include "../Input/InputManager.h"

#include "../GLWrapper/ShaderProgram.h"
#include "../GLWrapper/Buffer.h"
#include "../GLWrapper/VertexArray.h"
#include "../ImGui/imgui.h"

namespace CS562
{
	class GuiManager
	{
	public:
		GuiManager(unsigned width, unsigned height, FrameTimer& time, InputManager& input);

		~GuiManager();

		void Update();

		void StartGuiWindow();
		void EndGuiWindow();

		bool GuiVisible() const;

	private:

		void DrawInfoOverlay();

		FrameTimer& time_;
		InputManager& input_;

		std::shared_ptr<ShaderProgram> shader_;
		std::shared_ptr<Buffer<ImDrawVert>> vbo_;
		std::unique_ptr<VertexArray> vao_;
		unsigned gui_tex_;

		bool gui_visible_;

		static GuiManager* g_gui_;
		static void RenderImGui(ImDrawList** const cmd_lists, int cmd_lists_count);
	};
}

#endif
