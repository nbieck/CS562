#ifndef CS350_OCT_TREE_H_
#define CS350_OCT_TREE_H_

#include "OctTreeNode.h"
#include "../Math/Frustum.h"
#include "../Scene/Object.h"
#include "../Collision/Contact.h"

namespace CS350
{
	class Octtree
	{
	public:

		Octtree();

		Octtree(const std::vector<std::shared_ptr<Object>>& objects);

		void DrawBounds(const glm::mat4& view, const glm::mat4& projection, std::shared_ptr<ShaderProgram> shader, int level, const Frustum& frustum);

		void GetTopLevelExtents(glm::vec3& min, glm::vec3& max);

		std::vector<Contact> CheckCollision(std::shared_ptr<Object> dynamic);

	private:

		std::shared_ptr<OctTreeNode> tree_root_;

		std::shared_ptr<OctTreeNode> ConstructTreeRec(
			const std::vector<std::shared_ptr<Object>>& objects,
			unsigned levels,
			AABB&& bounding_box);
	};
}

#endif
