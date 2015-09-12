////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "ResourceLoader.h"

#include "tiny_obj_loader.h"
#include "stb_image.h"
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

	std::string ResourceLoader::LoadObjFile(std::vector<std::shared_ptr<Geometry>>& geom, 
		std::vector<std::shared_ptr<Material>>& mats, std::string filename)
	{
		using namespace tinyobj;

		std::vector<material_t> tiny_obj_mats;
		std::vector<shape_t> tiny_obj_shapes;

		std::string load_result = LoadObj(tiny_obj_shapes, tiny_obj_mats, filename.c_str());

		if (load_result.empty())
		{
			//extract data from tinyobj format and put it into our geometry
			for (const auto& shape : tiny_obj_shapes)
			{
				//indices can be used directly
				//vertices need to be translated
				std::vector<Vertex> vertices;
				std::size_t num_verts = shape.mesh.positions.size() / 3;

				const std::vector<float>& pos = shape.mesh.positions;
				const std::vector<float>& normal = shape.mesh.normals;

				for (std::size_t i = 0; i < num_verts; ++i)
				{
					vertices.push_back({ glm::vec3(pos[3 * i], pos[3 * i + 1], pos[3 * i + 2]), 
						glm::vec3(normal[i * 3], normal[i * 3 + 1], normal[i * 3 + 2]) });
				}

				geom.push_back(std::make_shared<Geometry>(vertices, shape.mesh.indices, PrimitiveTypes::Triangles));
			}
		}

		return load_result;
	}

	std::shared_ptr<Texture> ResourceLoader::LoadTextureFromFile(const std::string & filename)
	{
		int x, y, components;
		unsigned char* img_data = stbi_load(filename.c_str(), &x, &y, &components, STBI_default);

		if (img_data == nullptr)
			return nullptr;

		std::shared_ptr<Texture> tex = std::make_shared<Texture>();

		TextureFormatInternal format_internal;
		TextureFormat format;
		switch (components)
		{
		case 1:
			format_internal = TextureFormatInternal::R8;
			format = TextureFormat::R;
			break;
		case 2:
			format_internal = TextureFormatInternal::RG8;
			format = TextureFormat::RG;
			break;
		case 3:
			format_internal = TextureFormatInternal::RGB8;
			format = TextureFormat::RGB;
			break;
		case 4:
		default:
			format_internal = TextureFormatInternal::RGBA8;
			format = TextureFormat::RGBA;
		}

		auto unbind = tex->Bind(0);

		tex->AllocateSpace(x, y, format_internal, ComputeMipLevels(x, y));

		tex->TransferData(0, 0, x, y, format, TextureDataType::UnsignedByte, img_data);

		stbi_image_free(img_data);

		return tex;
	}

	int ResourceLoader::ComputeMipLevels(int width, int height)
	{
		return 0;
	}
}
