#ifndef CS350_GEOMETRY_TESTS_H_
#define CS350_GEOMETRY_TESTS_H_

#include <glm/glm.hpp>

namespace CS350
{
	namespace Tests
	{
		/*!
			Takes a point and a plane and computes whether the point is on the positive or negative halfspace

			\param p
				The point
			
			\param plane_point
				The point used to describe a point+normal plane

			\param normal
				The normal of the plane

			\retval 1
				point is in the positive halfspace

			\retval -1
				point is in the negative halfspace

			\retval 0
				point is on the plane
		*/
		int PointToPlane(const glm::vec3& p, const glm::vec3& plane_point, const glm::vec3& normal);

		bool AABBvsAABB(const glm::vec3& min1, const glm::vec3& max1, 
						const glm::vec3& min2, const glm::vec3& max2);

		bool AABBvsTriangle(const glm::vec3& min, const glm::vec3& max,
							const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);

		bool AABBvsPlane(const glm::vec3& min, const glm::vec3& max,
						 const glm::vec3& plane_pos, const glm::vec3& normal);

		bool AABBInFrontOfPlane(const glm::vec3& min, const glm::vec3& max, 
								const glm::vec3& plane_pos, const glm::vec3& normal);

		bool TrianglePerpendicularVector(const glm::vec3& vector, const glm::vec3& v0, 
			const glm::vec3& v1, const glm::vec3& v2);
	}
}

#endif
