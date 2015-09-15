////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_LIGHT_H_
#define CS350_LIGHT_H_

#include "../Transformation/Transformation.h"
#include "../GLWrapper/ShaderProgram.h"

#include <memory>

namespace CS562
{
	//simple pointlight
	class Light
	{
	public:
		Light(const Transformation& owner_trans);

		void SetUniforms(const std::shared_ptr<ShaderProgram>& prog, const glm::mat4& view);

	private:

		const Transformation& owner_world_trans_;
	};
}

#endif
