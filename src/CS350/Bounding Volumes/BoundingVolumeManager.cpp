////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "BoundingVolumeManager.h"

#include "../ResourceLoader/ResourceLoader.h"
#include "AABB.h"
#include "BoundingSphere.h"
#include "../Math/GeometryTests.h"
#include "../Math/MathUtils.h"
#include "../Collision/NarrowPhase.h"
#include "../Graphics/GraphicsManager.h"

#include <algorithm>
#include <numeric>
#include <queue>

namespace CS350
{
	BoundingVolumeManager::BoundingVolumeManager(GraphicsManager& gfx)
		: render_tree_(false), render_bounding_volumes_(false), gfx(gfx)
	{
		bv_shader_ = ResourceLoader::LoadShaderProgramFromFile("shaders/bv.shader");
	}

	void BoundingVolumeManager::Update()
	{
		for (auto &bv : bounding_volumes_)
			bv->Update();
	}

	void BoundingVolumeManager::Draw(const glm::mat4& view, const glm::mat4& projection, const Frustum& frustum)
	{
		if (render_tree_)
			octtree.DrawBounds(view, projection, bv_shader_, -1, frustum);

		if (render_bounding_volumes_)
			for (auto &bv : bounding_volumes_)
				bv->Draw(view, projection, bv_shader_, glm::vec4(0,0,1,1));
	}

	void BoundingVolumeManager::RecomputeBoundingVolumes(
		BVComputationType::Type type,
		const std::vector<std::shared_ptr<Object>>& objects)
	{
		bounding_volumes_.clear();

		for (auto obj : objects)
		{
			if (obj->drawable)
			{
				std::shared_ptr<AABB> bv;
				switch (type)
				{
				case BVComputationType::AABB:
					bv = std::make_shared<AABB>(obj->drawable->geometry, obj->GetGlobalTrans());;
					break;
					/*
				case BVComputationType::Centroid:
				bv = BoundingSphere::CentroidMethod(obj->drawable->geometry, obj->GetGlobalTrans(), bv_shader_);
				break;
				case BVComputationType::Ritters:
				bv = BoundingSphere::Ritters(obj->drawable->geometry, obj->GetGlobalTrans(), bv_shader_);
				break;
				case BVComputationType::Iterative:
				bv = BoundingSphere::IterativeRefinement(obj->drawable->geometry, obj->GetGlobalTrans(), bv_shader_, 50);
				break;
				*/
				}
				bounding_volumes_.push_back(bv);
				obj->bounding_volume = bv;
			}
		}
	}
	
	void BoundingVolumeManager::ComputeTree(const std::vector<std::shared_ptr<Object>>& objects)
	{
		octtree = Octtree(objects);
	}

	void BoundingVolumeManager::ComputeHierarchy(
		HierarchyType::Type type,
		const std::vector<std::shared_ptr<Object>>& objects)
	{
		//filter out non-graphical objects, they don't have bounding volumes either
		std::vector<std::shared_ptr<Object>> relevant_objects;
		std::copy_if(objects.begin(), objects.end(), std::back_inserter(relevant_objects), [](std::shared_ptr<Object> p){return p->drawable != nullptr; });

		switch (type)
		{
		case HierarchyType::TopDown:
			hierarchy_root_ = ComputeHierarchyTopDown(relevant_objects);
			break;
		case HierarchyType::BottomUp:
			hierarchy_root_ = ComputeHierarchyBottomUp(relevant_objects);
			break;
		case HierarchyType::Insertion:
			hierarchy_root_.reset();
			ComputeHierarchyInsertion(relevant_objects);
			break;
		}
	}

