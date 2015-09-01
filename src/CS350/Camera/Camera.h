////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_CAMERA_H_
#define CS350_CAMERA_H_

#include "../Transformation/Transformation.h"
#include "../Math/Frustum.h"

#include <glm/glm.hpp>

namespace CS350
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

		Frustum GetViewFrustum() const;

	private:

		const Transformation& owner_world_trans_;
	};
}

#endif
