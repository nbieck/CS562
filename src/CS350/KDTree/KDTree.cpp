#include "KDTree.h"

#include "../Math/GeometryTests.h"

#include <algorithm>

namespace CS350
{
	namespace
	{
		const int max_rec_level = 30;
	}

	KDTree::KDTree(std::shared_ptr<Geometry> mesh)
	{
		glm::vec3 min, max;

		AABB::GetExtents(mesh->vertices, glm::mat4(), min, max);

		tree_root_ = CreateTree(mesh->vertices, mesh->indices, AABB(min, max));
	}

	void KDTree::Draw(const glm::mat4& view, const glm::mat4& projection, std::shared_ptr<ShaderProgram> shader, int level)
	{
		if (tree_root_)
			tree_root_->Draw(view, projection, shader, level);
	}

	std::shared_ptr<KDTreeNode> KDTree::CreateTree(const std::vector<Vertex>& vertices,
		const std::vector<unsigned>& indices, AABB bounding_box, int rec_level) const
	{
		if (indices.size() == 0)
			return nullptr;

		auto root = std::make_shared<KDTreeNode>();

		//find minimum plane
		std::vector<unsigned> idx_left, idx_right;
		DividingPlane plane = FindMinimumPlane(vertices, indices, bounding_box, idx_left, idx_right);

		//check terminating condition
		if (rec_level >= max_rec_level || plane.cost >= indices.size() / 3)
		{
			root->aabb = std::move(bounding_box);
			root->is_leaf = true;

			return root;
		}

		//construct child AABBs
		glm::vec3 min, max;
		bounding_box.GetMinMax(min, max);

		glm::vec3 max_left, min_right;
		max_left = max;
		min_right = min;

		max_left[plane.axis] = min_right[plane.axis] = plane.position;

		//recurse
		root->left = CreateTree(vertices, idx_left, AABB(min, max_left), rec_level + 1);
		root->right = CreateTree(vertices, idx_right, AABB(min_right, max), rec_level + 1);

		root->aabb = std::move(bounding_box);
		root->is_leaf = false;
		return root;
	}
	
	KDTree::DividingPlane KDTree::FindMinimumPlane(const std::vector<Vertex>& vertices,
		const std::vector<unsigned>& indices, const AABB& bounding_box, 
			std::vector<unsigned>& idx_left, std::vector<unsigned>& idx_right) const
	{
		DividingPlane min_plane{ 0.f, std::numeric_limits<float>::max(), X };

		std::vector<unsigned> idx_left_lcl, idx_right_lcl;

		for (Axis i : {X, Y, Z})
		{
			EventList events = CreateEventList(i, vertices, indices, bounding_box);
			size_t num_left = 0;
			size_t num_right = indices.size() / 3;
			size_t num_coplanar = 0;

			idx_left_lcl.clear();
			idx_right_lcl = indices;

			for (auto ev = events.begin(); ev != events.end();)
			{
				DividingPlane curr_plane{ ev->pos, std::numeric_limits<float>::max(), i };

				size_t num_ending = 0;
				size_t num_starting = 0;
				num_coplanar = 0;

				std::vector<unsigned> idx_starting, idx_ending, idx_coplanar;

				//get all trangle events for current plane
				while (ev != events.end() && ev->pos == curr_plane.position)
				{
					switch (ev->type)
					{
					case EventType::Ending:
						idx_ending.push_back(ev->idx0);
						idx_ending.push_back(ev->idx1);
						idx_ending.push_back(ev->idx2);
						num_ending++;
						break;
					case EventType::Starting:
						idx_starting.push_back(ev->idx0);
						idx_starting.push_back(ev->idx1);
						idx_starting.push_back(ev->idx2);
						num_starting++;
						break;
					case EventType::Coplanar:
						idx_coplanar.push_back(ev->idx0);
						idx_coplanar.push_back(ev->idx1);
						idx_coplanar.push_back(ev->idx2);
						num_coplanar++;
						break;
					}

					ev++;
				}

				num_right = num_right - num_coplanar - num_ending;
				for (unsigned j = 0; j < num_coplanar; ++j)
				{
					auto start = std::search(idx_right_lcl.begin(), idx_right_lcl.end(),
						idx_coplanar.begin() + (3 * j), idx_coplanar.begin() + (3 * (j + 1)));
					idx_right_lcl.erase(start, start + 3);
				}
				for (unsigned j = 0; j < num_ending; ++j)
				{
					auto start = std::search(idx_right_lcl.begin(), idx_right_lcl.end(),
						idx_ending.begin() + (3 * j), idx_ending.begin() + (3 * (j + 1)));
					idx_right_lcl.erase(start, start + 3);
				}

				if (curr_plane.position != (bounding_box.min())[i] && curr_plane.position != (bounding_box.max())[i])
				{				//plane comparison
					ComputePlaneCost(curr_plane, num_left, num_right, num_coplanar, bounding_box);
					if (curr_plane.cost < min_plane.cost)
					{
						min_plane = curr_plane;
						idx_left = idx_left_lcl;
						idx_right = idx_right_lcl;
					}
				}

				num_left = num_left + num_coplanar + num_starting;
				idx_left_lcl.insert(idx_left_lcl.end(), idx_coplanar.begin(), idx_coplanar.end());
				idx_left_lcl.insert(idx_left_lcl.end(), idx_starting.begin(), idx_starting.end());
			}
		}

		return min_plane;
	}

