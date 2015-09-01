#include "KDTreeNode.h"

namespace CS350
{
	void KDTreeNode::Draw(const glm::mat4& view, const glm::mat4& projection, std::shared_ptr<ShaderProgram> shader, int level)
	{
		if (level < 0)
		{
			if (is_leaf)
			{
				aabb.Draw(view, projection, shader, glm::vec4(0,1,0,1));
			}
		}
		else if (level == 0)
		{
			aabb.Draw(view, projection, shader, glm::vec4(0,1,0,1));
			return;
		}

		if (left)
			left->Draw(view, projection, shader, level - 1);
		if (right)
			right->Draw(view, projection, shader, level - 1);
	}
}
