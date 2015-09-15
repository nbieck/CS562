////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../GLWrapper/Texture.h"
#include "../GLWrapper/ShaderProgram.h"

namespace CS562
{
	class Material
	{
	public:

		std::shared_ptr<Texture> diffuse_tex;

		void SetUniforms(std::shared_ptr<ShaderProgram> shader);
		void Cleanup();
	};
}
