////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Light.h"

namespace CS350
{
	Light::Light(const Transformation& owner_trans)
		: owner_world_trans_(owner_trans)
	{}

	void Light::SetUniforms(const std::shared_ptr<ShaderProgram>& prog, const glm::mat4& view)
	{
	}
}