	std::shared_ptr<BVHierarchyNode> BoundingVolumeManager::ComputeHierarchyTopDown(
		std::vector<std::shared_ptr<Object>> objects)
	{
		if (objects.empty())
			return nullptr;

		std::shared_ptr<BVHierarchyNode> root = std::make_shared<BVHierarchyNode>();

		if (objects.size() == 1)
		{
			std::shared_ptr<Object> obj = objects.front();

			//this should only run after individual bounding volumes have been computed, so we should have a bounding volume
			assert(!obj->bounding_volume.expired());

			auto obj_bv = obj->bounding_volume.lock();

			glm::vec3 min, max;
			obj_bv->GetMinMax(min, max);

			root->object = obj;
			root->bounding_volume = AABB(min, max);
			return root;
		}

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

		glm::vec3 dims = scene_max - scene_min;
		SplittingAxis axis = X;
		if (dims.y > dims.x && dims.y > dims.z)
			axis = Y;
		else if (dims.z > dims.x && dims.z > dims.y)
			axis = Z;

		std::sort(objects.begin(), objects.end(),
			[&](std::shared_ptr<Object> o1, std::shared_ptr<Object> o2)
		{
			return o1->GetGlobalTrans().position[axis] < o2->GetGlobalTrans().position[axis];
		});

		glm::vec3 split_point;
		//if we have an even number of objects, pick point in the midle between the two central objects
		size_t size = objects.size();
		if (size % 2 == 0)
		{
			split_point = objects[size / 2]->GetGlobalTrans().position;
			split_point += objects[size / 2 - 1]->GetGlobalTrans().position;
			split_point /= 2;
		}
		//otherwise, just pick the middle element (median)
		else
		{
			//this is integer division
			split_point = objects[size / 2]->GetGlobalTrans().position;
		}

		std::vector<std::shared_ptr<Object>> left, right;

		//make sub lists and split
		SplitOnAxis(axis, split_point, objects, left, right);

		//fill root data
		root->bounding_volume = AABB(scene_min, scene_max);
		root->left = ComputeHierarchyTopDown(left);
		if (root->left)
			root->left->parent = root;
		root->right = ComputeHierarchyTopDown(right);
		if (root->right)
			root->right->parent = root;

		return root;
	}

	void BoundingVolumeManager::SplitOnAxis(
		SplittingAxis axis,
		const glm::vec3& point,
		const std::vector<std::shared_ptr<Object>>& input,
		std::vector<std::shared_ptr<Object>>& left_side,
		std::vector<std::shared_ptr<Object>>& right_side)
	{
		assert(left_side.empty());
		assert(right_side.empty());

		glm::vec3 normal;
		normal[axis] = 1;

		//just classify via the position of the object
		for (auto o : input)
		{
			int result = Tests::PointToPlane(o->GetGlobalTrans().position, point, normal);
			if (result > 0)
				right_side.push_back(o);
			else if (result < 0)
				left_side.push_back(o);
			else //tie-break randomly
			{
				if (rand() % 2 == 0)
					left_side.push_back(o);
				else
					right_side.push_back(o);
			}
		}
	}

	std::shared_ptr<BVHierarchyNode> BoundingVolumeManager::ComputeHierarchyBottomUp(
		std::vector<std::shared_ptr<Object>> objects)
	{
		if (objects.empty())
			return nullptr;

		std::list<std::shared_ptr<BVHierarchyNode>> nodes;

		for (auto o : objects)
		{
			//this should only run after individual bounding volumes have been computed, so we should have a bounding volume
			assert(!o->bounding_volume.expired());

			std::shared_ptr<BVHierarchyNode> node = std::make_shared<BVHierarchyNode>();

			auto obj_bv = o->bounding_volume.lock();

			glm::vec3 min, max;
			obj_bv->GetMinMax(min, max);

			node->object = o;
			node->bounding_volume = AABB(min, max);

			nodes.push_back(node);
		}

		while (nodes.size() > 1)
		{
			//find to nodes to merge
			NodeItPair to_merge = FindNodesToMerge(nodes);

			//create new node with old two as children
			std::shared_ptr<BVHierarchyNode> merged_node = std::make_shared<BVHierarchyNode>();
			merged_node->left = *(to_merge.first);
			merged_node->left->parent = merged_node;
			merged_node->right = *(to_merge.second);
			merged_node->right->parent = merged_node;

			//determine the extents of the bounding box around both objects
			glm::vec3 min1, max1, min2, max2;
			merged_node->left->bounding_volume.GetMinMax(min1, max1);
			merged_node->right->bounding_volume.GetMinMax(min2, max2);

			glm::vec3 min, max;

			Utils::ComputeEncompassingBox(min1, max1, min2, max2, min, max);

			merged_node->bounding_volume = AABB(min, max);

			//remove the two merged nodes from the list
			nodes.erase(to_merge.first);
			nodes.erase(to_merge.second);

			//push back the newly created node
			nodes.push_back(merged_node);
		}

		return nodes.front();
	}

