////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Material.h"

void CS562::Material::SetUniforms(std::shared_ptr<ShaderProgram> shader)
{
	if (diffuse_tex)
	{
		diffuse_tex->Bind_NoUnbind(1);

		shader->SetUniform("DiffuseTex", 1);
	}
	if (specular_tex)
	{
		specular_tex->Bind_NoUnbind(2);
		shader->SetUniform("SpecularTex", 2);
	}
	shader->SetUniform("SpecularCoefficient", shininess);
}

void CS562::Material::Cleanup()
{
	if (diffuse_tex)
	{
		diffuse_tex->Unbind();
	}
	if (specular_tex)
		specular_tex->Unbind();
}
