#ifndef CS350_BV_HIERARCHY_NODE_H_
#define CS350_BV_HIERARCHY_NODE_H_

#include "AABB.h"
#include "../Scene/Object.h"

#include <memory>

namespace CS350
{
	class BVHierarchyNode
	{
	public:
		AABB bounding_volume;

		std::shared_ptr<BVHierarchyNode> left;
		std::shared_ptr<BVHierarchyNode> right;

		std::weak_ptr<BVHierarchyNode> parent;

		//only present in leaf nodes
		std::weak_ptr<Object> object;

		int GetHeight();

		//The level denotes which level of the tree should be drawn (-1 -> draw all of them)
		void Draw(const glm::mat4& view, const glm::mat4& projection, std::shared_ptr<ShaderProgram> shader, int level = -1);
	};
}

#endif
