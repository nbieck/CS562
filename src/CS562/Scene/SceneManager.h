////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_SCENE_MANAGER_H_
#define CS350_SCENE_MANAGER_H_

#include "Object.h"

#include "../Graphics/GraphicsManager.h"
#include "../FrameTimer/FrameTimer.h"

#include <memory>

namespace CS562
{
	class SceneManager
	{
	public:

		SceneManager(GraphicsManager& gfx, FrameTimer& time);

		void InitializeScene();

		std::shared_ptr<Object> GetSceneRoot();

		void Update();

		std::vector<std::shared_ptr<Object>> GetObjectList();

	private:

		std::shared_ptr<Object> AddObject(const glm::vec3 position, std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Geometry> geometry, std::shared_ptr<Material> material = nullptr);
		void CompObjListRec(std::shared_ptr<Object> node, std::vector<std::shared_ptr<Object>>& list);

		std::shared_ptr<Object> light_obj;

		GraphicsManager& gfx_;
		FrameTimer& time_;
		std::shared_ptr<Object> scene_root_;
	};
}

#endif
