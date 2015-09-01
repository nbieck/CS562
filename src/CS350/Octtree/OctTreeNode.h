#ifndef CS350_OCT_TREE_NODE_H_
#define CS350_OCT_TREE_NODE_H_

#include "../Bounding Volumes/AABB.h"

#include "../Math/Frustum.h"

#include "../Scene/Object.h"
#include "../Collision/Contact.h"

#include <memory>

namespace CS350
{
	class OctTreeNode
	{
	public:
		AABB aabb;

		bool is_leaf;

		std::shared_ptr<OctTreeNode> children[8];

		std::vector<std::shared_ptr<Object>> objects;

		void DrawBounds(const glm::mat4& view, const glm::mat4& projection, std::shared_ptr<ShaderProgram> shader, int level, const Frustum& frustum);
		
		std::vector<Contact> CheckCollision(std::shared_ptr<Object> dynamic);

	private:
		bool ShouldCull(const Frustum& frustum) const;
	};
}

#endif
