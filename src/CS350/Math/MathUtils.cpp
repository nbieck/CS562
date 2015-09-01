#include "MathUtils.h"

namespace CS350
{
	namespace Utils
	{
		void ComputeEncompassingBox(const glm::vec3& min1, const glm::vec3& max1, const glm::vec3& min2, const glm::vec3& max2, glm::vec3& min, glm::vec3& max)
		{
			glm::bvec3 min_less, max_greater;
			min_less = glm::lessThanEqual(min1, min2);
			max_greater = glm::greaterThanEqual(max1, max2);
			
			for (int i = 0; i < 3; ++i)
			{
				min[i] = (min_less[i]) ? min1[i] : min2[i];
				max[i] = (max_greater[i]) ? max1[i] : max2[i];
			}
		}
	}
}
