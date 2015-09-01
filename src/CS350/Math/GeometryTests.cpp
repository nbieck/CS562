#include "GeometryTests.h"

#include <glm/gtc/epsilon.hpp>

#include <algorithm>
#include <vector>

namespace CS350
{
	namespace Tests
	{
		namespace
		{
			const float epsilon = 0.0001f;
		}

		int PointToPlane(const glm::vec3& p, const glm::vec3& plane_point, const glm::vec3& normal)
		{
			float d = glm::dot((p - plane_point), normal);

			if (d > 0)
				return 1;
			if (d < 0)
				return -1;

			return 0;
		}
		
		bool AABBvsAABB(const glm::vec3& min1, const glm::vec3& max1,
						const glm::vec3& min2, const glm::vec3& max2)
		{
			if (min1.x > max2.x)
				return false;
			if (max1.x < min2.x)
				return false;
			if (min1.y > max2.y)
				return false; 
			if (max1.y < min2.y)
				return false;
			if (min1.z > max2.z)
				return false;
			if (max1.z < min2.z)
				return false;

			return true;
		}

		bool AABBvsTriangle(const glm::vec3& min, const glm::vec3& max,
							const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
		{
			//we work with VERY small numbers
			const float coll_epsilon = 1e-12f;

			//formulas taken fom Ericson's book
			glm::vec3 center, extents;
			center = (min + max) / 2.f;
			extents = (max - min) / 2.f;

			glm::vec3 p0,p1,p2;
			//conceptually move triangle to origin
			p0 = v0 - center;
			p1 = v1 - center;
			p2 = v2 - center;

			//find triangle edge vectors
			glm::vec3 f0 = v1 - v0, f1 = v2 - v1, f2 = v0 - v2;

			//do category 3 tests
			float proj0, proj1, proj2, r;

			//a00
			proj0 = p0.z * f0.y - p0.y * f0.z;
			proj2 = p2.z * f0.y - p2.y * f0.z;
			r = extents.y * abs(f0.z) + extents.z * abs(f0.y);
			if (std::max(-std::max(proj0, proj2), std::min(proj0, proj2)) + coll_epsilon >= r)
				return false;
			
			//a01
			proj0 = p0.z * f1.y - p0.y * f1.z;
			proj1 = p1.z * f1.y - p1.y * f1.z;
			r = extents.y * abs(f1.z) + extents.z * abs(f1.y);
			if (std::max(-std::max(proj0, proj1), std::min(proj0, proj1)) + coll_epsilon >= r)
				return false;
			
			//a02
			proj1 = p1.z * f2.y - p1.y * f2.z;
			proj2 = p2.z * f2.y - p2.y * f2.z;
			r = extents.y * abs(f2.z) + extents.z * abs(f2.y);
			if (std::max(-std::max(proj2, proj1), std::min(proj2, proj1)) + coll_epsilon >= r)
				return false;
			
			//a10
			proj0 = p0.x * f0.z - p0.z * f0.x;
			proj2 = p2.x * f0.z - p2.z * f0.x;
			r = extents.x * abs(f0.z) + extents.z * abs(f0.x);
			if (std::max(-std::max(proj0, proj2), std::min(proj0, proj2)) + coll_epsilon >= r)
				return false;
			
			//a11
			proj0 = p0.x * f1.z - p0.z * f1.x;
			proj1 = p1.x * f1.z - p1.z * f1.x;
			r = extents.x * abs(f1.z) + extents.z * abs(f1.x);
			if (std::max(-std::max(proj0, proj1), std::min(proj0, proj1)) + coll_epsilon >= r)
				return false;
			
			//a12
			proj1 = p1.x * f2.z - p1.z * f2.x;
			proj2 = p2.x * f2.z - p2.z * f2.x;
			r = extents.x * abs(f2.z) + extents.z * abs(f2.x);
			if (std::max(-std::max(proj2, proj1), std::min(proj2, proj1)) + coll_epsilon >= r)
				return false;
			
			//a20
			proj0 = -p0.x * f0.y + p0.y * f0.x;
			proj2 = -p2.x * f0.y + p2.y * f0.x;
			r = extents.x * abs(f0.y) + extents.y * abs(f0.x);
			if (std::max(-std::max(proj0, proj2), std::min(proj0, proj2)) + coll_epsilon >= r)
				return false;
			
			//a21
			proj0 = -p0.x * f1.y + p0.y * f1.x;
			proj1 = -p1.x * f1.y + p1.y * f1.x;
			r = extents.x * abs(f1.y) + extents.y * abs(f1.x);
			if (std::max(-std::max(proj0, proj1), std::min(proj0, proj1)) + coll_epsilon >= r)
				return false;
			
			//a22
			proj1 = -p1.x * f2.y + p1.y * f2.x;
			proj2 = -p2.x * f2.y + p2.y * f2.x;
			r = extents.x * abs(f2.y) + extents.y * abs(f2.x);
			if (std::max(-std::max(proj2, proj1), std::min(proj2, proj1)) + coll_epsilon >= r)
				return false;

			//test triangle AABB vs passed AABB
			if (std::max({ p0.x, p1.x, p2.x }) - coll_epsilon <= -extents.x || std::min({ p0.x, p1.x, p2.x }) + coll_epsilon >= extents.x)
				return false;
			if (std::max({ p0.y, p1.y, p2.y }) - coll_epsilon <= -extents.y || std::min({ p0.y, p1.y, p2.y }) + coll_epsilon >= extents.y)
				return false;
			if (std::max({ p0.z, p1.z, p2.z }) - coll_epsilon <= -extents.z || std::min({ p0.z, p1.z, p2.z }) + coll_epsilon >= extents.z)
				return false;

			//check along triangle normal
			glm::vec3 plane_normal = glm::normalize(glm::cross(f0, f1));
			return AABBvsPlane(-extents, extents, p0, plane_normal);
		}

		bool AABBvsPlane(const glm::vec3& min, const glm::vec3& max,
						 const glm::vec3& plane_pos, const glm::vec3& normal)
		{
			glm::vec3 center, extents;
			center = (min + max) / 2.f;
			extents = (max - min) / 2.f;

			float r = extents.x * abs(normal.x) +
					  extents.y * abs(normal.y) + 
					  extents.z * abs(normal.z);

			float s = glm::dot(center - plane_pos, normal);

			return abs(s) <= r;
		}

		bool AABBInFrontOfPlane(const glm::vec3& min, const glm::vec3& max, 
								const glm::vec3& plane_pos, const glm::vec3& normal)
		{
			std::vector<glm::vec3> corners(8);

			for (unsigned i = 0; i < 8; ++i)
			{
				corners[i].x = (i & 1) ? min.x : max.x;
				corners[i].y = (i & 2) ? min.y : max.y;
				corners[i].z = (i & 4) ? min.z : max.z;
			}

			for (unsigned i = 0; i < 8; ++i)
			{
				if (PointToPlane(corners[i], plane_pos, normal) == 1)
					return true;
			}

			return false;
		}
		
		bool TrianglePerpendicularVector(const glm::vec3& vector, const glm::vec3& v0,
			const glm::vec3& v1, const glm::vec3& v2)
		{
			glm::vec3 plane_vec0 = v1 - v0;
			glm::vec3 plane_vec1 = v2 - v0;

			return glm::epsilonEqual(glm::dot(plane_vec0, vector), 0.f, epsilon)
				&& glm::epsilonEqual(glm::dot(plane_vec1, vector), 0.f, epsilon);
		}
	}
}
