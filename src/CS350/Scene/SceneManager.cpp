////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#include "../Geometry/Geometry.h"
#include "../Drawable/Drawable.h"
#include "../Camera/Camera.h"
#include "../GLWrapper/Shader.h"
#include "../GLWrapper/ShaderProgram.h"
#include "../ResourceLoader/ResourceLoader.h"

#include <vector>

#include <glm/gtc/constants.hpp>

namespace CS350
{
	SceneManager::SceneManager(GraphicsManager& gfx, FrameTimer& time)
		: gfx_(gfx), time_(time), scene_root_(std::make_shared<Object>()), obj_velocity(-2.3153, 3.11233, -4.1),
		simulation_paused(false), single_step(false)
	{}

	void SceneManager::InitializeScene()
	{
		auto sphere = ResourceLoader::LoadGeometryFromFile("meshes/sphere.obj");
		auto box = ResourceLoader::LoadGeometryFromFile("meshes/box.obj");
		auto cone = ResourceLoader::LoadGeometryFromFile("meshes/cone.obj");
		auto cylinder = ResourceLoader::LoadGeometryFromFile("meshes/cylinder.obj");
		auto pyramid = ResourceLoader::LoadGeometryFromFile("meshes/pyramid.obj");

		auto shader = ResourceLoader::LoadShaderProgramFromFile("shaders/light.shader");

		shader->SetUniform("Material.roughness", 0.1f);
		shader->SetUniform("Material.F_0", glm::vec4(1, 1, 1, 1));

		dynamic_obj = AddObject(glm::vec3(0), shader, sphere);
		dynamic_obj->drawable->color = glm::vec4(0, 1, 1, 1);

		AddObject(glm::vec3(4, 1, -1), shader, box);
		AddObject(glm::vec3(-2, 0, 4), shader, cone);
		AddObject(glm::vec3(2, 5, 0), shader, cone);
		AddObject(glm::vec3(7, -2.5, 3), shader, pyramid);
		AddObject(glm::vec3(8, 1.3, -6), shader, box);
		AddObject(glm::vec3(-3, 1, -3), shader, cylinder);
		AddObject(glm::vec3(9, 0, 6), shader, cone);
		AddObject(glm::vec3(-1, 3, -5), shader, box);
		AddObject(glm::vec3(5, 5, 5), shader, pyramid);
		AddObject(glm::vec3(-6, 1, 4), shader, cylinder);
		AddObject(glm::vec3(0, 4.5, 1), shader, pyramid);
		AddObject(glm::vec3(-1.5, 2.4, 6), shader, box);
	}

	std::shared_ptr<Object> SceneManager::AddObject(const glm::vec3 position, std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Geometry> geometry)
	{
		Transformation t;
		t.position = position;
		t.scale = glm::vec3(0.1, 0.1, 0.1);

		std::shared_ptr<Object> drawable_obj = std::make_shared<Object>(t);
		scene_root_->AddChild(drawable_obj);
		std::shared_ptr<Drawable> draw = std::make_shared<Drawable>(drawable_obj->GetGlobalTrans(), shader, geometry);
		drawable_obj->drawable = draw;
	
		gfx_.RegisterDrawable(draw);

		return drawable_obj;
	}

	std::shared_ptr<Object> SceneManager::GetSceneRoot()
	{
		return scene_root_;
	}

	void SceneManager::Update()
	{
		if (dynamic_obj)
		{
			if (!simulation_paused)
				AdvanceSimulation(static_cast<float>(time_.dt()));
			else if (single_step)
			{
				AdvanceSimulation(0.016f);
				single_step = false;
			}
		}
	}

	std::vector<std::shared_ptr<Object>> SceneManager::GetObjectList()
	{
		std::vector<std::shared_ptr<Object>> list;

		CompObjListRec(scene_root_, list);

		return list;
	}

	void SceneManager::CompObjListRec(std::shared_ptr<Object> node, std::vector<std::shared_ptr<Object>>& list)
	{
		list.push_back(node);

		for (auto child : node->GetChildren())
		{
			CompObjListRec(child, list);
		}
	}

	void SceneManager::SetSceneExtents(glm::vec3 min, glm::vec3 max)
	{
		scene_min = min;
		scene_max = max;
	}

	void SceneManager::ReverseSimulation()
	{
		obj_velocity = -obj_velocity;
	}

	void SceneManager::AdvanceSimulation(float dt)
	{
			auto trans = dynamic_obj->GetLocalTrans();

			if (trans.position.x > scene_max.x)
			{
				obj_velocity.x = -obj_velocity.x;
				trans.position.x = scene_max.x;
			}
			if (trans.position.x < scene_min.x)
			{
				obj_velocity.x = -obj_velocity.x;
				trans.position.x = scene_min.x;
			}
			
			if (trans.position.y > scene_max.y)
			{
				obj_velocity.y = -obj_velocity.y;
				trans.position.y = scene_max.y;
			}
			if (trans.position.y < scene_min.y)
			{
				obj_velocity.y = -obj_velocity.y;
				trans.position.y = scene_min.y;
			}
			
			if (trans.position.z > scene_max.z)
			{
				obj_velocity.z = -obj_velocity.z;
				trans.position.z = scene_max.z;
			}
			if (trans.position.z < scene_min.z)
			{
				obj_velocity.z = -obj_velocity.z;
				trans.position.z = scene_min.z;
			}

			trans.position += obj_velocity * dt;
			dynamic_obj->SetLocalTrans(trans);
	}
}
