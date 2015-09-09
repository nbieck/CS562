////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_CAMERA_CONTROLLER_H_
#define CS350_CAMERA_CONTROLLER_H_

#include "../Graphics/GraphicsManager.h"
#include "../Scene/Object.h"
#include "../Scene/SceneManager.h"
#include "../Input/InputManager.h"
#include "../FrameTimer/FrameTimer.h"

namespace CS562
{
	class CameraController
	{
	public:

		CameraController(GraphicsManager& gfx, SceneManager& scene, InputManager& input, FrameTimer& time);

		void Init(unsigned width, unsigned height);

		void Update();

	private:

		void CardinalMovement();
		void Rotation();

		glm::vec3 GetPointOnSphere(const glm::vec2 &screen_point) const;

		GraphicsManager& gfx_; 
		SceneManager& scene_;
		InputManager& input_;
		FrameTimer& time_;
		std::shared_ptr<Object> cam_obj_;
		std::shared_ptr<Camera> camera_;

		bool rotating_;
		glm::vec3 curr_point_, prev_point_;
	};
}

#endif
