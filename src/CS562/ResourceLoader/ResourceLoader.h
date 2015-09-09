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

		static void LoadGeometryFromFile(const char* filename, 
										 std::vector<Vertex>& vertex_buffer, 
										 std::vector<unsigned>& index_buffer);

		static std::shared_ptr<Geometry> LoadGeometryFromFile(const char* filename);

	private:
		
		//this class is used to help in loading .obj files
		class ObjLoaderHelper
		{
		public:

			ObjLoaderHelper();

			unsigned GetIndex(const Vertex& vtx);
			std::vector<Vertex> GetVertexBuffer();

		private:

			std::map<Vertex, unsigned> vertex_to_index_;
			std::vector<Vertex> vertex_buffer_;
			unsigned next_index_;
		};

		//these functions load individual parts of an .obj file
		static void LoadPosition(std::istringstream& line, 
								 std::vector<Vertex::Position>& positions);
		static void LoadNormal(std::istringstream& line, 
							   std::vector<Vertex::Normal>& normals);
		static void LoadFace(std::istringstream& line, 
							 ObjLoaderHelper& helper, 
							 const std::vector<Vertex::Position>& positions, 
							 const std::vector<Vertex::Normal>& normals, 
							 std::vector<unsigned>& index_buffer);

	};
}

#endif
