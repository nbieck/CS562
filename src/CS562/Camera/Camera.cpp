////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace CS562
{
	Camera::Camera(const Transformation& owner_trans, 
				   float near_plane, 
				   float far_plane, 
				   float fov, 
				   float aspect_ratio)
		: owner_world_trans_(owner_trans), near_plane(near_plane), far_plane(far_plane), 
		 fov(fov), aspect_ratio(aspect_ratio)
	{}

	glm::mat4 Camera::GetProjectionMatrix() const
	{
		return glm::perspective(fov, aspect_ratio, near_plane, far_plane);
	}

	glm::mat4 Camera::GetViewMatrix() const
	{	
		glm::mat4 trans_matrix = owner_world_trans_.GetMatrix();

		glm::vec4 pos = glm::vec4(owner_world_trans_.position, 1);

		// get the vectors of the camera
		glm::vec4 view = glm::normalize(trans_matrix * glm::vec4(0, 0, -1, 0));
		glm::vec4 up = glm::normalize(trans_matrix * glm::vec4(0, 1, 0, 0));
		glm::vec4 right = glm::normalize(trans_matrix * glm::vec4(1, 0, 0, 0));

		glm::mat4 view_mtx(1);
		view_mtx[0][0] = right.x;
		view_mtx[1][0] = right.y;
		view_mtx[2][0] = right.z;
		view_mtx[0][1] = up.x;
		view_mtx[1][1] = up.y;
		view_mtx[2][1] = up.z;
		view_mtx[0][2] = -view.x;
		view_mtx[1][2] = -view.y;
		view_mtx[2][2] = -view.z;
		view_mtx[3][0] = -glm::dot(right, pos);
		view_mtx[3][1] = -glm::dot(up, pos);
		view_mtx[3][2] = glm::dot(view, pos);


		return view_mtx;
	}
}
