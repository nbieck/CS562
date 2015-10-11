////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "ResourceLoader.h"

#include "tiny_obj_loader.h"

#pragma warning(disable: 4312)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma warning(default: 4312)

#include <glm/gtc/epsilon.hpp>

#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>

#define MAX(a,b) ((a > b)?a:b)

namespace CS562
{
	std::map<std::string, std::weak_ptr<Texture>> ResourceLoader::loaded_textures_;

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
			case 'c':
				shader_type = ShaderType::Compute;
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

	std::string ResourceLoader::LoadObjFile(std::vector<std::pair<std::shared_ptr<Geometry>, unsigned>>& geom,
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
				const std::vector<float>& uv = shape.mesh.texcoords;

				for (std::size_t i = 0; i < num_verts; ++i)
				{
					vertices.push_back({ glm::vec3(pos[3 * i], pos[3 * i + 1], pos[3 * i + 2]), 
						glm::vec3(normal[i * 3], normal[i * 3 + 1], normal[i * 3 + 2]),
						glm::vec2(uv[i * 2], uv[i * 2 + 1]) });
				}

				geom.push_back(std::make_pair(std::make_shared<Geometry>(vertices, shape.mesh.indices, PrimitiveTypes::Triangles), shape.mesh.material_ids[0]));
			}

			for (const auto& material : tiny_obj_mats)
			{
				auto mat = std::make_shared<Material>();
				mat->diffuse_tex = LoadTextureFromFile(material.diffuse_texname);
				mat->specular_tex = LoadTextureFromFile(material.specular_texname);
				mat->shininess = material.shininess;

				mats.push_back(mat);
			}
		}

		return load_result;
	}

	std::shared_ptr<Texture> ResourceLoader::LoadTextureFromFile(const std::string & filename)
	{
		auto pre_loaded = loaded_textures_.find(filename);
		if (pre_loaded != loaded_textures_.end() && !pre_loaded->second.expired())
			return pre_loaded->second.lock();

		int x, y, components;
		unsigned char* img_data = stbi_load(filename.c_str(), &x, &y, &components, STBI_rgb_alpha);
		components = 4;

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

		auto unbind = tex->Bind(1);

		tex->AllocateSpace(x, y, format_internal, ComputeMipLevels(x, y));

		InvertImageVertically(x, y, components, img_data);

		tex->TransferData(0, 0, x, y, format, TextureDataType::UnsignedByte, img_data);

		tex->GenerateMipMaps();

		stbi_image_free(img_data);

		loaded_textures_[filename] = tex;

		return tex;
	}

	int ResourceLoader::ComputeMipLevels(int width, int height)
	{
		return static_cast<int>(std::floor(std::log2(MAX(width, height))) + 1);
	}
	void ResourceLoader::InvertImageVertically(int width, int height, int channels, unsigned char * const image)
	{
		for (int j = 0; j * 2 < height; ++j)
		{
			int index1 = j * width * channels;
			int index2 = (height - 1 - j) * width * channels;
			for (int i = width * channels; i > 0; --i)
			{
				unsigned char temp = image[index1];
				image[index1] = image[index2];
				image[index2] = temp;
				++index1;
				++index2;
			}
		}
	}
}
