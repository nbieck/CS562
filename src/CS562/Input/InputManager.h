////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_INPUT_MANAGER_H_
#define CS350_INPUT_MANAGER_H_

#include "../Window/WindowManager.h"

#include <glm/glm.hpp>

#include <memory>

namespace CS350
{
	namespace Button
	{
		enum Button
		{
			Left = 0,
			Right,
		};
	}

#ifdef _M_X64
	using WParam = uint64_t;
	using LParam = int64_t;
#else
	using WParam = unsigned;
	using LParam = long;
#endif

	class InputManager
	{
	public:

		InputManager(WindowManager& window);
		~InputManager();

		//internal, don't call
		bool HandleMessage(unsigned message, WParam w_param, LParam l_param);

		void Update();

		bool isKeyPressed(unsigned char key);
		bool isKeyTriggered(unsigned char key);

		bool isMousePressed(Button::Button button);
		bool isMouseTriggered(Button::Button button);

		//coordinates in normalized space (both coordinates go from -1 to 1)
		glm::vec2 GetMousePos();

	private:

		struct PImpl;
		std::unique_ptr<PImpl> impl;
	};
}

#endif
