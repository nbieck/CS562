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

		num_spheres_ = 15;

		std::vector<std::pair<std::shared_ptr<Geometry>, unsigned>> geom;
		std::vector<std::shared_ptr<Material>> mtl;

		ResourceLoader::LoadObjFile(geom, mtl, "meshes/dragon.obj");

		mat = std::make_shared<Material>();
		mat->k_d = glm::vec3(1, 1, 1);
		mat->k_s = glm::vec3(0.05, 0.05, 0.05);
		mat->shininess = 50.f;

		for (auto g : geom)
		{
			AddObject(glm::vec3(0), shader, g.first, mat);
		}

		geom.clear();
		mtl.clear();
		ResourceLoader::LoadObjFile(geom, mtl, "meshes/hq_sphere.obj");

		for (unsigned i = 0; i < num_spheres_; ++i)
		{
			auto sph_mat = std::make_shared<Material>();
			*sph_mat = *mat;
			AddObject(glm::vec3(7 * glm::cos(glm::two_pi<float>() * static_cast<float>(i) / num_spheres_), 0, -7 * glm::sin(glm::two_pi<float>() * static_cast<float>(i) / num_spheres_)), shader, geom.front().first, sph_mat, glm::vec3(1.f));
			sphere_mats_.push_back(sph_mat);
		}
	}

	std::shared_ptr<Object> SceneManager::AddObject(const glm::vec3 position, std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Geometry> geometry, std::shared_ptr<Material> material, const glm::vec3& scale)
	{
		Transformation t;
		t.position = position;
		t.scale = scale;

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
		for (unsigned i = 0; i < num_spheres_; ++i)
		{
			auto& sp_mat = sphere_mats_[i];
			*sp_mat = *mat;
			sp_mat->shininess = (500.f / num_spheres_) * i + 2.f;
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

	void SceneManager::AddSpotLight(const glm::vec3 & pos, const glm::vec3 & target, float inner_angle, float outer_angle, const glm::vec3 & color, float intensity, float max_dist)
	{
		Transformation t;
		t.position = pos;
		t.LookAt(target);
		auto spot_light_ =  std::make_shared<Object>(t);
		scene_root_->AddChild(spot_light_);
		std::shared_ptr<Light> spot_light = std::make_shared<Light>(spot_light_->GetGlobalTrans());

		spot_light->cast_shadow = true;
		spot_light->color = color;
		spot_light->inner_cos = glm::cos(glm::radians(inner_angle));
		spot_light->outer_cos = glm::cos(glm::radians(outer_angle));
		spot_light->intensity = intensity;
		spot_light->max_distance = max_dist;
		spot_light_->light = spot_light;

		gfx_.RegisterLight(spot_light);
	}
}