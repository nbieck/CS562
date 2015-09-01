#include "Octtree.h"

#include "../Math/GeometryTests.h"

#include <utility>
#include <numeric>
#include <algorithm>

namespace CS350
{

	Octtree::Octtree()
	{}
	
	Octtree::Octtree(const std::vector<std::shared_ptr<Object>>& objects)
	{
		//go with selecting the major axis of the surrounding bounding volume for our axis
		glm::vec3 scene_min(std::numeric_limits<float>::max());
		glm::vec3 scene_max(-std::numeric_limits<float>::max());
		for (auto o : objects)
		{
			//this should only run after individual bounding volumes have been computed, so we should have a bounding volume
			assert(!o->bounding_volume.expired());

			auto obj_bv = o->bounding_volume.lock();
			glm::vec3 obj_min, obj_max;
			obj_bv->GetMinMax(obj_min, obj_max);

			if (obj_min.x < scene_min.x)
				scene_min.x = obj_min.x;
			if (obj_min.y < scene_min.y)
				scene_min.y = obj_min.y;
			if (obj_min.z < scene_min.z)
				scene_min.z = obj_min.z;

			if (obj_max.x > scene_max.x)
				scene_max.x = obj_max.x;
			if (obj_max.y > scene_max.y)
				scene_max.y = obj_max.y;
			if (obj_max.z > scene_max.z)
				scene_max.z = obj_max.z;
		}

		tree_root_ = ConstructTreeRec(objects, 3, AABB(scene_min, scene_max));
	}

	std::shared_ptr<OctTreeNode> Octtree::ConstructTreeRec(
		const std::vector<std::shared_ptr<Object>>& objects,
		unsigned levels,
		AABB&& bounding_box)
	{
		std::shared_ptr<OctTreeNode> root = std::make_shared<OctTreeNode>();

		if (levels == 0 || objects.size() == 1)
		{
			root->is_leaf = true;
			root->objects = objects;
		}
		else
		{
			glm::vec3 min, max;
			bounding_box.GetMinMax(min, max);

			glm::vec3 center, extents;
			center = (min + max) / 2.f;
			extents = (max - min) / 2.f;

			glm::vec3 new_extents = extents / 2.f;

			for (unsigned i = 0; i < 8; ++i)
			{
				glm::vec3 offset;
				offset.x = ((i & 1) ? new_extents.x : -new_extents.x);
				offset.y = ((i & 2) ? new_extents.y : -new_extents.y);
				offset.z = ((i & 4) ? new_extents.z : -new_extents.z);

				glm::vec3 new_center = center + offset;
				glm::vec3 new_min, new_max;

				new_min = new_center - new_extents;
				new_max = new_center + new_extents;

				std::vector<std::shared_ptr<Object>> child_objs;

				for (auto obj : objects)
				{
					if (obj->bounding_volume.expired())
						continue;

					auto obj_aabb = obj->bounding_volume.lock();
					glm::vec3 obj_min, obj_max;
					obj_aabb->GetMinMax(obj_min, obj_max);
					if (Tests::AABBvsAABB(new_min, new_max, obj_min, obj_max))
					{
						child_objs.push_back(obj);
					}
				}

				if (!child_objs.empty())
				{
					root->children[i] = ConstructTreeRec(child_objs, levels - 1, AABB(new_min, new_max));
				}
			}
			root->is_leaf = false;
		}

		root->aabb = std::move(bounding_box);

		return root;
	}

	void Octtree::DrawBounds(const glm::mat4& view, const glm::mat4& projection, std::shared_ptr<ShaderProgram> shader, int level, const Frustum& frustum)
	{
		if (tree_root_)
			tree_root_->DrawBounds(view, projection, shader, level, frustum);
	}
	
	void Octtree::GetTopLevelExtents(glm::vec3& min, glm::vec3& max)
	{
		if (tree_root_)
			tree_root_->aabb.GetMinMax(min, max);
	}
	
	std::vector<Contact> Octtree::CheckCollision(std::shared_ptr<Object> dynamic)
	{
		if (tree_root_)
			return tree_root_->CheckCollision(dynamic);

		return{};
	}
}
