////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_I_BOUNDING_VOLUME_H_
#define CS350_I_BOUNDING_VOLUME_H_

#include "../GLWrapper/ShaderProgram.h"

#include <glm/glm.hpp>

#include <memory>

namespace CS350
{
	class IBoundingVolume
	{
	public:

		virtual void Update() = 0;
		virtual void Draw(const glm::mat4& view, const glm::mat4& projection, std::shared_ptr<ShaderProgram> shader, const glm::vec4& color) = 0;
		virtual void GetMinMax(glm::vec3& min, glm::vec3& max) const = 0;
	};
}

#endif
