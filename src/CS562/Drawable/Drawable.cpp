////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Drawable.h"

namespace CS562
{
	Drawable::Drawable(const Transformation& owner_trans, 
					   const std::shared_ptr<ShaderProgram>& shader,
					   const std::shared_ptr<Geometry>& geometry)
		: shader(shader), geometry(geometry), owner_world_trans_(owner_trans)
	{}

	void Drawable::SetTransformationUniforms(const glm::mat4& view, const glm::mat4& projection)
	{
		glm::mat4 model = owner_world_trans_.GetMatrix();
		glm::mat4 MVP = projection * view * model;

		shader->SetUniform("NormalMat", glm::transpose(glm::inverse(model)));
		shader->SetUniform("Model", model);
		shader->SetUniform("MVP", MVP);
	}

	void Drawable::Draw(const glm::mat4& view, const glm::mat4& projection)
	{
		SetTransformationUniforms(view, projection);

		if (material)
			material->SetUniforms(shader);

		auto unbind = shader->Bind();
		geometry->Draw();

		if (material)
			material->Cleanup();
	}
}
