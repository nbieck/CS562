////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_BOUNDING_VOLUME_MANAGER_H_
#define CS350_BOUNDING_VOLUME_MANAGER_H_

#include "../Scene/Object.h"
#include "IBoundingVolume.h"
#include "../GLWrapper/ShaderProgram.h"
#include "BVHierarchyNode.h"
#include "../Octtree/Octtree.h"
#include "../Collision/NarrowPhase.h"

#include <glm/glm.hpp>

namespace CS350
{
	class GraphicsManager;

	namespace BVComputationType
	{
		enum Type
		{
			AABB = 0,
			Centroid,
			Ritters,
			Iterative
		};
	}

	namespace HierarchyType
	{
		enum Type
		{
			TopDown = 0,
			BottomUp,
			Insertion
		};
	}

	namespace BVDrawMode
	{
		enum Mode
		{
			DrawVolumes = 0,
			DrawHierarchy
		};
	}

	class BoundingVolumeManager
	{
	public:

		BoundingVolumeManager(GraphicsManager& gfx);

		void RecomputeBoundingVolumes(BVComputationType::Type type, 
									  const std::vector<std::shared_ptr<Object>>& objects);
		void ComputeHierarchy(HierarchyType::Type type, 
							  const std::vector<std::shared_ptr<Object>>& objects);

		void ComputeTree(const std::vector<std::shared_ptr<Object>>& objects);

		void Update();
		void Draw(const glm::mat4& view, const glm::mat4& projection, const Frustum& frustum);

		void SetHierarchyDrawLevel(int level);
		
		void InsertObject(std::shared_ptr<Object> obj);
		void RemoveObject(std::shared_ptr<Object> obj);

		void SetDrawHierarchy(bool draw);
		void SetActiveObject(int active_obj);

		void SetRenderTree(bool render);
		void SetDrawBoundingVolumes(bool render);

		void GetSceneExtents(glm::vec3 &min, glm::vec3& max);

		void CheckCollisions(std::shared_ptr<Object> dynamic_obj);

	private:

		GraphicsManager& gfx;

		enum SplittingAxis
		{
			X,
			Y,
			Z
		};

		void SplitOnAxis(SplittingAxis axis, 
						 const glm::vec3& point, 
						 const std::vector<std::shared_ptr<Object>>& input, 
						 std::vector<std::shared_ptr<Object>>& left_side, 
						 std::vector<std::shared_ptr<Object>>& right_side);

		using NodeIt = std::list<std::shared_ptr<BVHierarchyNode>>::const_iterator;
		using NodeItPair = std::pair < NodeIt, NodeIt > ;

		NodeItPair FindNodesToMerge(const std::list<std::shared_ptr<BVHierarchyNode>>& nodes);
		float SurfaceHeuristic(std::shared_ptr<BVHierarchyNode> it1, std::shared_ptr<BVHierarchyNode> it2);

		void RemoveNode(std::shared_ptr<BVHierarchyNode> node);
		void BalanceTree(std::shared_ptr<BVHierarchyNode>& node);

		std::shared_ptr<BVHierarchyNode> ComputeHierarchyTopDown(std::vector<std::shared_ptr<Object>> objects);
		std::shared_ptr<BVHierarchyNode> ComputeHierarchyBottomUp(std::vector<std::shared_ptr<Object>> objects);
		void ComputeHierarchyInsertion(std::vector<std::shared_ptr<Object>> objects);

		std::vector<Contact> MiddlePhase(const std::vector<Contact>& from_broad_phase);

		std::shared_ptr<BVHierarchyNode> hierarchy_root_;

		std::vector<std::shared_ptr<IBoundingVolume>> bounding_volumes_;
		std::shared_ptr<ShaderProgram> bv_shader_;

		Octtree octtree;

		bool render_tree_;
		bool render_bounding_volumes_;

		std::vector<std::shared_ptr<Object>> prev_colored;
		
		SeparatingAxisTest test;
	};
}

#endif
