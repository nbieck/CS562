////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "InputManager.h"

#include "../ImGui/imgui.h"

#include <Windows.h>

#include <array>

namespace CS350
{
	struct InputManager::PImpl
	{
		InputManager& owner;
		WindowManager& window;

		std::array<char, 256> current_keys;
		std::array<char, 256> prev_keys;

		std::array<char, 2> current_mouse;
		std::array<char, 2> prev_mouse;

		glm::vec2 mouse_pos;

		PImpl(InputManager& owner, WindowManager& window)
			: owner(owner), window(window)
		{
			current_keys.fill(0);
			prev_keys.fill(0);

			current_mouse.fill(0);
			prev_mouse.fill(0);
		}

		void InitImGuiInput()
		{
			ImGuiIO& io = ImGui::GetIO();
			
			io.KeyMap[ImGuiKey_Tab] = VK_TAB;
			io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
			io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
			io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
			io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
			io.KeyMap[ImGuiKey_Home] = VK_HOME;
			io.KeyMap[ImGuiKey_End] = VK_END;
			io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
			io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
			io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
			io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
			io.KeyMap[ImGuiKey_A] = 'A';
			io.KeyMap[ImGuiKey_C] = 'C';
			io.KeyMap[ImGuiKey_V] = 'V';
			io.KeyMap[ImGuiKey_X] = 'X';
			io.KeyMap[ImGuiKey_Y] = 'Y';
			io.KeyMap[ImGuiKey_Z] = 'Z';
		}
	};

	InputManager::~InputManager() = default;

	InputManager::InputManager(WindowManager& window)
		: impl(std::make_unique<PImpl>(*this, window))
	{
		impl->InitImGuiInput();
	}

	bool InputManager::HandleMessage(unsigned message, WParam w_param, LParam l_param)
	{
		ImGuiIO& io = ImGui::GetIO();

		switch (message)
		{
		case WM_KEYDOWN:
			{
			impl->current_keys[w_param] = 1;
			io.KeysDown[w_param] = true;
			if (w_param == VK_CONTROL)
				io.KeyCtrl = true;
			if (w_param == VK_SHIFT)
				io.KeyShift = true;
			} break;
		case WM_KEYUP:
			{
			impl->current_keys[w_param] = 0;
			io.KeysDown[w_param] = false;
			if (w_param == VK_CONTROL)
				io.KeyCtrl = false;
			if (w_param == VK_SHIFT)
				io.KeyShift = false;
			} break;
		case WM_LBUTTONDOWN:
			{
			impl->current_mouse[Button::Left] = 1;
			io.MouseDown[0] = true;
			} break;
		case WM_LBUTTONUP:
			{
			impl->current_mouse[Button::Left] = 0;
			io.MouseDown[0] = false;
			} break;
		case WM_RBUTTONDOWN:
			{
			impl->current_mouse[Button::Right] = 1;
			io.MouseDown[1] = true;
			} break;
		case WM_RBUTTONUP:
			{
			impl->current_mouse[Button::Right] = 0;
			io.MouseDown[1] = false;
			} break;
		case WM_CHAR:
			{
			if (w_param > 0 && w_param < 0x10000)
				io.AddInputCharacter((unsigned short)w_param);
			} break;
		case WM_MOUSEWHEEL:
			{
			io.MouseWheel = GET_WHEEL_DELTA_WPARAM(w_param) > 0 ? 1.f : -1.f;
			} break;
		default:
			{
			return false;
			} break;
		}

		return true;
	}

	void InputManager::Update()
	{
		//detect triggers on keyboard
		for (unsigned i = 0; i < 256; ++i)
		{
			//if we have a trigger in here, it must be left over from last frame
			if (impl->current_keys[i] == 2)
				impl->current_keys[i] = 1;

			else if (impl->current_keys[i] == 1 && impl->prev_keys[i] == 0)
				impl->current_keys[i] = 2;

			impl->prev_keys[i] = impl->current_keys[i];
		}

		//detect triggers on mouse
		for (unsigned i = 0; i < 2; ++i)
		{
			if (impl->current_mouse[i] == 2)
				impl->current_mouse[i] = 1;

			else if (impl->current_mouse[i] == 1 && impl->prev_mouse[i] == 0)
				impl->current_mouse[i] = 2;

			impl->prev_mouse[i] = impl->current_mouse[i];
		}

		POINT pos;
		GetCursorPos(&pos);
		ScreenToClient(impl->window.GetWindowHandle(), &pos);

		ImGui::GetIO().MousePos.x = static_cast<float>(pos.x);
		ImGui::GetIO().MousePos.y = static_cast<float>(pos.y);

		impl->mouse_pos.x = static_cast<float>(pos.x) / static_cast<float>(impl->window.GetWindowWidth()) * 2.f - 1.f;
		impl->mouse_pos.y = -(static_cast<float>(pos.y) / static_cast<float>(impl->window.GetWindowHeight()) * 2.f - 1.f);
	}

	bool InputManager::isKeyPressed(unsigned char key)
	{
		if (ImGui::GetIO().WantCaptureKeyboard)
			return false;

		return impl->current_keys[key] > 0;
	}

	bool InputManager::isKeyTriggered(unsigned char key)
	{
		if (ImGui::GetIO().WantCaptureKeyboard)
			return false;

		return impl->current_keys[key] == 2;
	}
	
	bool InputManager::isMousePressed(Button::Button button)
	{
		if (ImGui::GetIO().WantCaptureMouse)
			return false;

		return impl->current_mouse[button] > 0;
	}

	bool InputManager::isMouseTriggered(Button::Button button)
	{
		if (ImGui::GetIO().WantCaptureMouse)
			return false;

		return impl->current_mouse[button] == 2;
	}

	glm::vec2 InputManager::GetMousePos()
	{
		return impl->mouse_pos;
	}
}
