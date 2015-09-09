////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Transformation.h"

#include <glm/gtc/matrix_transform.hpp>	//translate, scale, rotate
#include <glm/gtx/matrix_interpolation.hpp> //axisAngle

namespace CS562
{
	Transformation::Transformation()
		: position(0, 0, 0), scale(1, 1, 1), axis(1, 0, 0), angle(0.f)
	{}

	glm::mat4 Transformation::GetMatrix() const
	{
		glm::mat4 trans_mat, scale_mat, rot_mat;

		trans_mat = glm::translate(glm::mat4(), position);
		scale_mat = glm::scale(glm::mat4(), scale);
		rot_mat = glm::rotate(glm::mat4(), angle, glm::normalize(axis));

		return trans_mat * rot_mat * scale_mat;
	}

	Transformation Transformation::operator*(const Transformation& rhs) const
	{
		Transformation result;

		result.scale = scale * rhs.scale;

		glm::vec4 temp_trans = GetMatrix() * glm::vec4(rhs.position, 1);
		result.position = glm::vec3(temp_trans.x, temp_trans.y, temp_trans.z);

		glm::mat4 combined_rotation = glm::rotate(glm::mat4(), rhs.angle, rhs.axis);
		combined_rotation = glm::rotate(glm::mat4(), angle, axis) * combined_rotation;

		glm::axisAngle(combined_rotation, result.axis, result.angle);

		return result;
	}

	void Transformation::LookAt(const glm::vec3 &target, const glm::vec3 &up)
	{
		glm::mat4 matrix = glm::lookAt(position, target, up);
		matrix = glm::inverse(matrix);
		glm::axisAngle(matrix, axis, angle);
	}
}