	void KDTree::ComputePlaneCost(DividingPlane& plane, size_t num_left,
		size_t num_right, size_t num_coplanar, const AABB& bounding_box) const
	{
		float parent_surface = bounding_box.SurfaceArea();

		glm::vec3 min, max;
		bounding_box.GetMinMax(min, max);

		float width_l, width_r;
		float height_l, height_r;
		float depth_l, depth_r;

		width_l = width_r = max.x - min.x;
		height_l = height_r = max.y - min.y;
		depth_l = depth_r = max.z - min.z;

		switch (plane.axis)
		{
		case X:
			width_l = plane.position - min.x;
			width_r = max.x - plane.position;
			break;
		case Y:
			height_l = plane.position - min.y;
			height_r = max.y - plane.position;
			break;
		case Z:
			depth_l = plane.position - min.z;
			depth_r = max.z - plane.position;
			break;
		}

		float left_surface, right_surface;
		left_surface = 2 * (width_l * height_l + width_l * depth_l + height_l * depth_l);
		right_surface = 2 * (width_r * height_r + width_r * depth_r + height_r * depth_r);

		const float k = 0.1f;

		float factor = 1.f;// (num_left * num_right == 0) ? k : 1.f;

		plane.cost = factor *
			std::min((left_surface * (num_left + num_coplanar) + right_surface * num_right) / parent_surface,
					(left_surface * num_left + right_surface * (num_right + num_coplanar)) / parent_surface);
	}
	
	KDTree::EventList KDTree::CreateEventList(Axis axis, const std::vector<Vertex>& vertices,
		const std::vector<unsigned>& indices, const AABB& bounding_box) const
	{
		glm::vec3 axis_vec(0);
		axis_vec[axis] = 1.f;

		EventList result;
		
		glm::vec3 box_min, box_max;
		bounding_box.GetMinMax(box_min, box_max);

		size_t num_triangles = indices.size() / 3;
		for (size_t i = 0; i < num_triangles; ++i)
		{
			glm::vec3 v0, v1, v2;
			unsigned idx0, idx1, idx2;
			idx0 = indices[3 * i];
			idx1 = indices[3 * i + 1];
			idx2 = indices[3 * i + 2];

			v0 = vertices[idx0].pos;
			v1 = vertices[idx1].pos;
			v2 = vertices[idx2].pos;

			if (Tests::TrianglePerpendicularVector(axis_vec, v0, v1, v2))
			{
				result.push_back({EventType::Coplanar, idx0, idx1, idx2, v0[axis]});
			}
			else
			{
				float min_coord, max_coord;
				min_coord = max_coord = v0[axis];
				if (v1[axis] < min_coord)
					min_coord = v1[axis];
				if (v1[axis] > max_coord)
					max_coord = v1[axis];
				if (v2[axis] < min_coord)
					min_coord = v2[axis];
				if (v2[axis] > max_coord)
					max_coord = v2[axis];

				//clip to containing bounding box
				if (min_coord < box_min[axis])
					min_coord = box_min[axis];
				if (max_coord > box_max[axis])
					max_coord = box_max[axis];

				if (min_coord == max_coord)
				{
					result.push_back({EventType::Coplanar, idx0, idx1, idx2, min_coord});
					continue;
				}

				result.push_back({ EventType::Starting, idx0, idx1, idx2, min_coord });
				result.push_back({ EventType::Ending, idx0, idx1, idx2, max_coord });
			}
		}

		std::sort(result.begin(), result.end(), 
			[](const Event& ev1, const Event& ev2)
			{
				if (ev1.pos != ev2.pos)
					return ev1.pos < ev2.pos;

				return ev1.type < ev2.type;
			});

		return result;
	}
}
