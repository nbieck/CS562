#include "OctTreeNode.h"

#include "../Math/GeometryTests.h"

namespace CS350
{
	void OctTreeNode::DrawBounds(const glm::mat4& view, const glm::mat4& projection, std::shared_ptr<ShaderProgram> shader, int level, const Frustum& frustum)
	{
		if (ShouldCull(frustum))
			return;

		if (level <= 0)
			aabb.Draw(view, projection, shader, glm::vec4(0,1,0,1));

		if (level != 0)
		{
			for (unsigned i = 0; i < 8; ++i)
			{
				if (children[i])
					children[i]->DrawBounds(view, projection, shader, level - 1, frustum);
			}
		}
	}

	bool OctTreeNode::ShouldCull(const Frustum& frustum) const
	{
		glm::vec3 min, max;
		aabb.GetMinMax(min, max);

		if (!Tests::AABBInFrontOfPlane(min, max, frustum.front_point, frustum.front_norm))
			return true;
		if (!Tests::AABBInFrontOfPlane(min, max, frustum.back_point, frustum.back_norm))
			return true;
		if (!Tests::AABBInFrontOfPlane(min, max, frustum.left_point, frustum.left_norm))
			return true;
		if (!Tests::AABBInFrontOfPlane(min, max, frustum.right_point, frustum.right_norm))
			return true;
		if (!Tests::AABBInFrontOfPlane(min, max, frustum.top_point, frustum.top_norm))
			return true;
		if (!Tests::AABBInFrontOfPlane(min, max, frustum.bottom_point, frustum.bottom_norm))
			return true;

		return false;
	}
	
	std::vector<Contact> OctTreeNode::CheckCollision(std::shared_ptr<Object> dynamic)
	{
		glm::vec3 node_min, node_max, obj_min, obj_max;
		aabb.GetMinMax(node_min, node_max);

		assert(!dynamic->bounding_volume.expired());
		dynamic->bounding_volume.lock()->GetMinMax(obj_min, obj_max);

		if (Tests::AABBvsAABB(node_min, node_max, obj_min, obj_max))
		{
			std::vector<Contact> result;

			if (is_leaf)
			{
				for (auto obj : objects)
				{
					result.push_back({ dynamic, obj });
				}
			}
			else
			{
				for (unsigned i = 0; i < 8; ++i)
				{
					if (children[i])
					{
						auto child_contacts = children[i]->CheckCollision(dynamic);

						result.insert(result.end(), child_contacts.begin(), child_contacts.end());
					}
				}
			}

			return result;
		}

		return{};
	}
}
