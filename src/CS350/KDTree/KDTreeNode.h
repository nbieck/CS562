#ifndef CS350_KD_TREE_NODE_H_
#define CS350_KD_TREE_NODE_H_

#include "../Bounding Volumes/AABB.h"

#include <memory>

namespace CS350
{
	struct KDTreeNode
	{
		AABB aabb;

		bool is_leaf;

		std::shared_ptr<KDTreeNode> left;
		std::shared_ptr<KDTreeNode> right;

		//level : if -1 draws all leaf nodes, otherwise draws given level
		void Draw(const glm::mat4& view, const glm::mat4& projection, std::shared_ptr<ShaderProgram> shader, int level);
	};
}

#endif
