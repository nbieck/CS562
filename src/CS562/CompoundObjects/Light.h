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
	namespace LightType
	{
		enum type
		{
			Point = 0,
			Spot
		};
	}

	//simple pointlight
	class Light
	{
	public:
		Light(const Transformation& owner_trans);

		void SetUniforms(const std::shared_ptr<ShaderProgram>& prog);

		glm::vec3 color;
		float intensity;
		float max_distance;

		float inner_cos;
		float outer_cos;

		LightType::type light_type;
		bool cast_shadow;
	//private:

		const Transformation& owner_world_trans_;
	};
}

#endif
