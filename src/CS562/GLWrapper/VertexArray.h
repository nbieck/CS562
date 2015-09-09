////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_VERTEX_ARRAY_H_
#define CS350_VERTEX_ARRAY_H_

#include "Buffer.h"
#include "Unbinder.h"

#include <memory>
#include <vector>

namespace CS562
{
	namespace DataTypes
	{
		enum DataTypes
		{
			Byte			= 0x1400,
			Short			= 0x1402,
			Int				= 0x1404,
			Float			= 0x1406,
			HalfFloat		= 0x140B,
			Double			= 0x140A,
			UnsignedByte	= 0x1401,
			UnsignedShort	= 0x1403,
			UnsignedInt		= 0x1405
		};
	}

	namespace PrimitiveTypes
	{
		enum PrimitiveTypes
		{
			Points			= 0x0000,
			Lines			= 0x0001,
			LineLoop		= 0x0002,
			LineStrip		= 0x0003,
			Triangles		= 0x0004,
			TriangleStrip	= 0x0005,
			TriangleFan		= 0x0006
		};
	}

	/*!
		\note
			All operations require prior binding
	*/
	class VertexArray
	{
	public:

		VertexArray();
		~VertexArray();
		
		VertexArray(const VertexArray& rhs) = delete;
		VertexArray& operator=(const VertexArray& rhd) = delete;

		//bind + unbind
		Unbinder<VertexArray> Bind();
		void Unbind();

		//add index buffer
		void AddIndexBuffer(const std::shared_ptr<Buffer<unsigned int>>& indices);

		/*!
			\brief
				Adds the given buffer as a data buffer to the VertexArray

			\param buffer
				The buffer to add

			\param stride
				The distance between the starts of 2 of the same elements
				(usually the size of the data type stored in the buffer)

			\param offset
				The offset into the buffer to start at

			\returns
				The index of this buffer (needed to set up attribute association)
		*/
		unsigned AddDataBuffer(const std::shared_ptr<IBindableBuffer>& buffer, unsigned stride, unsigned offset = 0);

		/*!
			\brief
				This associates data in a bound data buffer with a shader vertex attribute

			\param attribute_idx
				The index of the vertex attribute

			\param buffer_idx
				The index of the buffer

			\param num_elements
				The number of elements this atribute consists of

			\param type
				The data type of this attribute

			\param offset
				The offset from the start of the buffer for the first attribute

			\param normalize
				Should the data be normalized?
		*/
		void SetAttributeAssociation(unsigned attribute_idx, unsigned buffer_idx, unsigned num_elements, DataTypes::DataTypes type, unsigned offset, bool normalize = false);

		//draw (not directly connected with this, but makes most sense imo)
		void Draw(PrimitiveTypes::PrimitiveTypes type, unsigned num_vertices, unsigned offset = 0);

	private:

		unsigned gl_object_;

		bool has_index_buffer_;
		std::shared_ptr<Buffer<unsigned int>> index_buffer_;

		unsigned num_buffers_;
		std::vector<std::shared_ptr<IBindableBuffer>> data_buffers_;

	};
}

#endif
