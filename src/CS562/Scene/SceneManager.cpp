////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
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

namespace CS562
{
	SceneManager::SceneManager(GraphicsManager& gfx, FrameTimer& time)
		: gfx_(gfx), time_(time), scene_root_(std::make_shared<Object>())
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