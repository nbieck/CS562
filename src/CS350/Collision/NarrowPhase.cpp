#include "NarrowPhase.h"

#include <glm/gtc/epsilon.hpp>

#include <algorithm>

namespace CS350
{
	namespace
	{
		const float epsilon = 0.00001f;

		struct VecLess
		{
			bool operator()(glm::vec3 v1, glm::vec3 v2)
			{
				glm::bvec3 nequal = glm::epsilonNotEqual(v1, v2, epsilon);

				if (nequal.x)
					return v1.x < v2.x;
				if (nequal.y)
					return v1.y < v2.y;
				if (nequal.z)
					return v1.z < v2.z;

				return false;
			}
		};

		struct VecEq
		{
			bool operator()(glm::vec3 v1, glm::vec3 v2)
			{
				return glm::all(glm::epsilonEqual(v1, v2, epsilon));
			}
		};
	}

	size_t SeparatingAxisTest::HashObjPair::operator()(const ObjectPair& pair)
	{
		std::hash<Object*> hasher;

		size_t hash = hasher(pair.first.get()) ^ hasher(pair.second.get());

		return hash;
	}

	bool SeparatingAxisTest::EquObjPair::operator()(const ObjectPair& left, const ObjectPair& right)
	{
		return left.first == right.first && left.second == right.second;
	}

	bool SeparatingAxisTest::operator()(std::shared_ptr<Object> obj1, std::shared_ptr<Object> obj2)
	{
		std::shared_ptr<Geometry> geom1 = obj1->drawable->geometry;
		std::shared_ptr<Geometry> geom2 = obj2->drawable->geometry;

		Transformation trans1 = obj1->GetGlobalTrans();
		Transformation trans2 = obj2->GetGlobalTrans();

		glm::mat4 mat1 = trans1.GetMatrix();
		glm::mat4 mat2 = trans2.GetMatrix();

		std::vector<glm::vec3> world_positions1(geom1->vertices.size());
		std::transform(geom1->vertices.begin(), geom1->vertices.end(), world_positions1.begin(), 
			[&](const Vertex& vtx)
			{
				return glm::vec3(mat1 * glm::vec4(vtx.pos, 1));
			});
		
		std::vector<glm::vec3> world_positions2(geom2->vertices.size());
		std::transform(geom2->vertices.begin(), geom2->vertices.end(), world_positions2.begin(), 
			[&](const Vertex& vtx)
			{
				return glm::vec3(mat2 * glm::vec4(vtx.pos, 1));
			});

		float middle_dist;

		auto last_axis = last_separating_axis_.find(std::make_pair(obj1, obj2));
		if (last_axis != last_separating_axis_.end())
		{
			if (CheckAxis(world_positions1, world_positions2, last_axis->second, middle_dist))
			{
				CompSepPlane(trans1.position, last_axis->second, middle_dist);
				return false;
			}
		}

		auto normals1 = ExtractNormals(world_positions1, geom1->indices);
		//test with normals 1
		size_t num_normals = normals1.size();
		for (size_t i = 0; i < num_normals; ++i)
		{
			if (CheckAxis(world_positions1, world_positions2, normals1[i], middle_dist))
			{
				CompSepPlane(trans1.position, normals1[i], middle_dist);
				last_separating_axis_[std::make_pair(obj1, obj2)] = normals1[i];
				return false;
			}
		}

		auto normals2 = ExtractNormals(world_positions2, geom2->indices);
		//test with normals 2
		num_normals = normals2.size();
		for (size_t i = 0; i < num_normals; ++i)
		{
			if (CheckAxis(world_positions1, world_positions2, normals2[i], middle_dist))
			{
				CompSepPlane(trans1.position, normals2[i], middle_dist);
				last_separating_axis_[std::make_pair(obj1, obj2)] = normals2[i];
				return false;
			}
		}

		auto edges1 = ExtractEdges(world_positions1, geom1->indices);
		auto edges2 = ExtractEdges(world_positions2, geom2->indices);
		//CHECK ALL AXES!!!!!!!
		size_t num_edge1 = edges1.size();
		size_t num_edge2 = edges2.size();
		for (size_t i = 0; i < num_edge1; ++i)
		{
			for (size_t j = 0; j < num_edge2; ++j)
			{
				glm::vec3 m = glm::normalize(glm::cross(edges1[i].vector, edges2[j].vector));
				
				if (!IsZeroVector(m))
				{
					if (CheckAxis(world_positions1, world_positions2, m, middle_dist))
					{
						CompSepPlane(trans1.position, m, middle_dist);
						last_separating_axis_[std::make_pair(obj1, obj2)] = m;
						return false;
					}
				}
				else
				{
					m = glm::normalize(glm::cross(edges1[i].vector, edges2[j].p0 - edges1[i].p0));
					
					if (!IsZeroVector(m))
					{
						if (CheckAxis(world_positions1, world_positions2, m, middle_dist))
						{
							CompSepPlane(trans1.position, m, middle_dist);
							last_separating_axis_[std::make_pair(obj1, obj2)] = m;
							return false;
						}
					}
				}
			}
		}

		return true;
	}
	
