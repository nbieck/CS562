////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_TRANSFORMATION_H_
#define CS350_TRANSFORMATION_H_

#include <glm/glm.hpp>

namespace CS562
{
	class Transformation
	{
	public:

		Transformation();

		glm::mat4 GetMatrix() const;

		void LookAt(const glm::vec3 &target, const glm::vec3 &up = glm::vec3(0,1,0));

		Transformation operator*(const Transformation& rhs) const;

		glm::vec3 position;
		glm::vec3 scale;
		glm::vec3 axis;
		float angle;
	};
}

#endif
