#ifndef CS350_FRUSTUM_H_
#define CS350_FRUSTUM_H_

#include <glm/glm.hpp>

namespace CS350
{
	struct Frustum
	{
		glm::vec3 front_point; 	glm::vec3 front_norm;
		glm::vec3 back_point; 	glm::vec3 back_norm;
		glm::vec3 left_point; 	glm::vec3 left_norm;
		glm::vec3 right_point; 	glm::vec3 right_norm;
		glm::vec3 top_point; 	glm::vec3 top_norm;
		glm::vec3 bottom_point; glm::vec3 bottom_norm;
	};
}

#endif