	BoundingVolumeManager::NodeItPair BoundingVolumeManager::FindNodesToMerge(
		const std::list<std::shared_ptr<BVHierarchyNode>>& nodes)
	{
		//should not be called otherwise
		assert(nodes.size() >= 2);

		//initialize to first two elements of the list
		NodeItPair best = std::make_pair(nodes.begin(), nodes.begin());
		best.second++;
		float best_val = SurfaceHeuristic(*best.first, *best.second);

		for (auto it1 = nodes.begin(); it1 != nodes.end(); ++it1)
		{
			auto it2 = it1;
			//we don't want to check elemets against themselves
			it2++;
			for (; it2 != nodes.end(); ++it2)
			{
				float curr_val = SurfaceHeuristic(*it1, *it2);
				if (curr_val < best_val)
				{
					best_val = curr_val;
					best.first = it1;
					best.second = it2;
				}
			}
		}

		return best;
	}

	float BoundingVolumeManager::SurfaceHeuristic(std::shared_ptr<BVHierarchyNode> it1, std::shared_ptr<BVHierarchyNode> it2)
	{
		glm::vec3 min1, max1, min2, max2;
		(it1)->bounding_volume.GetMinMax(min1, max1);
		(it2)->bounding_volume.GetMinMax(min2, max2);

		glm::vec3 min, max;

		Utils::ComputeEncompassingBox(min1, max1, min2, max2, min, max);

		glm::vec3 dims = max - min;

		//no need to multiply by two, since we are just interested in relative values
		return dims.x * dims.y + dims.x * dims.z + dims.y * dims.z;
	}

	void BoundingVolumeManager::InsertObject(std::shared_ptr<Object> obj)
	{
		assert(!obj->bounding_volume.expired());

		std::shared_ptr<BVHierarchyNode> node = std::make_shared<BVHierarchyNode>();
		node->object = obj;
		glm::vec3 node_min, node_max;
		obj->bounding_volume.lock()->GetMinMax(node_min, node_max);
		node->bounding_volume = AABB(node_min, node_max);

		if (!hierarchy_root_)
		{
			hierarchy_root_ = node;
			return;
		}

		std::shared_ptr<BVHierarchyNode> curr_node = hierarchy_root_;

		while (curr_node->left)
		{
			//if the left is non-null, the right should be as well
			assert(curr_node->right);

			float heuristic_left, heuristic_right;
			heuristic_left = SurfaceHeuristic(curr_node->left, node);
			heuristic_right = SurfaceHeuristic(curr_node->right, node);

			//grow box of current node
			glm::vec3 curr_min, curr_max;
			curr_node->bounding_volume.GetMinMax(curr_min, curr_max);
			glm::vec3 new_min, new_max;

			Utils::ComputeEncompassingBox(node_min, node_max,
				curr_min, curr_max,
				new_min, new_max);

			curr_node->bounding_volume = AABB(new_min, new_max);

			if (heuristic_left < heuristic_right)
				curr_node = curr_node->left;
			else
				curr_node = curr_node->right;
		}

		//we have reached a root node
		std::shared_ptr<BVHierarchyNode> new_parent = std::make_shared<BVHierarchyNode>();
		new_parent->right = node;
		node->parent = new_parent;
		new_parent->left = curr_node;
		new_parent->parent = curr_node->parent;
		curr_node->parent = new_parent;
		if (!new_parent->parent.expired())
		{
			auto grandparent = new_parent->parent.lock();
			if (grandparent->left == curr_node)
				grandparent->left = new_parent;
			else
				grandparent->right = new_parent;
		}
		else
		{
			//there is no grandparent, so the node was the root
			hierarchy_root_ = new_parent;
		}

		//correctly calculate bounding box
		glm::vec3 curr_min, curr_max;
		curr_node->bounding_volume.GetMinMax(curr_min, curr_max);
		glm::vec3 new_min, new_max;

		Utils::ComputeEncompassingBox(node_min, node_max,
			curr_min, curr_max,
			new_min, new_max);

		new_parent->bounding_volume = AABB(new_min, new_max);


		//go back up the tree and balance
		std::shared_ptr<BVHierarchyNode> go_up = new_parent;
		while (!go_up->parent.expired())
		{
			auto parent = go_up->parent.lock();
			//do it like this to also correctly update the left/right pointer
			if (parent->left == go_up)
				BalanceTree(parent->left);
			else
				BalanceTree(parent->right);
			go_up = parent;
		}

		BalanceTree(hierarchy_root_);

	}

