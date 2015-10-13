////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#include "../CompoundObjects/Geometry.h"
#include "../CompoundObjects/Drawable.h"
#include "../Camera/Camera.h"
#include "../GLWrapper/Shader.h"
#include "../GLWrapper/ShaderProgram.h"
#include "../ResourceLoader/ResourceLoader.h"

#include <vector>

#include <glm/gtc/constants.hpp>

#include <iostream>

namespace CS562
{
	SceneManager::SceneManager(GraphicsManager& gfx, FrameTimer& time)
		: gfx_(gfx), time_(time), scene_root_(std::make_shared<Object>())
	{}

	void SceneManager::InitializeScene()
	{
		auto shader = ResourceLoader::LoadShaderProgramFromFile("shaders/deferred_geometry.shader");

		std::vector<std::pair<std::shared_ptr<Geometry>, unsigned>> geom;
		std::vector<std::shared_ptr<Material>> mtl;

		ResourceLoader::LoadObjFile(geom, mtl, "sponza.obj");

		for (auto g : geom)
		{
			AddObject(glm::vec3(0), shader, g.first, mtl[g.second]);
		}

		Transformation t;
		t.position = glm::vec3(0, 40, 0);
		t.axis = glm::vec3(1, 0, 0);
		t.angle = glm::radians(-45.f);

		spot_light_ =  std::make_shared<Object>(t);
		scene_root_->AddChild(spot_light_);
		std::shared_ptr<Light> spot_light = std::make_shared<Light>(spot_light_->GetGlobalTrans());

		spot_light->cast_shadow = true;
		spot_light->color = glm::vec3(1);
		spot_light->inner_cos = glm::cos(glm::radians(45.f));
		spot_light->outer_cos = glm::cos(glm::radians(50.f));
		spot_light->intensity = 1.f;
		spot_light->max_distance = 100;
		spot_light_->light = spot_light;

		gfx_.RegisterLight(spot_light);
	}

	std::shared_ptr<Object> SceneManager::AddObject(const glm::vec3 position, std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Geometry> geometry, std::shared_ptr<Material> material)
	{
		Transformation t;
		t.position = position;
		t.scale = glm::vec3(0.1f, 0.1f, 0.1f);

		std::shared_ptr<Object> drawable_obj = std::make_shared<Object>(t);
		scene_root_->AddChild(drawable_obj);
		std::shared_ptr<Drawable> draw = std::make_shared<Drawable>(drawable_obj->GetGlobalTrans(), shader, geometry);
		draw->material = material;
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
		static float t = 0.0f;
		if (rotate_light_)
		{
			t += static_cast<float>(time_.dt());

			glm::vec3 target(30 * cos(t), 10, 30 * sin(t));

			auto tr = spot_light_->GetLocalTrans();
			tr.LookAt(target);
			spot_light_->SetLocalTrans(tr);
		}
	}

	std::vector<std::shared_ptr<Object>> SceneManager::GetObjectList()
	{
		std::vector<std::shared_ptr<Object>> list;

		CompObjListRec(scene_root_, list);

		return list;
	}

	void SceneManager::PushLight()
	{	
		Transformation t;

		std::uniform_real_distribution<float> pos_x(-150, 140);
		std::uniform_real_distribution<float> pos_y(1, 135);
		std::uniform_real_distribution<float> pos_z(-80, 70);

		std::uniform_real_distribution<float> intensity(0.1f, 1);
		std::uniform_real_distribution<float> dist(5, 15);

		std::uniform_real_distribution<float> color(0, 1);

		t.position = glm::vec3(pos_x(random_engine), pos_y(random_engine), pos_z(random_engine));
		t.scale = glm::vec3(0.1f);

		std::shared_ptr<Object> obj = std::make_shared<Object>(t);
		scene_root_->AddChild(obj);
		std::shared_ptr<Light> l = std::make_shared<Light>(obj->GetGlobalTrans());
		l->color = glm::vec3(color(random_engine), color(random_engine), color(random_engine));
		l->intensity = intensity(random_engine);
		l->max_distance = dist(random_engine);
		obj->light = l;

		gfx_.RegisterLight(l);

		lights_.push_back(obj);
	}

	void SceneManager::PopLight()
	{
		if (!lights_.empty())
		{
			auto obj = lights_.back();
			lights_.pop_back();

			scene_root_->RemoveChild(obj);
		}
	}

	void SceneManager::ToggleRotate()
	{
		rotate_light_ = !rotate_light_;
	}

	void SceneManager::CompObjListRec(std::shared_ptr<Object> node, std::vector<std::shared_ptr<Object>>& list)
	{
		list.push_back(node);

		for (auto child : node->GetChildren())
		{
			CompObjListRec(child, list);
		}
	}
}