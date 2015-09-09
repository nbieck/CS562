////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "VertexArray.h"

#include "../OpenGL/gl_core_4_4.hpp"

namespace CS562
{
	VertexArray::VertexArray()
		: has_index_buffer_(false), num_buffers_(0)
	{
		gl::GenVertexArrays(1, &gl_object_);
	}

	VertexArray::~VertexArray()
	{
		gl::DeleteVertexArrays(1, &gl_object_);
	}

	void VertexArray::AddIndexBuffer(const std::shared_ptr<Buffer<unsigned int>>& indices)
	{
		index_buffer_ = indices;
		index_buffer_->BindWithoutUnbind(BufferTargets::Index);
		has_index_buffer_ = true;
	}

	Unbinder<VertexArray> VertexArray::Bind()
	{
		gl::BindVertexArray(gl_object_);
		
		return Unbinder<VertexArray>(*this);
	}

	void VertexArray::Unbind()
	{
		gl::BindVertexArray(0);
	}

	unsigned VertexArray::AddDataBuffer(const std::shared_ptr<IBindableBuffer>& buffer, unsigned stride, unsigned offset)
	{
		data_buffers_.push_back(buffer);
		gl::BindVertexBuffer(num_buffers_, buffer->GetGLObject(), offset, stride);
		return (num_buffers_++);
	}

	void VertexArray::SetAttributeAssociation(unsigned attribute_idx, 
											  unsigned buffer_idx, 
											  unsigned num_elements, 
											  DataTypes::DataTypes type, 
											  unsigned offset,
											  bool normalize)
	{
		gl::EnableVertexAttribArray(attribute_idx);
		gl::VertexAttribBinding(attribute_idx, buffer_idx);
		gl::VertexAttribFormat(attribute_idx, num_elements, type, normalize, offset);
	}

	void VertexArray::Draw(PrimitiveTypes::PrimitiveTypes type, unsigned num_vertices, unsigned offset)
	{
		if (has_index_buffer_)
			gl::DrawElements(type, num_vertices, gl::UNSIGNED_INT, reinterpret_cast<GLvoid*>(offset * sizeof(unsigned)));
		else
			gl::DrawArrays(type, offset, num_vertices);
	}
}
