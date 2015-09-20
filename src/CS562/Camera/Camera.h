////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_CAMERA_H_
#define CS350_CAMERA_H_

#include "../Transformation/Transformation.h"

#include <glm/glm.hpp>

namespace CS562
{
	class Camera
	{
	public:

		Camera(const Transformation& owner_trans, float near_plane, float far_plane, float fov, float aspect_ratio);

		float near_plane;
		float far_plane;
		float fov;
		float aspect_ratio;

		glm::mat4 GetViewMatrix() const;
		glm::mat4 GetProjectionMatrix() const;

	//private:

		const Transformation& owner_world_trans_;
	};
}

#endif
