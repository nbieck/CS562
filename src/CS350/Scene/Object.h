////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_OBJECT_H_
#define CS350_OBJECT_H_

#include "../Transformation/Transformation.h"
#include "../Camera/Camera.h"
#include "../Drawable/Drawable.h"
#include "../Light/Light.h"
#include "../Bounding Volumes/AABB.h"

#include <memory>
#include <list>

namespace CS350
{
	class Object : public std::enable_shared_from_this<Object>
	{
	public:

		Object(const Transformation& local = Transformation());

		void SetParent(const std::weak_ptr<Object>& parent);

		Transformation GetLocalTrans() const;
		const Transformation& GetGlobalTrans() const;
		void SetLocalTrans(const Transformation& transform);

		void AddChild(const std::shared_ptr<Object>& child);

		std::shared_ptr<Camera> cam;
		std::shared_ptr<Drawable> drawable;
		std::shared_ptr<Light> light;
		std::weak_ptr<AABB> bounding_volume;

		const std::list<std::shared_ptr<Object>>& GetChildren() const;

	private:

		void UpdateGlobal(const Transformation& parent_trans);

		Transformation local_;
		Transformation global_;

		std::weak_ptr<Object> parent_;
		std::list<std::shared_ptr<Object>> children_;
	};
}

#endif
