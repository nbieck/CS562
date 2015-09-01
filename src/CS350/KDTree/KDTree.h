#ifndef CS350_KD_TREE_H_
#define CS350_KD_TREE_H_

#include "KDTreeNode.h"
#include "../Geometry/Geometry.h"

#include <memory>

namespace CS350
{
	class KDTree
	{
	public:

		KDTree() = default;

		KDTree(std::shared_ptr<Geometry> mesh);

		void Draw(const glm::mat4& view, const glm::mat4& projection, std::shared_ptr<ShaderProgram> shader, int level);

	private:
		
		enum Axis
		{
			X = 0,
			Y,
			Z,

			Num_Axes
		};

		struct DividingPlane
		{
			float position;
			float cost;
			Axis axis;
		};

		enum class EventType
		{
			Ending,
			Coplanar,
			Starting
		};

		struct Event
		{
			EventType type;
			unsigned idx0, idx1, idx2;
			float pos;
		};

		using EventList = std::vector < Event > ;

		std::shared_ptr<KDTreeNode> CreateTree(const std::vector<Vertex>& vertices, 
			const std::vector<unsigned>& indices, AABB bounding_box, int rec_level = 0) const;

		DividingPlane FindMinimumPlane(const std::vector<Vertex>& vertices,
			const std::vector<unsigned>& indices, const AABB& bounding_box, 
			std::vector<unsigned>& idx_left, std::vector<unsigned>& idx_right) const;

		EventList CreateEventList(Axis axis, const std::vector<Vertex>& vertices,
			const std::vector<unsigned>& indices, const AABB& bounding_box) const;

		void ComputePlaneCost(DividingPlane& plane, size_t num_left, 
			size_t num_right, size_t num_coplanar, const AABB& bounding_box) const;

		std::shared_ptr<KDTreeNode> tree_root_;

	};
}

#endif
