////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Drawable.h"

namespace CS350
{
	Drawable::Drawable(const Transformation& owner_trans, 
					   const std::shared_ptr<ShaderProgram>& shader,
					   const std::shared_ptr<Geometry>& geometry)
		: shader(shader), geometry(geometry), owner_world_trans_(owner_trans), color(1)
	{}

	void Drawable::SetTransformationUniforms(const glm::mat4& view, const glm::mat4& projection)
	{
		glm::mat4 model_view = view * owner_world_trans_.GetMatrix();
		glm::mat4 MVP = projection * model_view;

		shader->SetUniform("NormalMat", glm::transpose(glm::inverse(model_view)));
		shader->SetUniform("ModelView", model_view);
		shader->SetUniform("MVP", MVP);
	}

	void Drawable::Draw(const glm::mat4& view, const glm::mat4& projection)
	{
		SetTransformationUniforms(view, projection);

		shader->SetUniform("Material.F_0", color);

		auto unbind = shader->Bind();
		geometry->Draw();
	}
}
