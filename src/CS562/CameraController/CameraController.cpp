////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "CameraController.h"

#include "../CompoundObjects/Light.h"

#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm> //min

#include <iostream>

namespace CS562
{
	CameraController::CameraController(GraphicsManager& gfx, SceneManager& scene, InputManager& input, FrameTimer& time)
		: gfx_(gfx), scene_(scene), input_(input), time_(time), rotating_(false)
	{}

	void CameraController::Init(unsigned width, unsigned height)
	{
		Transformation cam_trans;
		cam_trans.position.z = 5;
		cam_obj_ = std::make_shared<Object>(cam_trans);
		scene_.GetSceneRoot()->AddChild(cam_obj_);
		
		camera_ = std::make_shared<Camera>(cam_obj_->GetGlobalTrans(), 10.f, 800.f, glm::radians(50.f), (static_cast<float>(width) / static_cast<float>(height)));
		cam_obj_->cam = camera_;

		gfx_.current_cam = camera_;
	}

	void CameraController::Update()
	{
		CardinalMovement();
		Rotation();
	}

	void CameraController::CardinalMovement()
	{
		//magic number, adjust this later to incorporate frame time
		const float speed = 60.f;

		Transformation t = cam_obj_->GetLocalTrans();

		//get the forward and right vectors of the camera correctly rotated
		glm::vec3 forward = glm::vec3(t.GetMatrix() * glm::vec4(0, 0, -1, 0));
		glm::vec3 right = glm::vec3(t.GetMatrix() * glm::vec4(1, 0, 0, 0));

		//we always want to move in the plane
		forward.y = 0;
		forward = glm::normalize(forward);
		right.y = 0;
		right = glm::normalize(right);

		//move up with q, down with e,
		//standard wasd movement otherwise
		if (input_.isKeyPressed('Q'))
			t.position.y += speed * static_cast<float>(time_.dt());
		if (input_.isKeyPressed('E'))
			t.position.y -= speed * static_cast<float>(time_.dt());
		if (input_.isKeyPressed('W'))
			t.position += forward * speed * static_cast<float>(time_.dt());
		if (input_.isKeyPressed('S'))
			t.position -= forward * speed * static_cast<float>(time_.dt());
		if (input_.isKeyPressed('D'))
			t.position += right * speed * static_cast<float>(time_.dt());
		if (input_.isKeyPressed('A'))
			t.position -= right * speed * static_cast<float>(time_.dt());

		cam_obj_->SetLocalTrans(t);
	}

	void CameraController::Rotation()
	{
		const float target_dist = 5.f;
		const float epsilon = 0.0001f;

		if (input_.isMouseTriggered(Button::Left))
		{
			rotating_ = true;
			curr_point_ = prev_point_ = GetPointOnSphere(input_.GetMousePos());
		}

		if (rotating_)
		{
			if (!input_.isMousePressed(Button::Left))
				rotating_ = false;

			curr_point_ = GetPointOnSphere(input_.GetMousePos());

			if (glm::any(glm::epsilonNotEqual(curr_point_, prev_point_, epsilon)))
			{
				float angle = std::acos(std::min(1.f, glm::dot(curr_point_, prev_point_)));
				glm::vec3 axis = glm::normalize(glm::cross(curr_point_, prev_point_));
				//convert to world space
				axis = glm::vec3(glm::inverse(camera_->GetViewMatrix()) * glm::vec4(axis, 0.f));

				Transformation global = cam_obj_->GetGlobalTrans();

				glm::vec4 z_axis = global.GetMatrix() * glm::vec4(0, 0, 1, 0);

				glm::vec4 target = glm::vec4(global.position,1) - z_axis * target_dist;

				glm::vec4 pos(target_dist * z_axis);
				pos.w = 1.f;

				glm::mat4 rotate = glm::rotate(glm::mat4(), angle, axis);

				glm::vec4 new_pos = rotate * pos;

				Transformation local = cam_obj_->GetLocalTrans();
				local.position = glm::vec3(new_pos + target);
				local.LookAt(glm::vec3(target));

				cam_obj_->SetLocalTrans(local);
			}

			prev_point_ = curr_point_;
		}
	}

	glm::vec3 CameraController::GetPointOnSphere(const glm::vec2& screen_point) const
	{
		glm::vec3 point_on_sphere(screen_point, 0.0f);

		float dist_square = point_on_sphere.x * point_on_sphere.x + point_on_sphere.y * point_on_sphere.y;

		//if this is less than one we are on the sphere and need to compute the correct z value
		if (dist_square <= 1.f)
			point_on_sphere.z = std::sqrt(1.f - dist_square);
		else
			point_on_sphere = glm::normalize(point_on_sphere);

		return point_on_sphere;
	}
}
