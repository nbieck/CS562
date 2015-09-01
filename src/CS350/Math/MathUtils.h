#ifndef CS350_MATH_UTILS_H_
#define CS350_MATH_UTILS_H_

#include <glm/glm.hpp>

namespace CS350
{
	namespace Utils
	{
		void ComputeEncompassingBox(const glm::vec3& min1, const glm::vec3& max1, const glm::vec3& min2, const glm::vec3& max2, glm::vec3& min, glm::vec3& max);
	}
}

#endif
