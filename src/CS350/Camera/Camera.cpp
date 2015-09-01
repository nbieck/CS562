////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace CS350
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
		return glm::perspectiveFov(fov, aspect_ratio, 1.f, near_plane, far_plane);
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

	Frustum Camera::GetViewFrustum() const
	{
		glm::mat4 trans_matrix = owner_world_trans_.GetMatrix();
		glm::vec3 pos = owner_world_trans_.position;
		
		glm::vec4 view_4 = trans_matrix * glm::vec4(0, 0, -1, 0);
		glm::vec4 up_4 = trans_matrix * glm::vec4(0, 1, 0, 0);
		glm::vec4 right_4 = trans_matrix * glm::vec4(1, 0, 0, 0);

		glm::vec3 view = glm::normalize(glm::vec3(view_4));
		glm::vec3 up = glm::normalize(glm::vec3(up_4));
		glm::vec3 right = glm::normalize(glm::vec3(right_4));
		
		float frustum_fov = fov * 0.5f;
			
		float height_near = 2 * glm::tan(frustum_fov) * near_plane;
		float width_near = height_near * aspect_ratio;
			
		float height_far = 2 * glm::tan(frustum_fov) * far_plane;
		float width_far = height_far * aspect_ratio;

		glm::vec3 near_center = pos + (near_plane) * view;
		glm::vec3 far_center = pos + (far_plane) * view;

		glm::vec3 ntl = near_center + -(width_near / 2) * right +  (height_near / 2) * up;
		glm::vec3 ntr = near_center +  (width_near / 2) * right +  (height_near / 2) * up;
		glm::vec3 nbl = near_center + -(width_near / 2) * right + -(height_near / 2) * up;
		glm::vec3 nbr = near_center +  (width_near / 2) * right + -(height_near / 2) * up;
		glm::vec3 ftl = far_center  + -(width_far / 2)  * right +  (height_far / 2)  * up;
		glm::vec3 ftr = far_center  +  (width_far / 2)  * right +  (height_far / 2)  * up;
		glm::vec3 fbl = far_center  + -(width_far / 2)  * right + -(height_far / 2)  * up;
		glm::vec3 fbr = far_center  +  (width_far / 2)  * right + -(height_far / 2)  * up;

		return 
		{
			ntl, glm::normalize(glm::cross(nbr - ntl, nbl - ntl)),
			ftl, glm::normalize(glm::cross(fbr - ftl, ftr - ftl)),
			ftl, glm::normalize(glm::cross(nbl - ftl, fbl - ftl)),
			ntr, glm::normalize(glm::cross(fbr - ntr, nbr - ntr)),
			ntl, glm::normalize(glm::cross(ftr - ntl, ntr - ntl)),
			nbl, glm::normalize(glm::cross(fbr - nbl, fbl - nbl)),
		};
	}
}
