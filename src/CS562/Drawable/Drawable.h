////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_DRAWABLE_H_
#define CS350_DRAWABLE_H_

#include "../Transformation/Transformation.h"
#include "../GLWrapper/ShaderProgram.h"
#include "../Geometry/Geometry.h"

#include <glm/glm.hpp>

#include <memory>

namespace CS350
{
	class Drawable
	{
	public:

		Drawable(const Transformation& owner_trans, 
			const std::shared_ptr<ShaderProgram>& shader,
			const std::shared_ptr<Geometry>& geometry);

		std::shared_ptr<ShaderProgram> shader;
		std::shared_ptr<Geometry> geometry;

		glm::vec4 color;

		//set any additional uniforms besides transformation before calling this
		void Draw(const glm::mat4& view, const glm::mat4& projection);

	private:
		
		void SetTransformationUniforms(const glm::mat4& view, const glm::mat4& projection);
		const Transformation& owner_world_trans_;
	};
}

#endif
