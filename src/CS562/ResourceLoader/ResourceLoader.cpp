////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "ResourceLoader.h"

#include <glm/gtc/epsilon.hpp>

#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>

namespace CS350
{
	bool operator<(const Vertex& lhs, const Vertex& rhs)
	{
		const float kEpsilon = 0.001f;

		glm::bvec3 pos_nequal = glm::epsilonNotEqual(lhs.pos, rhs.pos, kEpsilon);

		if (pos_nequal.x)
			return lhs.pos.x < rhs.pos.x;
		if (pos_nequal.y)
			return lhs.pos.y < rhs.pos.y;
		if (pos_nequal.z)
			return lhs.pos.z < rhs.pos.z;

		glm::bvec3 norm_nequal = glm::epsilonNotEqual(lhs.normal, rhs.normal, kEpsilon);

		if (norm_nequal.x)
			return lhs.normal.x < rhs.normal.x;
		if (norm_nequal.y)
			return lhs.normal.y < rhs.normal.y;
		if (norm_nequal.z)
			return lhs.normal.z < rhs.normal.z;

		return false;
	}

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

	void ResourceLoader::LoadGeometryFromFile(
		const char* filename, 
		std::vector<Vertex>& vertex_buffer, 
		std::vector<unsigned>& index_buffer)
	{
		//read a normal .obj format file
		std::ifstream in(filename);

		if (!in.is_open())
			return;

		ObjLoaderHelper helper;
		//these two are temporary to deal with the fact that .obj might use different indices per attribute
		std::vector<Vertex::Position> positions;
		std::vector<Vertex::Normal> normals;

		index_buffer.clear();

		while (!in.eof())
		{
			std::string line;
			std::getline(in, line);

			std::istringstream linestream(line);
			std::string identifier;

			linestream >> identifier;
			if (identifier.compare("v") == 0)
				LoadPosition(linestream, positions);
			else if (identifier.compare("f") == 0)
				LoadFace(linestream, helper, positions, normals, index_buffer);
			else if (identifier.compare("vn") == 0)
				LoadNormal(linestream, normals);
		}

		vertex_buffer = helper.GetVertexBuffer();
	}

	std::shared_ptr<Geometry> ResourceLoader::LoadGeometryFromFile(const char* filename)
	{

		std::vector<unsigned> index_buffer;
		std::vector<Vertex> vertex_buffer;

		LoadGeometryFromFile(filename, vertex_buffer, index_buffer);

		std::shared_ptr<Geometry> geom = std::make_shared<Geometry>(vertex_buffer, index_buffer, PrimitiveTypes::Triangles);

		return geom;
	}

	void ResourceLoader::LoadPosition(std::istringstream& line, 
									  std::vector<Vertex::Position>& positions)
	{
		Vertex::Position p;
		line >> p.x >> p.y >> p.z;
		positions.push_back(p);
	}

	void ResourceLoader::LoadNormal(std::istringstream& line, 
									std::vector<Vertex::Normal>& normals)
	{
		Vertex::Normal n;
		line >> n.x >> n.y >> n.z;
		normals.push_back(n);
	}
	
	void ResourceLoader::LoadFace(std::istringstream& line,
								  ObjLoaderHelper& helper,
								  const std::vector<Vertex::Position>& positions,
								  const std::vector<Vertex::Normal>& normals,
								  std::vector<unsigned>& index_buffer)
	{
		//this stores the indices for the current face (it might have more than three vertices)
		std::vector<unsigned> face_indices;

		while (!line.eof())
		{
			std::string block;
			line >> block;

			// if there are no more vertices, we stop
			if (block == "")
				break;

			unsigned num_slashes = static_cast<unsigned>(std::count(block.begin(), block.end(), '/'));
			
			Vertex vtx;
			int pos_idx, norm_idx;

			//if there is one slash, the format is pos/texcoord, but we are ignoring texture
			//coordinates, so we can treat them the same
			if (num_slashes <= 1)
			{ 
				std::istringstream iss(block);
				int idx;
				iss >> idx;
				pos_idx = norm_idx = idx;
			}
			else if (num_slashes == 2)
			{
				std::istringstream iss(block);
				iss >> pos_idx;
				//step over '/' character
				iss.seekg(1, std::ios::cur);
				//skip the texture coordinate index;
				iss.ignore(std::numeric_limits<std::streamsize>::max(), '/');
				iss >> norm_idx;
			}
			
			if (pos_idx > 0)
				//indices are one-based
				vtx.pos = positions[pos_idx - 1];
			else
				//negative index means from the back
				vtx.pos = positions[positions.size() + pos_idx];
			
			if (norm_idx > 0)
				//indices are one-based
				vtx.normal = normals[norm_idx - 1];
			else
				//negative index means from the back
				vtx.normal = normals[normals.size() + norm_idx];

			face_indices.push_back(helper.GetIndex(vtx));
		}

		for (unsigned i = 1; i < face_indices.size() - 1; ++i)
		{
			index_buffer.push_back(face_indices[0]);
			index_buffer.push_back(face_indices[i]);
			index_buffer.push_back(face_indices[i + 1]);
		}
	}

	ResourceLoader::ObjLoaderHelper::ObjLoaderHelper()
		: next_index_(0)
	{}

	unsigned ResourceLoader::ObjLoaderHelper::GetIndex(const Vertex& vtx)
	{
		auto it = vertex_to_index_.find(vtx);

		if (it != vertex_to_index_.end())
			return it->second;
		else
		{
			unsigned idx = next_index_++;
			vertex_buffer_.push_back(vtx);
			vertex_to_index_[vtx] = idx;
			return idx;
		}
	}

	std::vector<Vertex> ResourceLoader::ObjLoaderHelper::GetVertexBuffer()
	{
		return vertex_buffer_;
	}
}