	void BoundingVolumeManager::ComputeHierarchyInsertion(std::vector<std::shared_ptr<Object>> objects)
	{
		for (auto o : objects)
		{
			InsertObject(o);
		}
	}

	void BoundingVolumeManager::BalanceTree(std::shared_ptr<BVHierarchyNode>& node)
	{
		// we should always have both children, so just check for one
		if (!node->left)
			return;

		assert(node->left);
		assert(node->right);

		int height_left, height_right;
		height_left = node->left->GetHeight();
		height_right = node->right->GetHeight();

		if (height_left > height_right + 1)
		{
			auto left = node->left;
			auto left_right = left->right;

			//rotate
			left->right = node;
			node->left = left_right;
			if (left_right)
				left_right->parent = node;
			left->parent = node->parent;
			node->parent = left;

			auto prev_node = node;
			node = left;

			//rotation done, need to recompute volumes
			//we can just move the top bounding volume over, it contains the same objects
			glm::vec3 min1, max1, min2, max2;
			glm::vec3 min, max;
			prev_node->right->bounding_volume.GetMinMax(min1, max1);
			prev_node->left->bounding_volume.GetMinMax(min2, max2);
			Utils::ComputeEncompassingBox(min1, max1, min2, max2, min, max);
			prev_node->bounding_volume = AABB(min, max);

			node->right->bounding_volume.GetMinMax(min1, max1);
			node->left->bounding_volume.GetMinMax(min2, max2);
			Utils::ComputeEncompassingBox(min1, max1, min2, max2, min, max);
			node->bounding_volume = AABB(min, max);

		}
		else if (height_right > height_left + 1)
		{
			auto right = node->right;
			auto right_left = right->left;

			//rotate
			right->left = node;
			node->right = right_left;
			if (right_left)
				right_left->parent = node;
			right->parent = node->parent;
			node->parent = right;

			auto prev_node = node;
			node = right;

			glm::vec3 min1, max1, min2, max2;
			glm::vec3 min, max;
			prev_node->right->bounding_volume.GetMinMax(min1, max1);
			prev_node->left->bounding_volume.GetMinMax(min2, max2);
			Utils::ComputeEncompassingBox(min1, max1, min2, max2, min, max);
			prev_node->bounding_volume = AABB(min, max);

			node->right->bounding_volume.GetMinMax(min1, max1);
			node->left->bounding_volume.GetMinMax(min2, max2);
			Utils::ComputeEncompassingBox(min1, max1, min2, max2, min, max);
			node->bounding_volume = AABB(min, max);
		}
	}

	void BoundingVolumeManager::RemoveNode(std::shared_ptr<BVHierarchyNode> node)
	{
		if (node == hierarchy_root_)
		{
			hierarchy_root_ = nullptr;
			return;
		}

		//we are not the root, so we have to have a parent
		assert(!node->parent.expired());
		auto parent = node->parent.lock();
		std::shared_ptr<BVHierarchyNode> grandparent = parent->parent.lock();
		std::shared_ptr<BVHierarchyNode> sibling;
		if (parent->left == node)
			sibling = parent->right;
		else
			sibling = parent->left;

		if (grandparent)
		{
			if (grandparent->left == parent)
				grandparent->left = sibling;
			else
				grandparent->right = sibling;
		}
		else
			hierarchy_root_ = sibling;

		sibling->parent = grandparent;

		//recompute bounding volumes starting with the grandparent
		std::shared_ptr<BVHierarchyNode> recomp_bv = grandparent;
		while (recomp_bv)
		{
			auto left = recomp_bv->left;
			auto right = recomp_bv->right;

			glm::vec3 min1, max1, min2, max2;
			left->bounding_volume.GetMinMax(min1, max1);
			right->bounding_volume.GetMinMax(min2, max2);

			glm::vec3 min, max;

			Utils::ComputeEncompassingBox(min1, max1, min2, max2, min, max);

			recomp_bv->bounding_volume = AABB(min, max);

			recomp_bv = recomp_bv->parent.lock();
		}


		//go back up the tree and balance
		if (grandparent)
		{
			std::shared_ptr<BVHierarchyNode> go_up = grandparent;
			while (!go_up->parent.expired())
			{
				auto parent = go_up->parent.lock();
				//do it like this to also correctly update the left/right pointer
				if (parent->left == go_up)
					BalanceTree(parent->left);
				else
					BalanceTree(parent->right);
				go_up = parent;
			}

			BalanceTree(hierarchy_root_);
		}

	}