	std::vector<glm::vec3> SeparatingAxisTest::ExtractNormals(const std::vector<glm::vec3>& positions, const std::vector<unsigned>& indices)
	{
		std::vector<glm::vec3> result;

		size_t num_triangles = indices.size() / 3;
		for (size_t i = 0; i < num_triangles; ++i)
		{
			glm::vec3 v0, v1, v2;
			v0 = positions[indices[3 * i]];
			v1 = positions[indices[3 * i + 1]];
			v2 = positions[indices[3 * i + 2]];

			glm::vec3 edge1, edge2;
			edge1 = glm::normalize(v1 - v2);
			edge2 = glm::normalize(v1 - v0);

			glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

			if (!IsZeroVector(normal))
				result.push_back(normal);
		}

		std::sort(result.begin(), result.end(), VecLess());
		result.erase(std::unique(result.begin(), result.end(), VecEq()), result.end());

		return result;
	}
	
	std::vector<SeparatingAxisTest::Edge> SeparatingAxisTest::ExtractEdges(const std::vector<glm::vec3>& positions, const std::vector<unsigned>& indices)
	{
		std::vector<Edge> result;

		size_t num_triangles = indices.size() / 3;
		for (size_t i = 0; i < num_triangles; ++i)
		{
			glm::vec3 v0, v1, v2;
			v0 = positions[indices[3 * i]];
			v1 = positions[indices[3 * i + 1]];
			v2 = positions[indices[3 * i + 2]];

			glm::vec3 edge1, edge2, edge3;
			edge1 = glm::normalize(v1 - v2);
			edge2 = glm::normalize(v1 - v0);
			edge3 = glm::normalize(v2 - v0);

			result.push_back({ edge1, v2, v1 });
			result.push_back({ edge2, v0, v1 });
			result.push_back({ edge3, v0, v2 });
		}
		
		std::sort(result.begin(), result.end(), 
			[](const Edge& e1, const Edge& e2) -> bool
			{
				VecEq eq;
				VecLess less;

				if (!eq(e1.vector, e2.vector))
					return less(e1.vector, e2.vector);

				if (!eq(e1.p0, e2.p0))
					return less(e1.p0, e2.p0);

				return less(e1.p1, e2.p1);
			});

		result.erase(std::unique(result.begin(), result.end(), 
			[](const Edge& e1, const Edge& e2) -> bool
			{
				VecEq eq;
				return eq(e1.vector, e2.vector) &&
					eq(e1.p0, e2.p0) && eq(e1.p1, e2.p1);
			}), 
			result.end());

		return result;
	}
	
	void SeparatingAxisTest::ComputeInterval(const std::vector<glm::vec3>& positions, glm::vec3 axis,
		float& min, float& max)
	{
		min = glm::dot(positions[0], axis);
		max = min;

		size_t num_pos = positions.size();
		for (size_t i = 1; i < num_pos; ++i)
		{
			float val = glm::dot(positions[i], axis);
			if (val < min)
				min = val;
			if (val > max)
				max = val;
		}
	}
	
	bool SeparatingAxisTest::IsZeroVector(glm::vec3 vec)
	{
		return glm::all(glm::epsilonEqual(vec, glm::vec3(0), epsilon));
	}
	
	bool SeparatingAxisTest::CheckAxis(const std::vector<glm::vec3>& pos1, const std::vector<glm::vec3>& pos2,
		glm::vec3 axis, float& middle_point)
	{
		float min1, max1, min2, max2;
		ComputeInterval(pos1, axis, min1, max1);
		ComputeInterval(pos2, axis, min2, max2);

		if (max1 < min2)
		{
			middle_point = (max1 + min2) * 0.5f;
			return true;
		}

		if (max2 < min1)
		{
			middle_point = (max2 + min1) * 0.5f;
			return true;
		}

		return false;
	}
	
	void SeparatingAxisTest::CompSepPlane(glm::vec3 obj_pos, glm::vec3 axis, float middle_point)
	{
		float pos_proj = glm::dot(obj_pos, axis);

		float dist = middle_point - pos_proj;

		last_sep_plane_.n = axis;
		last_sep_plane_.p = obj_pos + (dist * axis);
	}
	
	SeparatingPlane SeparatingAxisTest::GetLastSeparatingPlane() const
	{
		return last_sep_plane_;
	}
}
