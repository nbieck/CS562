////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Object.h"

namespace CS562
{
	Object::Object(const Transformation& local)
		: local_(local), global_(local), parent_()
	{}

	Transformation Object::GetLocalTrans() const
	{
		return local_;
	}

	const Transformation& Object::GetGlobalTrans() const
	{
		return global_;
	}

	void Object::SetLocalTrans(const Transformation& transform)
	{
		local_ = transform;
		if (!parent_.expired())
		{
			UpdateGlobal(parent_.lock()->GetGlobalTrans());
			for (auto child : children_)
			{
				child->UpdateGlobal(global_);
			}
		}
		else
		{
			global_ = local_;
		}
	}

	void Object::AddChild(const std::shared_ptr<Object>& child)
	{
		children_.push_back(child);
		child->SetParent(shared_from_this());
	}

	void Object::RemoveChild(const std::shared_ptr<Object>& child)
	{
		auto it = std::find(children_.begin(), children_.end(), child);
		
		if (it != children_.end())
			children_.erase(it);
	}

	void Object::SetParent(const std::weak_ptr<Object>& parent)
	{
		parent_ = parent;
		if (!parent_.expired())
			UpdateGlobal(parent.lock()->GetGlobalTrans());
	}

	void Object::UpdateGlobal(const Transformation& parent_trans)
	{
		global_ = parent_trans * local_;
	}

	const std::list<std::shared_ptr<Object>>& Object::GetChildren() const
	{
		return children_;
	}
}
