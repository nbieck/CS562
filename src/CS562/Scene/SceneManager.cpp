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
	}

	std::shared_ptr<Object> SceneManager::AddObject(const glm::vec3 position, std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Geometry> geometry, std::shared_ptr<Material> material)
	{
		Transformation t;
		t.position = position;
		t.scale = glm::vec3(.1f, .1f, .1f);

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
}