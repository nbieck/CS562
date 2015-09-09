////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Light.h"

namespace CS562
{
	Light::Light(const Transformation& owner_trans)
		: owner_world_trans_(owner_trans)
	{}

	void Light::SetUniforms(const std::shared_ptr<ShaderProgram>& prog, const glm::mat4& view)
	{
	}
}