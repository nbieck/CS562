////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "ResourceLoader.h"

#include <glm/gtc/epsilon.hpp>

#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>

namespace CS562
{
	std::shared_ptr<Shader> ResourceLoader::LoadShaderFromFile(const char* filename, ShaderType::Type type)
	{
		//method of reading entire file into a string taken from here:
		// http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
		std::ifstream in(filename);

		if (!in.is_open())
			return nullptr;

		std::string str;

		in.seekg(0, std::ios::end);
		str.reserve(static_cast<unsigned>(in.tellg()));
		in.seekg(0, std::ios::beg);

		str.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

		std::shared_ptr<Shader> shader = std::make_shared<Shader>(type);
		shader->AppendSourceCode(str.c_str());
		if (!shader->Compile())
		{
			std::cerr << "Error compiling shader " << filename << ":" << std::endl
				<< shader->GetErrorString() << std::endl;
			return nullptr;
		}

		return shader;
	}

	std::shared_ptr<ShaderProgram> ResourceLoader::LoadShaderProgramFromFile(const char* filename)
	{
		//the file format for specifying shader programs is incredibly simple:
		// each line starts with one of:
		// f -> fragment shader
		// v -> vertex shader
		// g -> geometry shader
		// followed by the filepath for that shader stage

		std::ifstream in(filename);
		if (!in.is_open())
			return nullptr;

		std::shared_ptr<ShaderProgram> prog = std::make_shared<ShaderProgram>();

		while (!in.eof())
		{
			std::string line;

			std::getline(in, line);

			if (line.empty())
				continue;

			std::istringstream linestream(line);

			ShaderType::Type shader_type;

			char identifier;
			linestream >> identifier;
			switch (identifier)
			{
			case 'f':
				shader_type = ShaderType::Fragment;
				break;
			case 'v':
				shader_type = ShaderType::Vertex;
				break;
			case 'g':
				shader_type = ShaderType::Geometry;
				break;
			default:
				continue;
			}

			std::string shader_path;
			linestream >> shader_path;

			std::shared_ptr<Shader> shader = LoadShaderFromFile(shader_path.c_str(), shader_type);
			prog->AddShader(shader);
		}

		if (!prog->Link())
		{
			std::cerr << "Error linking shader program " << filename << ":" << std::endl
				<< prog->GetErrorString() << std::endl;
			return nullptr;
		}

		return prog;
	}
}
