#include "BVHierarchyNode.h"

namespace CS350
{
	void BVHierarchyNode::Draw(const glm::mat4& view, const glm::mat4& projection, 
		std::shared_ptr<ShaderProgram> shader, int level)
	{
		if (level < 1)
		{
			bounding_volume.Draw(view, projection, shader, glm::vec4(1));
			

			if (level == -1)
			{
				if (left)
					left->Draw(view, projection, shader);
				if (right)
					right->Draw(view, projection, shader);
			}
		}
		else
		{
			if (left)
				left->Draw(view, projection, shader, level - 1);
			if (right)
				right->Draw(view, projection, shader, level - 1);
		}
	}

	int BVHierarchyNode::GetHeight()
	{
		int height_left = -1, height_right = -1;
		if (left)
			height_left = left->GetHeight();
		if (right)
			height_right = right->GetHeight();

		return 1 + ((height_left > height_right) ? height_left : height_right);
	}
}