	void BoundingVolumeManager::RemoveObject(std::shared_ptr<Object> obj)
	{
		assert(!obj->bounding_volume.expired());

		if (!hierarchy_root_)
			return;


		glm::vec3 obj_min, obj_max;
		obj->bounding_volume.lock()->GetMinMax(obj_min, obj_max);

		//if we are not within the top level of the tree, we are not IN the tree
		glm::vec3 node_min, node_max;
		hierarchy_root_->bounding_volume.GetMinMax(node_min, node_max);
		if (!Tests::AABBvsAABB(obj_min, obj_max, node_min, node_max))
			return;

		std::queue<std::shared_ptr<BVHierarchyNode>> nodes;
		nodes.push(hierarchy_root_);

		while (!nodes.empty())
		{
			auto node = nodes.front();
			nodes.pop();

			if (node->left)
			{
				assert(node->left);
				assert(node->right);

				node->left->bounding_volume.GetMinMax(node_min, node_max);
				if (Tests::AABBvsAABB(obj_min, obj_max, node_min, node_max))
					nodes.push(node->left);

				node->right->bounding_volume.GetMinMax(node_min, node_max);
				if (Tests::AABBvsAABB(obj_min, obj_max, node_min, node_max))
					nodes.push(node->right);
			}
			else
			{
				//leaf node
				assert(!node->object.expired());

				if (node->object.lock() == obj)
				{
					RemoveNode(node);
					return;
				}
			}
		}
	}

	void BoundingVolumeManager::SetHierarchyDrawLevel(int level)
	{
	}

	void BoundingVolumeManager::SetDrawHierarchy(bool draw)
	{
	}

	void BoundingVolumeManager::SetActiveObject(int active_obj)
	{
	}

	void BoundingVolumeManager::SetRenderTree(bool render)
	{
		render_tree_ = render;
	}

	void BoundingVolumeManager::SetDrawBoundingVolumes(bool render)
	{
		render_bounding_volumes_ = render;
	}
	
	void BoundingVolumeManager::GetSceneExtents(glm::vec3 &min, glm::vec3& max)
	{
		octtree.GetTopLevelExtents(min, max);
	}
	
	void BoundingVolumeManager::CheckCollisions(std::shared_ptr<Object> dynamic_obj)
	{
		for (auto o : prev_colored)
			o->drawable->color = glm::vec4(1);

		prev_colored.clear();

		auto broad_contacts = octtree.CheckCollision(dynamic_obj);

		std::sort(broad_contacts.begin(), broad_contacts.end(), 
			[](Contact c1, Contact c2)
			{
				return c1.obj2 < c2.obj2;
			});

		broad_contacts.erase(std::unique(broad_contacts.begin(), broad_contacts.end(), 
			[](Contact c1, Contact c2)
			{
				return c1.obj1 == c2.obj1 && c1.obj2 == c2.obj2;
			}),
			broad_contacts.end());

		for (auto c : broad_contacts)
		{
			c.obj2->drawable->color = glm::vec4(0, 1, 0, 1);
			prev_colored.push_back(c.obj2);
		}

		auto middle_contacts = MiddlePhase(broad_contacts);

		for (auto c : middle_contacts)
		{
			if (test(c.obj1, c.obj2))
			{
				c.obj2->drawable->color = glm::vec4(1, 0, 0, 1);
			}
			else
			{
				auto sep_plane = test.GetLastSeparatingPlane();

				gfx.DrawPlane(sep_plane.p, sep_plane.n);

				c.obj2->drawable->color = glm::vec4(1, 0.5, 0, 1);
			}
		}
	}
	
	std::vector<Contact> BoundingVolumeManager::MiddlePhase(const std::vector<Contact>& from_broad_phase)
	{
		std::vector<Contact> output;

		for (auto c : from_broad_phase)
		{
			glm::vec3 obj1_min, obj1_max, obj2_min, obj2_max;

			c.obj1->bounding_volume.lock()->GetMinMax(obj1_min, obj1_max);
			c.obj2->bounding_volume.lock()->GetMinMax(obj2_min, obj2_max);

			if (Tests::AABBvsAABB(obj1_min, obj1_max, obj2_min, obj2_max))
				output.push_back(c);
		}

		return output;
	}
}
