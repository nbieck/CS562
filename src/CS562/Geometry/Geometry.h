////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_GEOMETRY_H_
#define CS350_GEOMETRY_H_

#include "../GLWrapper/Buffer.h"
#include "../GLWrapper/VertexArray.h"

#include <glm/glm.hpp>

#include <vector>
#include <memory>

namespace CS562
{
	struct Vertex
	{
		using Position = glm::vec3;
		using Normal = glm::vec3;

		Position pos;
		Normal normal;
	};

	class Geometry
	{
	public:

		Geometry(const std::vector<Vertex>& vertices, 
				 const std::vector<unsigned>& indices, 
				 PrimitiveTypes::PrimitiveTypes type);

		//this assumes that shaders have been bound and uniforms set correctly
		void Draw();

		const std::vector<Vertex> vertices;
		const std::vector<unsigned> indices;

	private:

		PrimitiveTypes::PrimitiveTypes type_;
		unsigned num_indices_;

		std::shared_ptr<Buffer<unsigned>> idx_buffer_;
		std::shared_ptr<Buffer<Vertex>> vtx_buffer_;

		std::unique_ptr<VertexArray> vao_;
	};
}

#endif
