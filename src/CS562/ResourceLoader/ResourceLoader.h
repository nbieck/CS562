////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_RESOURCE_LOADER_H_
#define CS350_RESOURCE_LOADER_H_

#include "../GLWrapper/Shader.h"
#include "../GLWrapper/ShaderProgram.h"
#include "../Geometry/Geometry.h"

#include <memory>
#include <map>

namespace CS562
{
	class ResourceLoader
	{
	public:

		static std::shared_ptr<Shader> LoadShaderFromFile(const char* filename, ShaderType::Type type);

		static std::shared_ptr<ShaderProgram> LoadShaderProgramFromFile(const char* filename);

	private:

	};
}

#endif
